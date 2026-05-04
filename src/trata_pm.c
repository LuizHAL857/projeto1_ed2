#include "../include/trata_pm.h"

#include "../include/hash_extensivel.h"
#include "../include/lista.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRATA_PM_CAPACIDADE_BUCKET 2u

typedef struct {
    HashExtensivel habitantes;
    Lista lista_habitantes;
    TrataGeo trata_geo;
    char *nome_pm;
    char *caminho_hash_habitantes;
    char *caminho_dump_habitantes;
} TrataPmImpl;

static TrataPmImpl *trata_pm_impl(TrataPm trata_pm) {
    return (TrataPmImpl *)trata_pm;
}

static Habitante clonar_habitante(Habitante habitante);
static char *montar_caminho_controle(const char *caminho_hash);
static void liberar_habitantes_em_memoria(Lista habitantes);
static void destruir_impl(TrataPmImpl *trata_pm, bool remover_arquivos);
static bool habitante_existe(TrataPmImpl *trata_pm, const char *cpf);
static Habitante buscar_habitante_em_memoria(TrataPmImpl *trata_pm, const char *cpf);
static bool cep_existe(TrataPmImpl *trata_pm, const char *cep);
static bool adicionar_habitante(TrataPmImpl *trata_pm, const char *cpf, const char *nome,
                                const char *sobrenome, char sexo, const char *nasc);
static bool definir_moradia_habitante(TrataPmImpl *trata_pm, const char *cpf, const char *cep,
                                      char face, int num, const char *compl);
static bool executa_comando_p(TrataPmImpl *trata_pm, const char *linha);
static bool executa_comando_m(TrataPmImpl *trata_pm, const char *linha);
static bool executa_linha_pm(TrataPmImpl *trata_pm, const char *linha);
static bool processar_linhas_pm(TrataPmImpl *trata_pm, DadosDoArquivo dados_pm);
static bool preparar_estrutura(TrataPmImpl *trata_pm, DadosDoArquivo dados_pm,
                               TrataGeo trata_geo, const char *caminho_output,
                               const char *nome_qry);

TrataPm processa_pm(DadosDoArquivo dados_pm, TrataGeo trata_geo,
                    const char *caminho_output, const char *nome_qry) {
    TrataPmImpl *trata_pm;

    trata_pm = (TrataPmImpl *)calloc(1, sizeof(TrataPmImpl));
    if (trata_pm == NULL) {
        return NULL;
    }

    if (!preparar_estrutura(trata_pm, dados_pm, trata_geo, caminho_output, nome_qry) ||
        !processar_linhas_pm(trata_pm, dados_pm)) {
        destruir_impl(trata_pm, true);
        return NULL;
    }

    return trata_pm;
}

void trata_pm_destruir(TrataPm trata_pm) {
    destruir_impl(trata_pm_impl(trata_pm), false);
}

void trata_pm_liberar_habitantes(Lista habitantes) {
    liberar_habitantes_em_memoria(habitantes);
}

Habitante trata_pm_obter_habitante(TrataPm trata_pm, const char *cpf) {
    TrataPmImpl *impl = trata_pm_impl(trata_pm);
    size_t tamanho_registro;
    unsigned char *registro;
    Habitante habitante = NULL;

    if (impl == NULL || cpf == NULL || impl->habitantes == NULL) {
        return NULL;
    }

    tamanho_registro = habitante_tamanho_registro();
    registro = (unsigned char *)calloc(1, tamanho_registro);
    if (registro == NULL) {
        return NULL;
    }

    if (he_buscar(impl->habitantes, cpf, registro)) {
        habitante = habitante_criar_de_bytes(registro, tamanho_registro);
    }

    free(registro);
    return habitante;
}

const char *trata_pm_obter_nome_pm(TrataPm trata_pm) {
    TrataPmImpl *impl = trata_pm_impl(trata_pm);
    return impl == NULL ? NULL : impl->nome_pm;
}

bool trata_pm_inserir_habitante(TrataPm trata_pm, const char *cpf, const char *nome,
                                const char *sobrenome, char sexo, const char *nasc) {
    TrataPmImpl *impl = trata_pm_impl(trata_pm);

    if (impl == NULL) {
        return false;
    }

    return adicionar_habitante(impl, cpf, nome, sobrenome, sexo, nasc);
}

bool trata_pm_definir_moradia(TrataPm trata_pm, const char *cpf, const char *cep,
                              char face, int num, const char *compl) {
    TrataPmImpl *impl = trata_pm_impl(trata_pm);

    if (impl == NULL) {
        return false;
    }

    return definir_moradia_habitante(impl, cpf, cep, face, num, compl);
}

bool trata_pm_remover_moradia(TrataPm trata_pm, const char *cpf) {
    TrataPmImpl *impl = trata_pm_impl(trata_pm);
    Habitante habitante;
    size_t tamanho_registro;
    unsigned char *registro;
    bool sucesso;

    if (impl == NULL || !habitante_existe(impl, cpf)) {
        return false;
    }

    habitante = buscar_habitante_em_memoria(impl, cpf);
    if (habitante == NULL || !habitante_eh_morador(habitante)) {
        return false;
    }

    tamanho_registro = habitante_tamanho_registro();
    registro = (unsigned char *)calloc(1, tamanho_registro);
    if (registro == NULL) {
        return false;
    }

    habitante_remover_moradia(habitante);
    sucesso = habitante_escrever_registro(habitante, registro, tamanho_registro) &&
              he_atualizar(impl->habitantes, registro);
    free(registro);
    return sucesso;
}

bool trata_pm_remover_habitante(TrataPm trata_pm, const char *cpf) {
    TrataPmImpl *impl = trata_pm_impl(trata_pm);
    Habitante habitante;

    if (impl == NULL || !habitante_existe(impl, cpf)) {
        return false;
    }

    habitante = buscar_habitante_em_memoria(impl, cpf);
    if (habitante == NULL || !he_remover(impl->habitantes, cpf) ||
        !removeElementoLista(impl->lista_habitantes, habitante)) {
        return false;
    }

    habitante_destruir(habitante);
    return true;
}

Lista trata_pm_listar_habitantes(TrataPm trata_pm) {
    TrataPmImpl *impl = trata_pm_impl(trata_pm);
    Lista copia;
    Celula atual;

    if (impl == NULL || impl->lista_habitantes == NULL) {
        return NULL;
    }

    copia = criaLista();
    if (copia == NULL) {
        return NULL;
    }

    atual = getInicioLista(impl->lista_habitantes);
    while (atual != NULL) {
        Habitante original = (Habitante)getConteudoCelula(atual);
        Habitante clone = clonar_habitante(original);

        if (clone == NULL) {
            liberar_habitantes_em_memoria(copia);
            return NULL;
        }

        insereFinalLista(copia, clone);
        atual = getProxCelula(atual);
    }

    return copia;
}

Lista trata_pm_tornar_sem_teto_por_cep(TrataPm trata_pm, const char *cep) {
    TrataPmImpl *impl = trata_pm_impl(trata_pm);
    Lista afetados;
    Lista originais;
    Celula atual;
    Celula cursor;

    if (impl == NULL || impl->lista_habitantes == NULL || impl->habitantes == NULL ||
        cep == NULL) {
        return NULL;
    }

    afetados = criaLista();
    originais = criaLista();
    if (afetados == NULL || originais == NULL) {
        liberaLista(afetados);
        liberaLista(originais);
        return NULL;
    }

    atual = getInicioLista(impl->lista_habitantes);
    while (atual != NULL) {
        Habitante habitante = (Habitante)getConteudoCelula(atual);

        if (habitante != NULL && habitante_eh_morador(habitante) &&
            strcmp(habitante_obter_cep(habitante), cep) == 0) {
            Habitante clone = clonar_habitante(habitante);

            if (clone == NULL) {
                liberar_habitantes_em_memoria(afetados);
                liberaLista(originais);
                return NULL;
            }

            insereFinalLista(afetados, clone);
            insereFinalLista(originais, habitante);
        }

        atual = getProxCelula(atual);
    }

    cursor = getInicioLista(originais);
    while (cursor != NULL) {
        Habitante habitante = (Habitante)getConteudoCelula(cursor);
        size_t tamanho_registro = habitante_tamanho_registro();
        unsigned char *registro = (unsigned char *)calloc(1, tamanho_registro);
        bool sucesso;

        if (registro == NULL) {
            liberar_habitantes_em_memoria(afetados);
            liberaLista(originais);
            return NULL;
        }

        habitante_remover_moradia(habitante);
        sucesso = habitante_escrever_registro(habitante, registro, tamanho_registro) &&
                  he_atualizar(impl->habitantes, registro);
        free(registro);

        if (!sucesso) {
            liberar_habitantes_em_memoria(afetados);
            liberaLista(originais);
            return NULL;
        }

        cursor = getProxCelula(cursor);
    }

    liberaLista(originais);
    return afetados;
}

static Habitante clonar_habitante(Habitante habitante) {
    size_t tamanho_registro;
    unsigned char *registro;
    Habitante clone = NULL;

    if (habitante == NULL) {
        return NULL;
    }

    tamanho_registro = habitante_tamanho_registro();
    registro = (unsigned char *)calloc(1, tamanho_registro);
    if (registro == NULL) {
        return NULL;
    }

    if (habitante_escrever_registro(habitante, registro, tamanho_registro)) {
        clone = habitante_criar_de_bytes(registro, tamanho_registro);
    }

    free(registro);
    return clone;
}

static char *montar_caminho_controle(const char *caminho_hash) {
    size_t tamanho_base;
    char *caminho_controle;

    if (!arquivo_termina_com(caminho_hash, ".hf")) {
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

static void liberar_habitantes_em_memoria(Lista habitantes) {
    if (habitantes == NULL) {
        return;
    }

    while (!listaVazia(habitantes)) {
        Habitante habitante = (Habitante)removeInicioLista(habitantes);
        habitante_destruir(habitante);
    }

    liberaLista(habitantes);
}

static void destruir_impl(TrataPmImpl *trata_pm, bool remover_arquivos) {
    char *controle_habitantes = NULL;

    if (trata_pm == NULL) {
        return;
    }

    if (trata_pm->habitantes != NULL) {
        if (!remover_arquivos && trata_pm->caminho_dump_habitantes != NULL) {
            he_dump(trata_pm->habitantes, trata_pm->caminho_dump_habitantes);
        }
        he_fechar(trata_pm->habitantes);
        trata_pm->habitantes = NULL;
    }

    if (remover_arquivos && trata_pm->caminho_hash_habitantes != NULL) {
        controle_habitantes = montar_caminho_controle(trata_pm->caminho_hash_habitantes);
        remove(trata_pm->caminho_hash_habitantes);
        if (controle_habitantes != NULL) {
            remove(controle_habitantes);
        }
    }

    if (remover_arquivos && trata_pm->caminho_dump_habitantes != NULL) {
        remove(trata_pm->caminho_dump_habitantes);
    }

    free(controle_habitantes);
    liberar_habitantes_em_memoria(trata_pm->lista_habitantes);
    free(trata_pm->nome_pm);
    free(trata_pm->caminho_hash_habitantes);
    free(trata_pm->caminho_dump_habitantes);
    free(trata_pm);
}

static bool habitante_existe(TrataPmImpl *trata_pm, const char *cpf) {
    return trata_pm != NULL && trata_pm->habitantes != NULL && cpf != NULL &&
           he_contem(trata_pm->habitantes, cpf);
}

static Habitante buscar_habitante_em_memoria(TrataPmImpl *trata_pm, const char *cpf) {
    Celula atual;

    if (trata_pm == NULL || trata_pm->lista_habitantes == NULL || cpf == NULL) {
        return NULL;
    }

    atual = getInicioLista(trata_pm->lista_habitantes);
    while (atual != NULL) {
        Habitante habitante = (Habitante)getConteudoCelula(atual);
        if (habitante != NULL &&
            strcmp(habitante_obter_cpf(habitante), cpf) == 0) {
            return habitante;
        }
        atual = getProxCelula(atual);
    }

    return NULL;
}

static bool cep_existe(TrataPmImpl *trata_pm, const char *cep) {
    Quadra quadra;
    bool existe;

    if (trata_pm == NULL || cep == NULL) {
        return false;
    }

    quadra = trata_geo_obter_quadra(trata_pm->trata_geo, cep);
    existe = quadra != NULL;
    quadra_destruir(quadra);
    return existe;
}

static bool adicionar_habitante(TrataPmImpl *trata_pm, const char *cpf, const char *nome,
                                const char *sobrenome, char sexo, const char *nasc) {
    size_t tamanho_registro;
    unsigned char *registro;
    Habitante habitante;
    bool sucesso = false;

    if (trata_pm == NULL) {
        return false;
    }

    habitante = habitante_criar(cpf, nome, sobrenome, sexo, nasc);
    if (habitante == NULL) {
        return false;
    }

    tamanho_registro = habitante_tamanho_registro();
    registro = (unsigned char *)calloc(1, tamanho_registro);
    if (registro != NULL &&
        habitante_escrever_registro(habitante, registro, tamanho_registro) &&
        he_inserir(trata_pm->habitantes, registro)) {
        insereFinalLista(trata_pm->lista_habitantes, habitante);
        sucesso = true;
    }

    free(registro);
    if (!sucesso) {
        habitante_destruir(habitante);
    }

    return sucesso;
}

static bool definir_moradia_habitante(TrataPmImpl *trata_pm, const char *cpf, const char *cep,
                                      char face, int num, const char *compl) {
    size_t tamanho_registro;
    unsigned char *registro;
    Habitante habitante;
    bool sucesso;

    if (trata_pm == NULL || !habitante_existe(trata_pm, cpf) || !cep_existe(trata_pm, cep)) {
        return false;
    }

    habitante = buscar_habitante_em_memoria(trata_pm, cpf);
    if (habitante == NULL ||
        !habitante_definir_moradia(habitante, cep, face, num, compl)) {
        return false;
    }

    tamanho_registro = habitante_tamanho_registro();
    registro = (unsigned char *)calloc(1, tamanho_registro);
    sucesso = registro != NULL &&
              habitante_escrever_registro(habitante, registro, tamanho_registro) &&
              he_atualizar(trata_pm->habitantes, registro);
    free(registro);

    return sucesso;
}

static bool executa_comando_p(TrataPmImpl *trata_pm, const char *linha) {
    char cpf[128];
    char nome[128];
    char sobrenome[128];
    char sexo;
    char nasc[128];
    int pos = 0;

    if (sscanf(linha, "p %127s %127s %127s %c %127s %n",
               cpf, nome, sobrenome, &sexo, nasc, &pos) != 5 ||
        !arquivo_resto_valido(linha + pos)) {
        return false;
    }

    return adicionar_habitante(trata_pm, cpf, nome, sobrenome, sexo, nasc);
}

static bool executa_comando_m(TrataPmImpl *trata_pm, const char *linha) {
    char cpf[128];
    char cep[128];
    char face;
    int num;
    char compl[128];
    int pos = 0;

    if (sscanf(linha, "m %127s %127s %c %d %127s %n",
               cpf, cep, &face, &num, compl, &pos) != 5 ||
        !arquivo_resto_valido(linha + pos)) {
        return false;
    }

    return definir_moradia_habitante(trata_pm, cpf, cep, face, num, compl);
}

static bool executa_linha_pm(TrataPmImpl *trata_pm, const char *linha) {
    const char *cursor;

    if (trata_pm == NULL || linha == NULL || arquivo_linha_ignorada(linha)) {
        return true;
    }

    cursor = arquivo_pular_espacos(linha);
    if (cursor[0] == 'p' && isspace((unsigned char)cursor[1]) != 0) {
        return executa_comando_p(trata_pm, cursor);
    }

    if (cursor[0] == 'm' && isspace((unsigned char)cursor[1]) != 0) {
        return executa_comando_m(trata_pm, cursor);
    }

    return false;
}

static bool processar_linhas_pm(TrataPmImpl *trata_pm, DadosDoArquivo dados_pm) {
    Lista linhas;
    Celula atual;

    if (trata_pm == NULL || dados_pm == NULL) {
        return false;
    }

    linhas = obter_lista_linhas(dados_pm);
    if (linhas == NULL) {
        return false;
    }

    atual = getInicioLista(linhas);
    while (atual != NULL) {
        const char *linha = (const char *)getConteudoCelula(atual);
        if (!executa_linha_pm(trata_pm, linha)) {
            return false;
        }
        atual = getProxCelula(atual);
    }

    return true;
}

static bool preparar_estrutura(TrataPmImpl *trata_pm, DadosDoArquivo dados_pm,
                               TrataGeo trata_geo, const char *caminho_output,
                               const char *nome_qry) {
    const char *nome_arquivo_pm;
    char *nome_base_persistencia;

    if (trata_pm == NULL || dados_pm == NULL || trata_geo == NULL ||
        caminho_output == NULL) {
        return false;
    }

    nome_arquivo_pm = obter_nome_arquivo(dados_pm);
    trata_pm->trata_geo = trata_geo;
    trata_pm->nome_pm = arquivo_extrair_nome_base(nome_arquivo_pm, ".pm");
    if (trata_pm->nome_pm == NULL) {
        return false;
    }

    nome_base_persistencia =
        arquivo_montar_nome_composto(trata_pm->nome_pm, "-", nome_qry);
    if (nome_base_persistencia == NULL) {
        return false;
    }

    trata_pm->caminho_hash_habitantes =
        arquivo_montar_caminho_saida(caminho_output, nome_base_persistencia,
                                     "-habitantes.hf");
    trata_pm->caminho_dump_habitantes =
        arquivo_montar_caminho_saida(caminho_output, nome_base_persistencia,
                                     "-habitantes.hfd");
    trata_pm->lista_habitantes = criaLista();
    free(nome_base_persistencia);

    if (trata_pm->caminho_hash_habitantes == NULL ||
        trata_pm->caminho_dump_habitantes == NULL ||
        trata_pm->lista_habitantes == NULL) {
        return false;
    }

    trata_pm->habitantes =
        he_criar(trata_pm->caminho_hash_habitantes, TRATA_PM_CAPACIDADE_BUCKET,
                 habitante_tamanho_registro());

    return trata_pm->habitantes != NULL;
}
