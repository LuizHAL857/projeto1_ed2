#include "../include/trata_qry.h"

#include "../include/habitante.h"
#include "../include/lista.h"
#include "../include/quadra.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRATA_QRY_MARGEM_SVG 10.0
#define TRATA_QRY_X_TAMANHO 4.0
#define TRATA_QRY_QUADRADO_TAMANHO 8.0
#define TRATA_QRY_CIRCULO_RAIO 3.0
#define TRATA_QRY_MARGEM_TEXTO_FACE 4.0

typedef enum {
    ANOTACAO_LINHA,
    ANOTACAO_TEXTO,
    ANOTACAO_RETANGULO,
    ANOTACAO_CIRCULO
} TipoAnotacaoSvg;

typedef struct {
    TipoAnotacaoSvg tipo;
    double x1;
    double y1;
    double x2;
    double y2;
    double x_texto;
    double y_texto;
    double raio;
    int tamanho_fonte;
    char ancora[8];
    char conteudo[64];
    char cor[16];
    char preenchimento[16];
} AnotacaoSvg;

typedef struct {
    TrataGeo trata_geo;
    TrataPm trata_pm;
    char *nome_qry;
    char *caminho_svg_final;
    char *caminho_txt_final;
    Lista anotacoes_svg;
} TrataQryImpl;

typedef struct {
    int norte;
    int sul;
    int leste;
    int oeste;
    int total;
} ContagemQuadra;

typedef struct {
    int habitantes_total;
    int moradores_total;
    int homens_total;
    int mulheres_total;
    int moradores_homens;
    int moradores_mulheres;
    int sem_teto_total;
    int sem_teto_homens;
    int sem_teto_mulheres;
} EstatisticasCenso;

static TrataQryImpl *trata_qry_impl(TrataQry trata_qry) {
    return (TrataQryImpl *)trata_qry;
}
static void liberar_anotacoes(Lista anotacoes);
static void destruir_impl(TrataQryImpl *trata_qry, bool remover_arquivos);

//Funções de adição de formas ao svg
static bool adicionar_linha_svg(TrataQryImpl *trata_qry, double x1, double y1,
                                double x2, double y2, const char *cor);
static bool adicionar_retangulo_svg(TrataQryImpl *trata_qry, double x, double y,
                                    double largura, double altura, const char *cor,
                                    const char *preenchimento);
static bool adicionar_circulo_svg(TrataQryImpl *trata_qry, double x, double y,
                                  double raio, const char *cor,
                                  const char *preenchimento);
static bool adicionar_texto_svg_tamanho(TrataQryImpl *trata_qry, double x, double y,
                                        const char *ancora, const char *cor,
                                        const char *conteudo, int tamanho_fonte);
static bool adicionar_texto_svg(TrataQryImpl *trata_qry, double x, double y,
                                const char *ancora, const char *cor,
                                const char *conteudo);

static void calcular_limites_quadras(Lista quadras, double *min_x, double *min_y,
                                     double *max_x, double *max_y, bool *encontrou);
static void calcular_limites_anotacoes(Lista anotacoes, double *min_x, double *min_y,
                                       double *max_x, double *max_y, bool *encontrou);
static bool escrever_svg_final(TrataQryImpl *trata_qry);
static bool escrever_cabecalho_consulta(FILE *txt, const char *linha);
static bool escrever_dados_habitante(FILE *txt, Habitante habitante,
                                     const char *rotulo_endereco);
static bool calcular_ponto_endereco(TrataQryImpl *trata_qry, const char *cep, char face,
                                    int num, double *x, double *y);

static Lista listar_habitantes_atuais(TrataQryImpl *trata_qry);
static void contar_moradores_da_quadra(Lista habitantes, const char *cep,
                                       ContagemQuadra *contagem);
static void calcular_censo(Lista habitantes, EstatisticasCenso *estatisticas);
static double calcular_percentual(int parte, int total);
//Execução de comandos
static bool executar_h(TrataQryImpl *trata_qry, FILE *txt, const char *linha);
static bool executar_nasc(TrataQryImpl *trata_qry, FILE *txt, const char *linha);
static bool executar_rip(TrataQryImpl *trata_qry, FILE *txt, const char *linha);
static bool executar_mud(TrataQryImpl *trata_qry, FILE *txt, const char *linha);
static bool executar_dspj(TrataQryImpl *trata_qry, FILE *txt, const char *linha);
static bool executar_rq(TrataQryImpl *trata_qry, FILE *txt, const char *linha);
static bool executar_pq(TrataQryImpl *trata_qry, FILE *txt, const char *linha);
static bool executar_censo(TrataQryImpl *trata_qry, FILE *txt, const char *linha);
static bool executa_linha_qry(TrataQryImpl *trata_qry, FILE *txt, const char *linha);
static bool processar_linhas_qry(TrataQryImpl *trata_qry, DadosDoArquivo dados_qry);
static bool preparar_estrutura(TrataQryImpl *trata_qry, DadosDoArquivo dados_qry,
                               TrataGeo trata_geo, TrataPm trata_pm,
                               const char *caminho_output);

TrataQry processa_qry(DadosDoArquivo dados_qry, TrataGeo trata_geo, TrataPm trata_pm,
                      const char *caminho_output) {
    TrataQryImpl *trata_qry;

    trata_qry = (TrataQryImpl *)calloc(1, sizeof(TrataQryImpl));
    if (trata_qry == NULL) {
        return NULL;
    }

    if (!preparar_estrutura(trata_qry, dados_qry, trata_geo, trata_pm, caminho_output) ||
        !processar_linhas_qry(trata_qry, dados_qry) ||
        !escrever_svg_final(trata_qry)) {
        destruir_impl(trata_qry, true);
        return NULL;
    }

    return trata_qry;
}

void trata_qry_destruir(TrataQry trata_qry) {
    destruir_impl(trata_qry_impl(trata_qry), false);
}

const char *trata_qry_obter_nome_qry(TrataQry trata_qry) {
    TrataQryImpl *impl = trata_qry_impl(trata_qry);
    return impl == NULL ? NULL : impl->nome_qry;
}

static void liberar_anotacoes(Lista anotacoes) {
    if (anotacoes == NULL) {
        return;
    }

    while (!listaVazia(anotacoes)) {
        free(removeInicioLista(anotacoes));
    }

    liberaLista(anotacoes);
}

static void destruir_impl(TrataQryImpl *trata_qry, bool remover_arquivos) {
    if (trata_qry == NULL) {
        return;
    }

    if (remover_arquivos && trata_qry->caminho_svg_final != NULL) {
        remove(trata_qry->caminho_svg_final);
    }

    if (remover_arquivos && trata_qry->caminho_txt_final != NULL) {
        remove(trata_qry->caminho_txt_final);
    }

    liberar_anotacoes(trata_qry->anotacoes_svg);
    free(trata_qry->nome_qry);
    free(trata_qry->caminho_svg_final);
    free(trata_qry->caminho_txt_final);
    free(trata_qry);
}

static bool adicionar_linha_svg(TrataQryImpl *trata_qry, double x1, double y1,
                                double x2, double y2, const char *cor) {
    AnotacaoSvg *anotacao;

    if (trata_qry == NULL || trata_qry->anotacoes_svg == NULL || cor == NULL) {
        return false;
    }

    anotacao = (AnotacaoSvg *)calloc(1, sizeof(AnotacaoSvg));
    if (anotacao == NULL) {
        return false;
    }

    anotacao->tipo = ANOTACAO_LINHA;
    anotacao->x1 = x1;
    anotacao->y1 = y1;
    anotacao->x2 = x2;
    anotacao->y2 = y2;
    strncpy(anotacao->cor, cor, sizeof(anotacao->cor) - 1u);
    insereFinalLista(trata_qry->anotacoes_svg, anotacao);
    return true;
}

static bool adicionar_retangulo_svg(TrataQryImpl *trata_qry, double x, double y,
                                    double largura, double altura, const char *cor,
                                    const char *preenchimento) {
    AnotacaoSvg *anotacao;

    if (trata_qry == NULL || trata_qry->anotacoes_svg == NULL || cor == NULL ||
        preenchimento == NULL) {
        return false;
    }

    anotacao = (AnotacaoSvg *)calloc(1, sizeof(AnotacaoSvg));
    if (anotacao == NULL) {
        return false;
    }

    anotacao->tipo = ANOTACAO_RETANGULO;
    anotacao->x1 = x;
    anotacao->y1 = y;
    anotacao->x2 = largura;
    anotacao->y2 = altura;
    strncpy(anotacao->cor, cor, sizeof(anotacao->cor) - 1u);
    strncpy(anotacao->preenchimento, preenchimento, sizeof(anotacao->preenchimento) - 1u);
    insereFinalLista(trata_qry->anotacoes_svg, anotacao);
    return true;
}

static bool adicionar_circulo_svg(TrataQryImpl *trata_qry, double x, double y,
                                  double raio, const char *cor,
                                  const char *preenchimento) {
    AnotacaoSvg *anotacao;

    if (trata_qry == NULL || trata_qry->anotacoes_svg == NULL || cor == NULL ||
        preenchimento == NULL) {
        return false;
    }

    anotacao = (AnotacaoSvg *)calloc(1, sizeof(AnotacaoSvg));
    if (anotacao == NULL) {
        return false;
    }

    anotacao->tipo = ANOTACAO_CIRCULO;
    anotacao->x1 = x;
    anotacao->y1 = y;
    anotacao->raio = raio;
    strncpy(anotacao->cor, cor, sizeof(anotacao->cor) - 1u);
    strncpy(anotacao->preenchimento, preenchimento, sizeof(anotacao->preenchimento) - 1u);
    insereFinalLista(trata_qry->anotacoes_svg, anotacao);
    return true;
}

static bool adicionar_texto_svg_tamanho(TrataQryImpl *trata_qry, double x, double y,
                                        const char *ancora, const char *cor,
                                        const char *conteudo, int tamanho_fonte) {
    AnotacaoSvg *anotacao;

    if (trata_qry == NULL || trata_qry->anotacoes_svg == NULL || ancora == NULL ||
        cor == NULL || conteudo == NULL || tamanho_fonte <= 0) {
        return false;
    }

    anotacao = (AnotacaoSvg *)calloc(1, sizeof(AnotacaoSvg));
    if (anotacao == NULL) {
        return false;
    }

    anotacao->tipo = ANOTACAO_TEXTO;
    anotacao->x_texto = x;
    anotacao->y_texto = y;
    anotacao->tamanho_fonte = tamanho_fonte;
    strncpy(anotacao->ancora, ancora, sizeof(anotacao->ancora) - 1u);
    strncpy(anotacao->cor, cor, sizeof(anotacao->cor) - 1u);
    strncpy(anotacao->conteudo, conteudo, sizeof(anotacao->conteudo) - 1u);
    insereFinalLista(trata_qry->anotacoes_svg, anotacao);
    return true;
}

static bool adicionar_texto_svg(TrataQryImpl *trata_qry, double x, double y,
                                const char *ancora, const char *cor,
                                const char *conteudo) {
    return adicionar_texto_svg_tamanho(trata_qry, x, y, ancora, cor, conteudo, 11);
}

static void considerar_ponto(double x, double y, double *min_x, double *min_y,
                             double *max_x, double *max_y, bool *encontrou) {
    if (!*encontrou) {
        *min_x = x;
        *min_y = y;
        *max_x = x;
        *max_y = y;
        *encontrou = true;
        return;
    }

    if (x < *min_x) {
        *min_x = x;
    }
    if (y < *min_y) {
        *min_y = y;
    }
    if (x > *max_x) {
        *max_x = x;
    }
    if (y > *max_y) {
        *max_y = y;
    }
}

static void calcular_limites_quadras(Lista quadras, double *min_x, double *min_y,
                                     double *max_x, double *max_y, bool *encontrou) {
    Celula atual = getInicioLista(quadras);

    while (atual != NULL) {
        Quadra quadra = (Quadra)getConteudoCelula(atual);
        double esquerda = quadra_obter_x(quadra) - quadra_obter_w(quadra);
        double topo = quadra_obter_y(quadra) - quadra_obter_h(quadra);
        double direita = quadra_obter_x(quadra);
        double base = quadra_obter_y(quadra);

        considerar_ponto(esquerda, topo, min_x, min_y, max_x, max_y, encontrou);
        considerar_ponto(direita, base, min_x, min_y, max_x, max_y, encontrou);
        atual = getProxCelula(atual);
    }
}

static void calcular_limites_anotacoes(Lista anotacoes, double *min_x, double *min_y,
                                       double *max_x, double *max_y, bool *encontrou) {
    Celula atual = getInicioLista(anotacoes);

    while (atual != NULL) {
        AnotacaoSvg *anotacao = (AnotacaoSvg *)getConteudoCelula(atual);

        if (anotacao->tipo == ANOTACAO_LINHA) {
            considerar_ponto(anotacao->x1, anotacao->y1, min_x, min_y, max_x, max_y,
                             encontrou);
            considerar_ponto(anotacao->x2, anotacao->y2, min_x, min_y, max_x, max_y,
                             encontrou);
        } else if (anotacao->tipo == ANOTACAO_TEXTO) {
            considerar_ponto(anotacao->x_texto - 8.0, anotacao->y_texto - 12.0,
                             min_x, min_y, max_x, max_y, encontrou);
            considerar_ponto(anotacao->x_texto + 8.0, anotacao->y_texto + 4.0,
                             min_x, min_y, max_x, max_y, encontrou);
        } else if (anotacao->tipo == ANOTACAO_RETANGULO) {
            considerar_ponto(anotacao->x1, anotacao->y1, min_x, min_y, max_x, max_y,
                             encontrou);
            considerar_ponto(anotacao->x1 + anotacao->x2, anotacao->y1 + anotacao->y2,
                             min_x, min_y, max_x, max_y, encontrou);
        } else if (anotacao->tipo == ANOTACAO_CIRCULO) {
            considerar_ponto(anotacao->x1 - anotacao->raio, anotacao->y1 - anotacao->raio,
                             min_x, min_y, max_x, max_y, encontrou);
            considerar_ponto(anotacao->x1 + anotacao->raio, anotacao->y1 + anotacao->raio,
                             min_x, min_y, max_x, max_y, encontrou);
        }

        atual = getProxCelula(atual);
    }
}

static bool escrever_svg_final(TrataQryImpl *trata_qry) {
    FILE *arquivo;
    Lista quadras;
    Celula atual;
    double min_x = 0.0;
    double min_y = 0.0;
    double max_x = 100.0;
    double max_y = 100.0;
    double largura;
    double altura;
    bool encontrou = false;

    if (trata_qry == NULL || trata_qry->caminho_svg_final == NULL) {
        return false;
    }

    quadras = trata_geo_listar_quadras(trata_qry->trata_geo);
    if (quadras == NULL) {
        return false;
    }

    calcular_limites_quadras(quadras, &min_x, &min_y, &max_x, &max_y, &encontrou);
    calcular_limites_anotacoes(trata_qry->anotacoes_svg, &min_x, &min_y, &max_x, &max_y,
                               &encontrou);
    if (!encontrou) {
        min_x = 0.0;
        min_y = 0.0;
        max_x = 100.0;
        max_y = 100.0;
    }

    largura = (max_x - min_x) + 2.0 * TRATA_QRY_MARGEM_SVG;
    altura = (max_y - min_y) + 2.0 * TRATA_QRY_MARGEM_SVG;

    arquivo = fopen(trata_qry->caminho_svg_final, "w");
    if (arquivo == NULL) {
        trata_geo_liberar_quadras(quadras);
        return false;
    }

    if (fprintf(arquivo,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                "viewBox=\"%.2f %.2f %.2f %.2f\">\n",
                min_x - TRATA_QRY_MARGEM_SVG, min_y - TRATA_QRY_MARGEM_SVG,
                largura, altura) < 0) {
        fclose(arquivo);
        trata_geo_liberar_quadras(quadras);
        return false;
    }

    atual = getInicioLista(quadras);
    while (atual != NULL) {
        Quadra quadra = (Quadra)getConteudoCelula(atual);
        double x = quadra_obter_x(quadra) - quadra_obter_w(quadra);
        double y = quadra_obter_y(quadra) - quadra_obter_h(quadra);

        if (fprintf(arquivo,
                    "  <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" "
                    "fill=\"%s\" fill-opacity=\"0.70\" stroke=\"%s\" "
                    "stroke-width=\"%s\" />\n",
                    x, y, quadra_obter_w(quadra), quadra_obter_h(quadra),
                    quadra_obter_cfill(quadra), quadra_obter_cstrk(quadra),
                    quadra_obter_sw(quadra)) < 0 ||
            fprintf(arquivo,
                    "  <text x=\"%.2f\" y=\"%.2f\" font-size=\"11\" "
                    "font-weight=\"bold\" fill=\"black\" stroke=\"white\" "
                    "stroke-width=\"0.45\" paint-order=\"stroke\">%s</text>\n",
                    x + 4.0, y + 12.0, quadra_obter_cep(quadra)) < 0) {
            fclose(arquivo);
            trata_geo_liberar_quadras(quadras);
            return false;
        }

        atual = getProxCelula(atual);
    }

    atual = getInicioLista(trata_qry->anotacoes_svg);
    while (atual != NULL) {
        AnotacaoSvg *anotacao = (AnotacaoSvg *)getConteudoCelula(atual);

        if (anotacao->tipo == ANOTACAO_LINHA) {
            if (fprintf(arquivo,
                        "  <line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" "
                        "stroke=\"%s\" stroke-width=\"1.60\" />\n",
                        anotacao->x1, anotacao->y1, anotacao->x2, anotacao->y2,
                        anotacao->cor) < 0) {
                fclose(arquivo);
                trata_geo_liberar_quadras(quadras);
                return false;
            }
        } else if (anotacao->tipo == ANOTACAO_TEXTO) {
            if (fprintf(arquivo,
                        "  <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"%s\" "
                        "font-size=\"%d\" font-weight=\"bold\" fill=\"%s\" "
                        "stroke=\"white\" stroke-width=\"0.45\" "
                        "paint-order=\"stroke\">%s</text>\n",
                        anotacao->x_texto, anotacao->y_texto, anotacao->ancora,
                        anotacao->tamanho_fonte, anotacao->cor,
                        anotacao->conteudo) < 0) {
                fclose(arquivo);
                trata_geo_liberar_quadras(quadras);
                return false;
            }
        } else if (anotacao->tipo == ANOTACAO_RETANGULO) {
            if (fprintf(arquivo,
                        "  <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" "
                        "fill=\"%s\" stroke=\"%s\" stroke-width=\"1.20\" />\n",
                        anotacao->x1, anotacao->y1, anotacao->x2, anotacao->y2,
                        anotacao->preenchimento, anotacao->cor) < 0) {
                fclose(arquivo);
                trata_geo_liberar_quadras(quadras);
                return false;
            }
        } else if (anotacao->tipo == ANOTACAO_CIRCULO) {
            if (fprintf(arquivo,
                        "  <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" fill=\"%s\" "
                        "stroke=\"%s\" stroke-width=\"1.20\" />\n",
                        anotacao->x1, anotacao->y1, anotacao->raio,
                        anotacao->preenchimento, anotacao->cor) < 0) {
                fclose(arquivo);
                trata_geo_liberar_quadras(quadras);
                return false;
            }
        }

        atual = getProxCelula(atual);
    }

    if (fprintf(arquivo, "</svg>\n") < 0) {
        fclose(arquivo);
        trata_geo_liberar_quadras(quadras);
        return false;
    }

    fclose(arquivo);
    trata_geo_liberar_quadras(quadras);
    return true;
}

static bool escrever_cabecalho_consulta(FILE *txt, const char *linha) {
    return txt != NULL && linha != NULL && fprintf(txt, "[*] %s\n", linha) >= 0;
}

static bool escrever_dados_habitante(FILE *txt, Habitante habitante,
                                     const char *rotulo_endereco) {
    if (txt == NULL || habitante == NULL) {
        return false;
    }

    if (fprintf(txt, "cpf: %s\nnome: %s %s\nsexo: %c\nnasc: %s\n",
                habitante_obter_cpf(habitante), habitante_obter_nome(habitante),
                habitante_obter_sobrenome(habitante), habitante_obter_sexo(habitante),
                habitante_obter_nasc(habitante)) < 0) {
        return false;
    }

    if (habitante_eh_morador(habitante)) {
        const char *rotulo = rotulo_endereco == NULL ? "endereco" : rotulo_endereco;

        return fprintf(txt, "situacao: morador\n%s: %s/%c/%d %s\n", rotulo,
                       habitante_obter_cep(habitante), habitante_obter_face(habitante),
                       habitante_obter_num(habitante), habitante_obter_compl(habitante)) >= 0;
    }

    return fprintf(txt, "situacao: sem-teto\n") >= 0;
}

static bool calcular_ponto_endereco(TrataQryImpl *trata_qry, const char *cep, char face,
                                    int num, double *x, double *y) {
    Quadra quadra;
    double esquerda;
    double topo;
    double direita;
    double base;

    if (trata_qry == NULL || cep == NULL || x == NULL || y == NULL || num < 0) {
        return false;
    }

    quadra = trata_geo_obter_quadra(trata_qry->trata_geo, cep);
    if (quadra == NULL) {
        return false;
    }

    esquerda = quadra_obter_x(quadra) - quadra_obter_w(quadra);
    topo = quadra_obter_y(quadra) - quadra_obter_h(quadra);
    direita = quadra_obter_x(quadra);
    base = quadra_obter_y(quadra);

    if (face == 'N') {
        *x = esquerda + (double)num;
        *y = base;
    } else if (face == 'S') {
        *x = esquerda + (double)num;
        *y = topo;
    } else if (face == 'L') {
        *x = esquerda;
        *y = topo + (double)num;
    } else if (face == 'O') {
        *x = direita;
        *y = topo + (double)num;
    } else {
        quadra_destruir(quadra);
        return false;
    }

    quadra_destruir(quadra);
    return true;
}

static Lista listar_habitantes_atuais(TrataQryImpl *trata_qry) {
    Lista habitantes;

    if (trata_qry == NULL) {
        return NULL;
    }

    if (trata_qry->trata_pm == NULL) {
        habitantes = criaLista();
        return habitantes;
    }

    return trata_pm_listar_habitantes(trata_qry->trata_pm);
}

static void contar_moradores_da_quadra(Lista habitantes, const char *cep,
                                       ContagemQuadra *contagem) {
    Celula atual = getInicioLista(habitantes);

    memset(contagem, 0, sizeof(*contagem));
    while (atual != NULL) {
        Habitante habitante = (Habitante)getConteudoCelula(atual);

        if (habitante != NULL && habitante_eh_morador(habitante) &&
            strcmp(habitante_obter_cep(habitante), cep) == 0) {
            char face = habitante_obter_face(habitante);

            contagem->total++;
            if (face == 'N') {
                contagem->norte++;
            } else if (face == 'S') {
                contagem->sul++;
            } else if (face == 'L') {
                contagem->leste++;
            } else if (face == 'O') {
                contagem->oeste++;
            }
        }

        atual = getProxCelula(atual);
    }
}

static void calcular_censo(Lista habitantes, EstatisticasCenso *estatisticas) {
    Celula atual = getInicioLista(habitantes);

    memset(estatisticas, 0, sizeof(*estatisticas));
    while (atual != NULL) {
        Habitante habitante = (Habitante)getConteudoCelula(atual);
        bool morador;

        if (habitante == NULL) {
            atual = getProxCelula(atual);
            continue;
        }

        estatisticas->habitantes_total++;
        if (habitante_obter_sexo(habitante) == 'M') {
            estatisticas->homens_total++;
        } else if (habitante_obter_sexo(habitante) == 'F') {
            estatisticas->mulheres_total++;
        }

        morador = habitante_eh_morador(habitante);
        if (morador) {
            estatisticas->moradores_total++;
            if (habitante_obter_sexo(habitante) == 'M') {
                estatisticas->moradores_homens++;
            } else if (habitante_obter_sexo(habitante) == 'F') {
                estatisticas->moradores_mulheres++;
            }
        } else {
            estatisticas->sem_teto_total++;
            if (habitante_obter_sexo(habitante) == 'M') {
                estatisticas->sem_teto_homens++;
            } else if (habitante_obter_sexo(habitante) == 'F') {
                estatisticas->sem_teto_mulheres++;
            }
        }

        atual = getProxCelula(atual);
    }
}

static double calcular_percentual(int parte, int total) {
    if (total <= 0) {
        return 0.0;
    }

    return ((double)parte * 100.0) / (double)total;
}

static bool executar_h(TrataQryImpl *trata_qry, FILE *txt, const char *linha) {
    char cpf[128];
    int pos = 0;
    Habitante habitante;
    bool sucesso;

    if (trata_qry == NULL || trata_qry->trata_pm == NULL) {
        return false;
    }

    if (sscanf(linha, "h? %127s %n", cpf, &pos) != 1 ||
        !arquivo_resto_valido(linha + pos)) {
        return false;
    }

    habitante = trata_pm_obter_habitante(trata_qry->trata_pm, cpf);
    if (habitante == NULL) {
        return escrever_cabecalho_consulta(txt, linha) &&
               fprintf(txt, "habitante inexistente\n\n") >= 0;
    }

    sucesso = escrever_cabecalho_consulta(txt, linha) &&
              escrever_dados_habitante(txt, habitante, "endereco") &&
              fprintf(txt, "\n") >= 0;
    habitante_destruir(habitante);
    return sucesso;
}

static bool executar_nasc(TrataQryImpl *trata_qry, FILE *txt, const char *linha) {
    char cpf[128];
    char nome[128];
    char sobrenome[128];
    char sexo;
    char nasc[128];
    Habitante habitante;
    int pos = 0;

    if (trata_qry == NULL || trata_qry->trata_pm == NULL) {
        return false;
    }

    if (sscanf(linha, "nasc %127s %127s %127s %c %127s %n",
               cpf, nome, sobrenome, &sexo, nasc, &pos) != 5 ||
        !arquivo_resto_valido(linha + pos)) {
        return false;
    }

    if (!trata_pm_inserir_habitante(trata_qry->trata_pm, cpf, nome, sobrenome, sexo, nasc)) {
        return false;
    }

    habitante = trata_pm_obter_habitante(trata_qry->trata_pm, cpf);
    habitante_destruir(habitante);
    return escrever_cabecalho_consulta(txt, linha);
}

static bool executar_rip(TrataQryImpl *trata_qry, FILE *txt, const char *linha) {
    char cpf[128];
    Habitante habitante;
    int pos = 0;
    bool era_morador;
    double x = 0.0;
    double y = 0.0;

    if (trata_qry == NULL || trata_qry->trata_pm == NULL) {
        return false;
    }

    if (sscanf(linha, "rip %127s %n", cpf, &pos) != 1 ||
        !arquivo_resto_valido(linha + pos)) {
        return false;
    }

    habitante = trata_pm_obter_habitante(trata_qry->trata_pm, cpf);
    if (habitante == NULL) {
        return escrever_cabecalho_consulta(txt, linha) &&
               fprintf(txt, "habitante inexistente\n\n") >= 0;
    }

    era_morador = habitante_eh_morador(habitante);
    if (era_morador &&
        !calcular_ponto_endereco(trata_qry, habitante_obter_cep(habitante),
                                 habitante_obter_face(habitante),
                                 habitante_obter_num(habitante), &x, &y)) {
        habitante_destruir(habitante);
        return false;
    }

    if (!escrever_cabecalho_consulta(txt, linha) ||
        !escrever_dados_habitante(txt, habitante, "endereco do falecimento") ||
        fprintf(txt, "\n") < 0 ||
        !trata_pm_remover_habitante(trata_qry->trata_pm, cpf)) {
        habitante_destruir(habitante);
        return false;
    }

    if (era_morador &&
        (!adicionar_linha_svg(trata_qry, x - TRATA_QRY_X_TAMANHO, y - TRATA_QRY_X_TAMANHO,
                              x + TRATA_QRY_X_TAMANHO, y + TRATA_QRY_X_TAMANHO, "red") ||
         !adicionar_linha_svg(trata_qry, x - TRATA_QRY_X_TAMANHO, y + TRATA_QRY_X_TAMANHO,
                              x + TRATA_QRY_X_TAMANHO, y - TRATA_QRY_X_TAMANHO, "red"))) {
        habitante_destruir(habitante);
        return false;
    }

    habitante_destruir(habitante);
    return true;
}

static bool executar_mud(TrataQryImpl *trata_qry, FILE *txt, const char *linha) {
    char cpf[128];
    char cep[128];
    char face;
    char face_token[128];
    int num;
    char compl[128];
    double x;
    double y;
    int pos = 0;

    if (trata_qry == NULL || trata_qry->trata_pm == NULL) {
        return false;
    }

    face = '\0';
    if (sscanf(linha, "mud %127s %127s %127s %d %127s %n",
               cpf, cep, face_token, &num, compl, &pos) != 5 ||
        !arquivo_resto_valido(linha + pos)) {
        return false;
    }

    if (strlen(face_token) != 1u) {
        return false;
    }

    face = (char)toupper((unsigned char)face_token[0]);
    if (face != 'N' && face != 'S' && face != 'L' && face != 'O') {
        return false;
    }

    if (!calcular_ponto_endereco(trata_qry, cep, face, num, &x, &y) ||
        !trata_pm_definir_moradia(trata_qry->trata_pm, cpf, cep, face, num, compl) ||
        !escrever_cabecalho_consulta(txt, linha) ||
        !adicionar_retangulo_svg(trata_qry, x - TRATA_QRY_QUADRADO_TAMANHO / 2.0,
                                 y - TRATA_QRY_QUADRADO_TAMANHO / 2.0,
                                 TRATA_QRY_QUADRADO_TAMANHO,
                                 TRATA_QRY_QUADRADO_TAMANHO, "red", "none") ||
        !adicionar_texto_svg_tamanho(trata_qry, x, y + 2.0, "middle", "red", cpf, 7)) {
        return false;
    }

    return true;
}

static bool executar_dspj(TrataQryImpl *trata_qry, FILE *txt, const char *linha) {
    char cpf[128];
    Habitante habitante;
    int pos = 0;
    double x;
    double y;

    if (trata_qry == NULL || trata_qry->trata_pm == NULL) {
        return false;
    }

    if (sscanf(linha, "dspj %127s %n", cpf, &pos) != 1 ||
        !arquivo_resto_valido(linha + pos)) {
        return false;
    }

    habitante = trata_pm_obter_habitante(trata_qry->trata_pm, cpf);
    if (habitante == NULL) {
        habitante_destruir(habitante);
        return false;
    }

    if (!habitante_eh_morador(habitante)) {
        if (!escrever_cabecalho_consulta(txt, linha) ||
            !escrever_dados_habitante(txt, habitante, "endereco do despejo") ||
            fprintf(txt, "\n") < 0) {
            habitante_destruir(habitante);
            return false;
        }

        habitante_destruir(habitante);
        return true;
    }

    if (!calcular_ponto_endereco(trata_qry, habitante_obter_cep(habitante),
                                 habitante_obter_face(habitante),
                                 habitante_obter_num(habitante), &x, &y) ||
        !escrever_cabecalho_consulta(txt, linha) ||
        !escrever_dados_habitante(txt, habitante, "endereco do despejo") ||
        fprintf(txt, "\n") < 0 ||
        !trata_pm_remover_moradia(trata_qry->trata_pm, cpf) ||
        !adicionar_circulo_svg(trata_qry, x, y, TRATA_QRY_CIRCULO_RAIO, "black",
                               "black")) {
        habitante_destruir(habitante);
        return false;
    }

    habitante_destruir(habitante);
    return true;
}

static bool executar_rq(TrataQryImpl *trata_qry, FILE *txt, const char *linha) {
    char cep[128];
    int pos = 0;
    Quadra quadra;
    Lista afetados;
    Celula atual;

    if (sscanf(linha, "rq %127s %n", cep, &pos) != 1 ||
        !arquivo_resto_valido(linha + pos)) {
        return false;
    }

    quadra = trata_geo_obter_quadra(trata_qry->trata_geo, cep);
    if (quadra == NULL) {
        return false;
    }

    if (!escrever_cabecalho_consulta(txt, linha)) {
        quadra_destruir(quadra);
        return false;
    }

    if (trata_qry->trata_pm != NULL) {
        afetados = trata_pm_tornar_sem_teto_por_cep(trata_qry->trata_pm, cep);
    } else {
        afetados = criaLista();
    }
    if (afetados == NULL) {
        quadra_destruir(quadra);
        return false;
    }

    if (!trata_geo_remover_quadra(trata_qry->trata_geo, cep)) {
        trata_pm_liberar_habitantes(afetados);
        quadra_destruir(quadra);
        return false;
    }

    if (listaVazia(afetados)) {
        if (fprintf(txt, "nenhum morador afetado\n") < 0) {
            trata_pm_liberar_habitantes(afetados);
            quadra_destruir(quadra);
            return false;
        }
    } else {
        atual = getInicioLista(afetados);
        while (atual != NULL) {
            Habitante habitante = (Habitante)getConteudoCelula(atual);
            if (fprintf(txt, "%s %s %s\n", habitante_obter_cpf(habitante),
                        habitante_obter_nome(habitante),
                        habitante_obter_sobrenome(habitante)) < 0) {
                trata_pm_liberar_habitantes(afetados);
                quadra_destruir(quadra);
                return false;
            }
            atual = getProxCelula(atual);
        }
    }

    if (fprintf(txt, "\n") < 0 ||
        !adicionar_linha_svg(trata_qry, quadra_obter_x(quadra) - quadra_obter_w(quadra),
                             quadra_obter_y(quadra) - quadra_obter_h(quadra),
                             quadra_obter_x(quadra), quadra_obter_y(quadra), "red") ||
        !adicionar_linha_svg(trata_qry, quadra_obter_x(quadra) - quadra_obter_w(quadra),
                             quadra_obter_y(quadra), quadra_obter_x(quadra),
                             quadra_obter_y(quadra) - quadra_obter_h(quadra), "red")) {
        trata_pm_liberar_habitantes(afetados);
        quadra_destruir(quadra);
        return false;
    }

    trata_pm_liberar_habitantes(afetados);
    quadra_destruir(quadra);
    return true;
}

static bool executar_pq(TrataQryImpl *trata_qry, FILE *txt, const char *linha) {
    char cep[128];
    int pos = 0;
    Quadra quadra;
    Lista habitantes;
    ContagemQuadra contagem;
    char buffer[32];
    double esquerda;
    double topo;
    double direita;
    double base;
    double centro_x;
    double centro_y;

    if (sscanf(linha, "pq %127s %n", cep, &pos) != 1 ||
        !arquivo_resto_valido(linha + pos)) {
        return false;
    }

    quadra = trata_geo_obter_quadra(trata_qry->trata_geo, cep);
    if (quadra == NULL) {
        return false;
    }

    habitantes = listar_habitantes_atuais(trata_qry);
    if (habitantes == NULL) {
        quadra_destruir(quadra);
        return false;
    }

    contar_moradores_da_quadra(habitantes, cep, &contagem);
    if (!escrever_cabecalho_consulta(txt, linha)) {
        trata_pm_liberar_habitantes(habitantes);
        quadra_destruir(quadra);
        return false;
    }

    esquerda = quadra_obter_x(quadra) - quadra_obter_w(quadra);
    topo = quadra_obter_y(quadra) - quadra_obter_h(quadra);
    direita = quadra_obter_x(quadra);
    base = quadra_obter_y(quadra);
    centro_x = esquerda + quadra_obter_w(quadra) / 2.0;
    centro_y = topo + quadra_obter_h(quadra) / 2.0;

    snprintf(buffer, sizeof(buffer), "%d", contagem.norte);
    if (!adicionar_texto_svg(trata_qry, centro_x, base - TRATA_QRY_MARGEM_TEXTO_FACE,
                             "middle", "black", buffer)) {
        trata_pm_liberar_habitantes(habitantes);
        quadra_destruir(quadra);
        return false;
    }

    snprintf(buffer, sizeof(buffer), "%d", contagem.sul);
    if (!adicionar_texto_svg(trata_qry, centro_x, topo + 12.0, "middle", "black", buffer)) {
        trata_pm_liberar_habitantes(habitantes);
        quadra_destruir(quadra);
        return false;
    }

    snprintf(buffer, sizeof(buffer), "%d", contagem.leste);
    if (!adicionar_texto_svg(trata_qry, esquerda + TRATA_QRY_MARGEM_TEXTO_FACE, centro_y,
                             "start", "black", buffer)) {
        trata_pm_liberar_habitantes(habitantes);
        quadra_destruir(quadra);
        return false;
    }

    snprintf(buffer, sizeof(buffer), "%d", contagem.oeste);
    if (!adicionar_texto_svg(trata_qry, direita - TRATA_QRY_MARGEM_TEXTO_FACE, centro_y,
                             "end", "black", buffer)) {
        trata_pm_liberar_habitantes(habitantes);
        quadra_destruir(quadra);
        return false;
    }

    snprintf(buffer, sizeof(buffer), "%d", contagem.total);
    if (!adicionar_texto_svg(trata_qry, centro_x, centro_y + 4.0, "middle", "black",
                             buffer)) {
        trata_pm_liberar_habitantes(habitantes);
        quadra_destruir(quadra);
        return false;
    }

    trata_pm_liberar_habitantes(habitantes);
    quadra_destruir(quadra);
    return true;
}

static bool executar_censo(TrataQryImpl *trata_qry, FILE *txt, const char *linha) {
    Lista habitantes;
    EstatisticasCenso estatisticas;
    double proporcao_moradores;

    if (strcmp(linha, "censo") != 0) {
        return false;
    }

    habitantes = listar_habitantes_atuais(trata_qry);
    if (habitantes == NULL) {
        return false;
    }

    calcular_censo(habitantes, &estatisticas);
    if (estatisticas.habitantes_total <= 0) {
        proporcao_moradores = 0.0;
    } else {
        proporcao_moradores =
            (double)estatisticas.moradores_total / (double)estatisticas.habitantes_total;
    }

    if (!escrever_cabecalho_consulta(txt, linha) ||
        fprintf(txt,
                "habitantes: %d\n"
                "moradores: %d\n"
                "proporcao moradores/habitantes: %.4f\n"
                "homens: %d\n"
                "mulheres: %d\n"
                "%% habitantes homens: %.2f\n"
                "%% habitantes mulheres: %.2f\n"
                "%% moradores homens: %.2f\n"
                "%% moradores mulheres: %.2f\n"
                "sem-tetos: %d\n"
                "%% sem-tetos homens: %.2f\n"
                "%% sem-tetos mulheres: %.2f\n\n",
                estatisticas.habitantes_total, estatisticas.moradores_total,
                proporcao_moradores, estatisticas.homens_total,
                estatisticas.mulheres_total,
                calcular_percentual(estatisticas.homens_total,
                                    estatisticas.habitantes_total),
                calcular_percentual(estatisticas.mulheres_total,
                                    estatisticas.habitantes_total),
                calcular_percentual(estatisticas.moradores_homens,
                                    estatisticas.moradores_total),
                calcular_percentual(estatisticas.moradores_mulheres,
                                    estatisticas.moradores_total),
                estatisticas.sem_teto_total,
                calcular_percentual(estatisticas.sem_teto_homens,
                                    estatisticas.sem_teto_total),
                calcular_percentual(estatisticas.sem_teto_mulheres,
                                    estatisticas.sem_teto_total)) < 0) {
        trata_pm_liberar_habitantes(habitantes);
        return false;
    }

    trata_pm_liberar_habitantes(habitantes);
    return true;
}

static bool executa_linha_qry(TrataQryImpl *trata_qry, FILE *txt, const char *linha) {
    const char *cursor;

    if (trata_qry == NULL || txt == NULL || linha == NULL ||
        arquivo_linha_ignorada(linha)) {
        return true;
    }

    cursor = arquivo_pular_espacos(linha);
    if (strncmp(cursor, "h?", 2u) == 0 && isspace((unsigned char)cursor[2]) != 0) {
        return executar_h(trata_qry, txt, cursor);
    }

    if (strncmp(cursor, "nasc", 4u) == 0 && isspace((unsigned char)cursor[4]) != 0) {
        return executar_nasc(trata_qry, txt, cursor);
    }

    if (strncmp(cursor, "rip", 3u) == 0 && isspace((unsigned char)cursor[3]) != 0) {
        return executar_rip(trata_qry, txt, cursor);
    }

    if (strncmp(cursor, "mud", 3u) == 0 && isspace((unsigned char)cursor[3]) != 0) {
        return executar_mud(trata_qry, txt, cursor);
    }

    if (strncmp(cursor, "dspj", 4u) == 0 && isspace((unsigned char)cursor[4]) != 0) {
        return executar_dspj(trata_qry, txt, cursor);
    }

    if (strncmp(cursor, "rq", 2u) == 0 && isspace((unsigned char)cursor[2]) != 0) {
        return executar_rq(trata_qry, txt, cursor);
    }

    if (strncmp(cursor, "pq", 2u) == 0 && isspace((unsigned char)cursor[2]) != 0) {
        return executar_pq(trata_qry, txt, cursor);
    }

    if (strcmp(cursor, "censo") == 0) {
        return executar_censo(trata_qry, txt, cursor);
    }

    return false;
}

static bool processar_linhas_qry(TrataQryImpl *trata_qry, DadosDoArquivo dados_qry) {
    FILE *txt;
    Lista linhas;
    Celula atual;
    bool sucesso = true;

    if (trata_qry == NULL || dados_qry == NULL || trata_qry->caminho_txt_final == NULL) {
        return false;
    }

    linhas = obter_lista_linhas(dados_qry);
    if (linhas == NULL) {
        return false;
    }

    txt = fopen(trata_qry->caminho_txt_final, "w");
    if (txt == NULL) {
        return false;
    }

    atual = getInicioLista(linhas);
    while (atual != NULL) {
        const char *linha = (const char *)getConteudoCelula(atual);
        if (!executa_linha_qry(trata_qry, txt, linha)) {
            sucesso = false;
            break;
        }
        atual = getProxCelula(atual);
    }

    if (fclose(txt) != 0) {
        sucesso = false;
    }

    return sucesso;
}

static bool preparar_estrutura(TrataQryImpl *trata_qry, DadosDoArquivo dados_qry,
                               TrataGeo trata_geo, TrataPm trata_pm,
                               const char *caminho_output) {
    const char *nome_qry_arquivo;
    const char *nome_geo;
    char *nome_base_saida;

    if (trata_qry == NULL || dados_qry == NULL || trata_geo == NULL ||
        caminho_output == NULL) {
        return false;
    }

    nome_qry_arquivo = obter_nome_arquivo(dados_qry);
    nome_geo = trata_geo_obter_nome_geo(trata_geo);
    trata_qry->trata_geo = trata_geo;
    trata_qry->trata_pm = trata_pm;
    trata_qry->nome_qry = arquivo_extrair_nome_base(nome_qry_arquivo, ".qry");
    trata_qry->anotacoes_svg = criaLista();
    if (trata_qry->nome_qry == NULL || nome_geo == NULL || trata_qry->anotacoes_svg == NULL) {
        return false;
    }

    nome_base_saida = arquivo_montar_nome_composto(nome_geo, "-", trata_qry->nome_qry);
    if (nome_base_saida == NULL) {
        return false;
    }

    trata_qry->caminho_svg_final =
        arquivo_montar_caminho_saida(caminho_output, nome_base_saida, ".svg");
    trata_qry->caminho_txt_final =
        arquivo_montar_caminho_saida(caminho_output, nome_base_saida, ".txt");
    free(nome_base_saida);

    return trata_qry->caminho_svg_final != NULL && trata_qry->caminho_txt_final != NULL;
}
