#include "../include/trata_geo.h"

#include "../include/hash_extensivel.h"
#include "../include/lista.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRATA_GEO_SW_INICIAL "1px"
#define TRATA_GEO_CFILL_INICIAL "none"
#define TRATA_GEO_CSTRK_INICIAL "black"
#define TRATA_GEO_CAPACIDADE_BUCKET 2u
#define TRATA_GEO_MARGEM_SVG 10.0

typedef struct {
    HashExtensivel quadras;
    Lista lista_quadras;
    Lista lista_svg;
    char *nome_geo;
    char *caminho_svg_inicial;
    char *caminho_hash_quadras;
    char *caminho_dump_quadras;
    char sw_atual[QUADRA_TAMANHO_ESPESSURA_MAX + 1];
    char cfill_atual[QUADRA_TAMANHO_COR_MAX + 1];
    char cstrk_atual[QUADRA_TAMANHO_COR_MAX + 1];
} TrataGeoImpl;

static TrataGeoImpl *trata_geo_impl(TrataGeo trata_geo) {
    return (TrataGeoImpl *)trata_geo;
}

/* Valida se uma string nao vazia cabe no limite informado. */
static bool texto_valido(const char *texto, size_t tamanho_maximo);

/* Copia uma string para um buffer de destino com checagem de tamanho. */
static bool copiar_texto(char *destino, size_t tamanho_destino, const char *origem);

/* Avanca o cursor ate o primeiro caractere nao branco. */
static const char *pular_espacos(const char *texto);

/* Identifica linhas vazias ou comentarios do arquivo .geo. */
static bool linha_ignorada(const char *linha);

/* Verifica se o restante da linha esta vazio ou comentado. */
static bool resto_valido(const char *trecho);

/* Confere se um texto termina com um dado sufixo. */
static bool termina_com(const char *texto, const char *sufixo);

/* Extrai o nome-base do arquivo .geo, sem extensao. */
static char *extrair_nome_base_geo(const char *nome_arquivo);

/* Monta um caminho de saida concatenando diretorio, base e sufixo. */
static char *montar_caminho_saida(const char *diretorio, const char *nome_base,
                                  const char *sufixo);

/* Deriva o caminho do arquivo .hfc a partir do .hf. */
static char *montar_caminho_controle(const char *caminho_hash);

/* Libera todas as quadras armazenadas na lista em memoria. */
static void liberar_quadras_em_memoria(Lista quadras);

/* Desmonta o estado interno e remove arquivos quando necessario. */
static void destruir_impl(TrataGeoImpl *trata_geo, bool remover_arquivos);

/* Atualiza o estilo corrente usado nos proximos comandos q. */
static bool atualizar_estilo_atual(TrataGeoImpl *trata_geo, const char *sw,
                                   const char *cfill, const char *cstrk);

/* Inicializa o estilo padrao antes de ler o primeiro cq. */
static bool inicializar_estilo_padrao(TrataGeoImpl *trata_geo);

/* Cria, persiste e registra uma nova quadra no estado da cidade. */
static bool adicionar_quadra(TrataGeoImpl *trata_geo, const char *cep, double x, double y,
                             double w, double h);

/* Processa um comando cq e atualiza o estilo atual. */
static bool executa_comando_cq(TrataGeoImpl *trata_geo, const char *linha);

/* Processa um comando q e insere a quadra correspondente. */
static bool executa_comando_q(TrataGeoImpl *trata_geo, const char *linha);

/* Decide qual comando da linha deve ser executado. */
static bool executa_linha_geo(TrataGeoImpl *trata_geo, const char *linha);

/* Percorre todas as linhas do .geo e executa seus comandos. */
static bool processar_linhas_geo(TrataGeoImpl *trata_geo, DadosDoArquivo dados_geo);

/* Calcula a area minima que contem todas as quadras para o viewBox. */
static void calcular_limites_svg(Lista quadras, double *min_x, double *min_y,
                                 double *max_x, double *max_y);
/* Gera o SVG inicial com o estado da cidade apos o .geo. */
static bool escrever_svg_inicial(TrataGeoImpl *trata_geo);

/* Gera um dump textual legivel da hash de quadras. */
static bool escrever_dump_quadras(TrataGeoImpl *trata_geo);

/* Inicializa caminhos, listas, estilo e hash usados pelo processamento. */
static bool preparar_estrutura(TrataGeoImpl *trata_geo, DadosDoArquivo dados_geo,
                               const char *caminho_output);

TrataGeo processa_geo(DadosDoArquivo dados_geo, const char *caminho_output) {
    TrataGeoImpl *trata_geo;

    trata_geo = (TrataGeoImpl *)calloc(1, sizeof(TrataGeoImpl));
    if (trata_geo == NULL) {
        return NULL;
    }

    if (!preparar_estrutura(trata_geo, dados_geo, caminho_output) ||
        !processar_linhas_geo(trata_geo, dados_geo) ||
        !escrever_svg_inicial(trata_geo) ||
        !escrever_dump_quadras(trata_geo)) {
        destruir_impl(trata_geo, true);
        return NULL;
    }

    return trata_geo;
}

void trata_geo_destruir(TrataGeo trata_geo) {
    destruir_impl(trata_geo_impl(trata_geo), false);
}

Quadra trata_geo_obter_quadra(TrataGeo trata_geo, const char *cep) {
    TrataGeoImpl *impl = trata_geo_impl(trata_geo);
    size_t tamanho_registro;
    unsigned char *registro;
    Quadra quadra = NULL;

    if (impl == NULL || cep == NULL || impl->quadras == NULL) {
        return NULL;
    }

    tamanho_registro = quadra_tamanho_registro();
    registro = (unsigned char *)calloc(1, tamanho_registro);
    if (registro == NULL) {
        return NULL;
    }

    if (he_buscar(impl->quadras, cep, registro)) {
        quadra = quadra_criar_de_bytes(registro, tamanho_registro);
    }

    free(registro);
    return quadra;
}

const char *trata_geo_obter_nome_geo(TrataGeo trata_geo) {
    TrataGeoImpl *impl = trata_geo_impl(trata_geo);
    return impl == NULL ? NULL : impl->nome_geo;
}

static bool texto_valido(const char *texto, size_t tamanho_maximo) {
    size_t tamanho;

    if (texto == NULL || texto[0] == '\0') {
        return false;
    }

    tamanho = strlen(texto);
    return tamanho > 0u && tamanho <= tamanho_maximo;
}

static bool copiar_texto(char *destino, size_t tamanho_destino, const char *origem) {
    size_t tamanho;

    if (destino == NULL || origem == NULL) {
        return false;
    }

    tamanho = strlen(origem);
    if (tamanho + 1u > tamanho_destino) {
        return false;
    }

    memcpy(destino, origem, tamanho + 1u);
    return true;
}

static const char *pular_espacos(const char *texto) {
    while (texto != NULL && *texto != '\0' && isspace((unsigned char)*texto) != 0) {
        texto++;
    }

    return texto;
}

static bool linha_ignorada(const char *linha) {
    const char *cursor = pular_espacos(linha);
    return cursor != NULL && (*cursor == '\0' || *cursor == '#');
}

static bool resto_valido(const char *trecho) {
    trecho = pular_espacos(trecho);
    return trecho != NULL && (*trecho == '\0' || *trecho == '#');
}

static bool termina_com(const char *texto, const char *sufixo) {
    size_t tamanho_texto;
    size_t tamanho_sufixo;

    if (texto == NULL || sufixo == NULL) {
        return false;
    }

    tamanho_texto = strlen(texto);
    tamanho_sufixo = strlen(sufixo);
    if (tamanho_texto < tamanho_sufixo) {
        return false;
    }

    return strcmp(texto + tamanho_texto - tamanho_sufixo, sufixo) == 0;
}

static char *extrair_nome_base_geo(const char *nome_arquivo) {
    size_t tamanho_base;
    char *nome_base;

    if (!termina_com(nome_arquivo, ".geo")) {
        return NULL;
    }

    tamanho_base = strlen(nome_arquivo) - 4u;
    if (tamanho_base == 0u) {
        return NULL;
    }

    nome_base = (char *)calloc(tamanho_base + 1u, sizeof(char));
    if (nome_base == NULL) {
        return NULL;
    }

    memcpy(nome_base, nome_arquivo, tamanho_base);
    return nome_base;
}

static char *montar_caminho_saida(const char *diretorio, const char *nome_base,
                                  const char *sufixo) {
    size_t tamanho_diretorio;
    size_t tamanho_base;
    size_t tamanho_sufixo;
    bool precisa_barra;
    char *caminho;

    if (diretorio == NULL || nome_base == NULL || sufixo == NULL) {
        return NULL;
    }

    tamanho_diretorio = strlen(diretorio);
    tamanho_base = strlen(nome_base);
    tamanho_sufixo = strlen(sufixo);
    precisa_barra = tamanho_diretorio > 0u && diretorio[tamanho_diretorio - 1u] != '/';

    caminho = (char *)malloc(tamanho_diretorio + (precisa_barra ? 1u : 0u) +
                             tamanho_base + tamanho_sufixo + 1u);
    if (caminho == NULL) {
        return NULL;
    }

    memcpy(caminho, diretorio, tamanho_diretorio);
    if (precisa_barra) {
        caminho[tamanho_diretorio] = '/';
        tamanho_diretorio++;
    }
    memcpy(caminho + tamanho_diretorio, nome_base, tamanho_base);
    memcpy(caminho + tamanho_diretorio + tamanho_base, sufixo, tamanho_sufixo + 1u);
    return caminho;
}

static char *montar_caminho_controle(const char *caminho_hash) {
    size_t tamanho_base;
    char *caminho_controle;

    if (!termina_com(caminho_hash, ".hf")) {
        return NULL;
    }

    tamanho_base = strlen(caminho_hash) - 3u;
    caminho_controle = (char *)malloc(tamanho_base + 5u);
    if (caminho_controle == NULL) {
        return NULL;
    }

    memcpy(caminho_controle, caminho_hash, tamanho_base);
    memcpy(caminho_controle + tamanho_base, ".hfc", 5u);
    return caminho_controle;
}

static void liberar_quadras_em_memoria(Lista quadras) {
    if (quadras == NULL) {
        return;
    }

    while (!listaVazia(quadras)) {
        Quadra quadra = (Quadra)removeInicioLista(quadras);
        quadra_destruir(quadra);
    }

    liberaLista(quadras);
}

static void destruir_impl(TrataGeoImpl *trata_geo, bool remover_arquivos) {
    char *caminho_controle = NULL;

    if (trata_geo == NULL) {
        return;
    }

    if (trata_geo->quadras != NULL) {
        if (!remover_arquivos && trata_geo->caminho_dump_quadras != NULL) {
            he_dump(trata_geo->quadras, trata_geo->caminho_dump_quadras);
        }
        he_fechar(trata_geo->quadras);
        trata_geo->quadras = NULL;
    }

    if (remover_arquivos && trata_geo->caminho_svg_inicial != NULL) {
        remove(trata_geo->caminho_svg_inicial);
    }

    if (remover_arquivos && trata_geo->caminho_hash_quadras != NULL) {
        caminho_controle = montar_caminho_controle(trata_geo->caminho_hash_quadras);
        remove(trata_geo->caminho_hash_quadras);
        if (caminho_controle != NULL) {
            remove(caminho_controle);
        }
    }

    if (remover_arquivos && trata_geo->caminho_dump_quadras != NULL) {
        remove(trata_geo->caminho_dump_quadras);
    }

    free(caminho_controle);
    if (trata_geo->lista_svg != NULL) {
        liberaLista(trata_geo->lista_svg);
    }
    liberar_quadras_em_memoria(trata_geo->lista_quadras);
    free(trata_geo->nome_geo);
    free(trata_geo->caminho_svg_inicial);
    free(trata_geo->caminho_hash_quadras);
    free(trata_geo->caminho_dump_quadras);
    free(trata_geo);
}

static bool atualizar_estilo_atual(TrataGeoImpl *trata_geo, const char *sw,
                                   const char *cfill, const char *cstrk) {
    if (trata_geo == NULL ||
        !texto_valido(sw, QUADRA_TAMANHO_ESPESSURA_MAX) ||
        !texto_valido(cfill, QUADRA_TAMANHO_COR_MAX) ||
        !texto_valido(cstrk, QUADRA_TAMANHO_COR_MAX)) {
        return false;
    }

    return copiar_texto(trata_geo->sw_atual, sizeof(trata_geo->sw_atual), sw) &&
           copiar_texto(trata_geo->cfill_atual, sizeof(trata_geo->cfill_atual), cfill) &&
           copiar_texto(trata_geo->cstrk_atual, sizeof(trata_geo->cstrk_atual), cstrk);
}

static bool inicializar_estilo_padrao(TrataGeoImpl *trata_geo) {
    return atualizar_estilo_atual(trata_geo, TRATA_GEO_SW_INICIAL,
                                  TRATA_GEO_CFILL_INICIAL,
                                  TRATA_GEO_CSTRK_INICIAL);
}

static bool adicionar_quadra(TrataGeoImpl *trata_geo, const char *cep, double x, double y,
                             double w, double h) {
    size_t tamanho_registro;
    unsigned char *registro;
    Quadra quadra;
    bool sucesso = false;

    quadra = quadra_criar(cep, x, y, w, h, trata_geo->sw_atual,
                          trata_geo->cfill_atual, trata_geo->cstrk_atual);
    if (quadra == NULL) {
        return false;
    }

    tamanho_registro = quadra_tamanho_registro();
    registro = (unsigned char *)calloc(1, tamanho_registro);
    if (registro != NULL &&
        quadra_escrever_registro(quadra, registro, tamanho_registro) &&
        he_inserir(trata_geo->quadras, registro)) {
        insereFinalLista(trata_geo->lista_quadras, quadra);
        insereFinalLista(trata_geo->lista_svg, quadra);
        sucesso = true;
    }

    free(registro);
    if (!sucesso) {
        quadra_destruir(quadra);
    }

    return sucesso;
}

static bool executa_comando_cq(TrataGeoImpl *trata_geo, const char *linha) {
    char sw[128];
    char cfill[128];
    char cstrk[128];
    int pos = 0;

    if (sscanf(linha, "cq %127s %127s %127s %n", sw, cfill, cstrk, &pos) != 3 ||
        !resto_valido(linha + pos)) {
        return false;
    }

    return atualizar_estilo_atual(trata_geo, sw, cfill, cstrk);
}

static bool executa_comando_q(TrataGeoImpl *trata_geo, const char *linha) {
    char cep[128];
    double x;
    double y;
    double w;
    double h;
    int pos = 0;

    if (sscanf(linha, "q %127s %lf %lf %lf %lf %n", cep, &x, &y, &w, &h, &pos) != 5 ||
        !resto_valido(linha + pos)) {
        return false;
    }

    return adicionar_quadra(trata_geo, cep, x, y, w, h);
}

static bool executa_linha_geo(TrataGeoImpl *trata_geo, const char *linha) {
    const char *cursor;

    if (trata_geo == NULL || linha == NULL || linha_ignorada(linha)) {
        return true;
    }

    cursor = pular_espacos(linha);
    if (strncmp(cursor, "cq", 2u) == 0 && isspace((unsigned char)cursor[2]) != 0) {
        return executa_comando_cq(trata_geo, cursor);
    }

    if (cursor[0] == 'q' && isspace((unsigned char)cursor[1]) != 0) {
        return executa_comando_q(trata_geo, cursor);
    }

    return false;
}

static bool processar_linhas_geo(TrataGeoImpl *trata_geo, DadosDoArquivo dados_geo) {
    Lista linhas;
    Celula atual;

    if (trata_geo == NULL || dados_geo == NULL) {
        return false;
    }

    linhas = obter_lista_linhas(dados_geo);
    if (linhas == NULL) {
        return false;
    }

    atual = getInicioLista(linhas);
    while (atual != NULL) {
        const char *linha = (const char *)getConteudoCelula(atual);
        if (!executa_linha_geo(trata_geo, linha)) {
            return false;
        }
        atual = getProxCelula(atual);
    }

    return true;
}

static void calcular_limites_svg(Lista quadras, double *min_x, double *min_y,
                                 double *max_x, double *max_y) {
    Celula atual = getInicioLista(quadras);
    bool encontrou = false;

    *min_x = 0.0;
    *min_y = 0.0;
    *max_x = 100.0;
    *max_y = 100.0;

    while (atual != NULL) {
        Quadra quadra = (Quadra)getConteudoCelula(atual);
        double esquerda = quadra_obter_x(quadra) - quadra_obter_w(quadra);
        double topo = quadra_obter_y(quadra) - quadra_obter_h(quadra);
        double direita = quadra_obter_x(quadra);
        double base = quadra_obter_y(quadra);

        if (!encontrou) {
            *min_x = esquerda;
            *min_y = topo;
            *max_x = direita;
            *max_y = base;
            encontrou = true;
        } else {
            if (esquerda < *min_x) {
                *min_x = esquerda;
            }
            if (topo < *min_y) {
                *min_y = topo;
            }
            if (direita > *max_x) {
                *max_x = direita;
            }
            if (base > *max_y) {
                *max_y = base;
            }
        }

        atual = getProxCelula(atual);
    }
}

static bool escrever_svg_inicial(TrataGeoImpl *trata_geo) {
    FILE *arquivo;
    Celula atual;
    double min_x;
    double min_y;
    double max_x;
    double max_y;
    double largura;
    double altura;

    if (trata_geo == NULL || trata_geo->caminho_svg_inicial == NULL ||
        trata_geo->lista_svg == NULL) {
        return false;
    }

    arquivo = fopen(trata_geo->caminho_svg_inicial, "w");
    if (arquivo == NULL) {
        return false;
    }

    calcular_limites_svg(trata_geo->lista_svg, &min_x, &min_y, &max_x, &max_y);
    largura = (max_x - min_x) + 2.0 * TRATA_GEO_MARGEM_SVG;
    altura = (max_y - min_y) + 2.0 * TRATA_GEO_MARGEM_SVG;

    if (fprintf(arquivo,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                "viewBox=\"%.2f %.2f %.2f %.2f\">\n",
                min_x - TRATA_GEO_MARGEM_SVG, min_y - TRATA_GEO_MARGEM_SVG,
                largura, altura) < 0) {
        fclose(arquivo);
        return false;
    }

    atual = getInicioLista(trata_geo->lista_svg);
    while (atual != NULL) {
        Quadra quadra = (Quadra)getConteudoCelula(atual);
        double x = quadra_obter_x(quadra) - quadra_obter_w(quadra);
        double y = quadra_obter_y(quadra) - quadra_obter_h(quadra);
        double texto_x = x + 4.0;
        double texto_y = y + 12.0;

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
                    texto_x, texto_y, quadra_obter_cep(quadra)) < 0) {
            fclose(arquivo);
            return false;
        }

        atual = getProxCelula(atual);
    }

    if (fprintf(arquivo, "</svg>\n") < 0) {
        fclose(arquivo);
        return false;
    }

    fclose(arquivo);
    return true;
}

static bool escrever_dump_quadras(TrataGeoImpl *trata_geo) {
    FILE *arquivo_dump;

    if (trata_geo == NULL || trata_geo->quadras == NULL ||
        trata_geo->caminho_dump_quadras == NULL) {
        return false;
    }

    he_dump(trata_geo->quadras, trata_geo->caminho_dump_quadras);
    arquivo_dump = fopen(trata_geo->caminho_dump_quadras, "r");
    if (arquivo_dump == NULL) {
        return false;
    }

    fclose(arquivo_dump);
    return true;
}

static bool preparar_estrutura(TrataGeoImpl *trata_geo, DadosDoArquivo dados_geo,
                               const char *caminho_output) {
    const char *nome_arquivo_geo;

    if (trata_geo == NULL || dados_geo == NULL || caminho_output == NULL) {
        return false;
    }

    nome_arquivo_geo = obter_nome_arquivo(dados_geo);
    trata_geo->nome_geo = extrair_nome_base_geo(nome_arquivo_geo);
    if (trata_geo->nome_geo == NULL) {
        return false;
    }

    trata_geo->caminho_svg_inicial =
        montar_caminho_saida(caminho_output, trata_geo->nome_geo, ".svg");
    trata_geo->caminho_hash_quadras =
        montar_caminho_saida(caminho_output, trata_geo->nome_geo, "-quadras.hf");
    trata_geo->caminho_dump_quadras =
        montar_caminho_saida(caminho_output, trata_geo->nome_geo, "-quadras.hfd");
    trata_geo->lista_quadras = criaLista();
    trata_geo->lista_svg = criaLista();

    if (trata_geo->caminho_svg_inicial == NULL || trata_geo->caminho_hash_quadras == NULL ||
        trata_geo->caminho_dump_quadras == NULL || trata_geo->lista_quadras == NULL ||
        trata_geo->lista_svg == NULL ||
        !inicializar_estilo_padrao(trata_geo)) {
        return false;
    }

    trata_geo->quadras =
        he_criar(trata_geo->caminho_hash_quadras, TRATA_GEO_CAPACIDADE_BUCKET,
                 quadra_tamanho_registro());
    return trata_geo->quadras != NULL;
}
