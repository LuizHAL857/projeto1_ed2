#include "../include/hash_extensivel.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HE_MAGIC 0x48455837u
#define HE_VERSAO 1u

typedef struct HE_PACKED {
    uint32_t magic;
    uint32_t versao;
    uint64_t profundidade_global;
    uint64_t capacidade_bucket;
    uint64_t tamanho_registro;
    uint64_t tamanho;
    uint64_t quantidade_expansoes;
} CabecalhoIndice;

typedef struct HE_PACKED {
    uint64_t profundidade_local;
    uint64_t quantidade;
    unsigned char dados[];
} BucketDisco;

typedef struct HE_PACKED {
    int64_t offset_bucket_antigo;
    int64_t offset_bucket_novo;
    uint64_t profundidade_local_antiga;
    uint64_t profundidade_local_nova;
    uint64_t profundidade_global_apos;
} ExpansaoBucketDisco;

typedef struct {
    FILE *arquivo_dados;
    FILE *arquivo_indice;
    char *caminho_hf;
    char *caminho_hfc;
    uint64_t profundidade_global;
    uint64_t capacidade_bucket;
    uint64_t tamanho_registro;
    uint64_t tamanho;
    int64_t *diretorio;
    ExpansaoBucketDisco *expansoes;
    uint64_t quantidade_expansoes;
} HashExtensivelImpl;

static HashExtensivelImpl *he_impl(HashExtensivel he) {
    return (HashExtensivelImpl *)he;
}

static int termina_com(const char *texto, const char *sufixo) {
    size_t tamanho_texto;
    size_t tamanho_sufixo;

    if (texto == NULL || sufixo == NULL) {
        return 0;
    }

    tamanho_texto = strlen(texto);
    tamanho_sufixo = strlen(sufixo);
    if (tamanho_texto < tamanho_sufixo) {
        return 0;
    }

    return strcmp(texto + tamanho_texto - tamanho_sufixo, sufixo) == 0;
}

static char *duplicar_texto(const char *texto) {
    size_t tamanho;
    char *copia;

    if (texto == NULL) {
        return NULL;
    }

    tamanho = strlen(texto) + 1u;
    copia = (char *)malloc(tamanho);
    if (copia != NULL) {
        memcpy(copia, texto, tamanho);
    }

    return copia;
}

static char *montar_caminho_auxiliar(const char *caminho_hf, const char *nova_extensao) {
    size_t tamanho_base;
    size_t tamanho_extensao;
    char *caminho;

    if (!termina_com(caminho_hf, ".hf") || nova_extensao == NULL) {
        return NULL;
    }

    tamanho_base = strlen(caminho_hf) - 3u;
    tamanho_extensao = strlen(nova_extensao);
    caminho = (char *)malloc(tamanho_base + tamanho_extensao + 1u);
    if (caminho == NULL) {
        return NULL;
    }

    memcpy(caminho, caminho_hf, tamanho_base);
    memcpy(caminho + tamanho_base, nova_extensao, tamanho_extensao + 1u);
    return caminho;
}

static int ir_para(FILE *arquivo, int64_t offset) {
    return arquivo != NULL && fseek(arquivo, (long)offset, SEEK_SET) == 0;
}

static size_t potencia2(uint64_t expoente) {
    return ((size_t)1u) << expoente;
}

static size_t bytes_entrada(const HashExtensivelImpl *he) {
    return (size_t)he->tamanho_registro;
}

static size_t bytes_bucket(const HashExtensivelImpl *he) {
    return sizeof(BucketDisco) + (size_t)he->capacidade_bucket * bytes_entrada(he);
}

static unsigned char *entrada_ptr(const HashExtensivelImpl *he, BucketDisco *bucket,
                                  size_t indice) {
    return bucket->dados + indice * bytes_entrada(he);
}

static const unsigned char *entrada_ptr_const(const HashExtensivelImpl *he,
                                              const BucketDisco *bucket,
                                              size_t indice) {
    return bucket->dados + indice * bytes_entrada(he);
}

static size_t chave_tamanho_no_bloco(const unsigned char *bloco, size_t limite) {
    size_t i;

    for (i = 0; i < limite; i++) {
        if (bloco[i] == '\0') {
            return i;
        }
    }

    return limite;
}

static int chave_valida_texto(const char *chave) {
    size_t i;
    size_t tamanho;

    if (chave == NULL) {
        return 0;
    }

    tamanho = strlen(chave);
    if (tamanho == 0 || tamanho > HE_TAMANHO_CHAVE_MAX) {
        return 0;
    }

    for (i = 0; i < tamanho; i++) {
        unsigned char c = (unsigned char)chave[i];
        if (!isalnum(c) && c != '-' && c != '.') {
            return 0;
        }
    }

    return 1;
}

static int extrair_chave_do_registro(const HashExtensivelImpl *he, const void *registro,
                                     char chave_out[HE_TAMANHO_CHAVE_MAX + 1]) {
    size_t tamanho;

    if (he == NULL || registro == NULL || chave_out == NULL || he->tamanho_registro == 0) {
        return 0;
    }

    tamanho = chave_tamanho_no_bloco((const unsigned char *)registro,
                                     (size_t)he->tamanho_registro);
    if (tamanho == 0 || tamanho > HE_TAMANHO_CHAVE_MAX) {
        return 0;
    }
    if (tamanho == (size_t)he->tamanho_registro) {
        return 0;
    }

    memcpy(chave_out, registro, tamanho);
    chave_out[tamanho] = '\0';
    return chave_valida_texto(chave_out);
}

static uint64_t hash_texto(const char *chave) {
    uint64_t hash = 0;
    size_t i;

    for (i = 0; chave[i] != '\0'; i++) {
        hash = hash * 31u + (unsigned char)chave[i];
    }

    return hash;
}

static size_t indice_diretorio(const HashExtensivelImpl *he, const char *chave) {
    if (he->profundidade_global == 0) {
        return 0;
    }

    return (size_t)(hash_texto(chave) & (uint64_t)(potencia2(he->profundidade_global) - 1u));
}

static BucketDisco *bucket_criar(const HashExtensivelImpl *he, size_t profundidade_local) {
    BucketDisco *bucket = (BucketDisco *)calloc(1, bytes_bucket(he));

    if (bucket != NULL) {
        bucket->profundidade_local = profundidade_local;
    }

    return bucket;
}

static BucketDisco *bucket_carregar(HashExtensivelImpl *he, int64_t offset) {
    BucketDisco *bucket;

    if (he == NULL || he->arquivo_dados == NULL) {
        return NULL;
    }

    bucket = (BucketDisco *)malloc(bytes_bucket(he));
    if (bucket == NULL) {
        return NULL;
    }

    if (!ir_para(he->arquivo_dados, offset) ||
        fread(bucket, bytes_bucket(he), 1, he->arquivo_dados) != 1) {
        free(bucket);
        return NULL;
    }

    return bucket;
}

static bool bucket_salvar(HashExtensivelImpl *he, int64_t offset,
                          const BucketDisco *bucket) {
    return he != NULL && bucket != NULL && ir_para(he->arquivo_dados, offset) &&
           fwrite(bucket, bytes_bucket(he), 1, he->arquivo_dados) == 1 &&
           fflush(he->arquivo_dados) == 0;
}

static int64_t bucket_anexar(HashExtensivelImpl *he, const BucketDisco *bucket) {
    int64_t offset;

    if (he == NULL || he->arquivo_dados == NULL || bucket == NULL ||
        fseek(he->arquivo_dados, 0L, SEEK_END) != 0) {
        return -1;
    }

    offset = ftell(he->arquivo_dados);
    if (offset < 0) {
        return -1;
    }

    if (fwrite(bucket, bytes_bucket(he), 1, he->arquivo_dados) != 1 ||
        fflush(he->arquivo_dados) != 0) {
        return -1;
    }

    return offset;
}

static int bucket_buscar_posicao(const HashExtensivelImpl *he,
                                 const BucketDisco *bucket, const char *chave) {
    size_t i;
    char chave_registro[HE_TAMANHO_CHAVE_MAX + 1];

    if (he == NULL || bucket == NULL || chave == NULL) {
        return -1;
    }

    for (i = 0; i < bucket->quantidade; i++) {
        if (extrair_chave_do_registro(he, entrada_ptr_const(he, bucket, i), chave_registro) &&
            strcmp(chave_registro, chave) == 0) {
            return (int)i;
        }
    }

    return -1;
}

static bool registrar_expansao(HashExtensivelImpl *he, int64_t offset_antigo,
                               int64_t offset_novo, uint64_t profundidade_local_antiga,
                               uint64_t profundidade_local_nova) {
    ExpansaoBucketDisco *novas_expansoes;
    ExpansaoBucketDisco *evento;

    if (he == NULL) {
        return false;
    }

    novas_expansoes = (ExpansaoBucketDisco *)realloc(
        he->expansoes,
        sizeof(ExpansaoBucketDisco) * (size_t)(he->quantidade_expansoes + 1u));
    if (novas_expansoes == NULL) {
        return false;
    }

    he->expansoes = novas_expansoes;
    evento = &he->expansoes[he->quantidade_expansoes];
    evento->offset_bucket_antigo = offset_antigo;
    evento->offset_bucket_novo = offset_novo;
    evento->profundidade_local_antiga = profundidade_local_antiga;
    evento->profundidade_local_nova = profundidade_local_nova;
    evento->profundidade_global_apos = he->profundidade_global;
    he->quantidade_expansoes++;
    return true;
}

static bool escrever_indice(HashExtensivelImpl *he) {
    CabecalhoIndice cabecalho;
    size_t tamanho_diretorio;

    if (he == NULL || he->arquivo_indice == NULL) {
        return false;
    }

    cabecalho.magic = HE_MAGIC;
    cabecalho.versao = HE_VERSAO;
    cabecalho.profundidade_global = he->profundidade_global;
    cabecalho.capacidade_bucket = he->capacidade_bucket;
    cabecalho.tamanho_registro = he->tamanho_registro;
    cabecalho.tamanho = he->tamanho;
    cabecalho.quantidade_expansoes = he->quantidade_expansoes;
    tamanho_diretorio = potencia2(he->profundidade_global);

    return ir_para(he->arquivo_indice, 0) &&
           fwrite(&cabecalho, sizeof(cabecalho), 1, he->arquivo_indice) == 1 &&
           fwrite(he->diretorio, sizeof(int64_t), tamanho_diretorio, he->arquivo_indice) ==
               tamanho_diretorio &&
           fwrite(he->expansoes, sizeof(ExpansaoBucketDisco),
                  (size_t)he->quantidade_expansoes, he->arquivo_indice) ==
               (size_t)he->quantidade_expansoes &&
           fflush(he->arquivo_indice) == 0;
}

static bool carregar_indice(HashExtensivelImpl *he, size_t capacidade_bucket,
                            size_t tamanho_registro) {
    CabecalhoIndice cabecalho;
    size_t tamanho_diretorio;

    if (he == NULL || he->arquivo_indice == NULL || !ir_para(he->arquivo_indice, 0)) {
        return false;
    }

    if (fread(&cabecalho, sizeof(cabecalho), 1, he->arquivo_indice) != 1 ||
        cabecalho.magic != HE_MAGIC || cabecalho.versao != HE_VERSAO ||
        cabecalho.capacidade_bucket != capacidade_bucket ||
        cabecalho.tamanho_registro != tamanho_registro) {
        return false;
    }

    he->profundidade_global = cabecalho.profundidade_global;
    he->capacidade_bucket = cabecalho.capacidade_bucket;
    he->tamanho_registro = cabecalho.tamanho_registro;
    he->tamanho = cabecalho.tamanho;
    he->quantidade_expansoes = cabecalho.quantidade_expansoes;

    tamanho_diretorio = potencia2(he->profundidade_global);
    he->diretorio = (int64_t *)malloc(sizeof(int64_t) * tamanho_diretorio);
    if (he->diretorio == NULL) {
        return false;
    }

    if (fread(he->diretorio, sizeof(int64_t), tamanho_diretorio, he->arquivo_indice) !=
        tamanho_diretorio) {
        free(he->diretorio);
        he->diretorio = NULL;
        return false;
    }

    if (he->quantidade_expansoes > 0) {
        he->expansoes = (ExpansaoBucketDisco *)malloc(sizeof(ExpansaoBucketDisco) *
                                                      (size_t)he->quantidade_expansoes);
        if (he->expansoes == NULL) {
            free(he->diretorio);
            he->diretorio = NULL;
            return false;
        }

        if (fread(he->expansoes, sizeof(ExpansaoBucketDisco),
                  (size_t)he->quantidade_expansoes, he->arquivo_indice) !=
            (size_t)he->quantidade_expansoes) {
            free(he->expansoes);
            he->expansoes = NULL;
            free(he->diretorio);
            he->diretorio = NULL;
            return false;
        }
    }

    return true;
}

static bool duplicar_diretorio(HashExtensivelImpl *he) {
    size_t tamanho_antigo;
    size_t i;
    int64_t *novo_diretorio;

    if (he == NULL || he->diretorio == NULL) {
        return false;
    }

    tamanho_antigo = potencia2(he->profundidade_global);
    novo_diretorio = (int64_t *)malloc(sizeof(int64_t) * 2u * tamanho_antigo);
    if (novo_diretorio == NULL) {
        return false;
    }

    for (i = 0; i < tamanho_antigo; i++) {
        novo_diretorio[i] = he->diretorio[i];
        novo_diretorio[i + tamanho_antigo] = he->diretorio[i];
    }

    free(he->diretorio);
    he->diretorio = novo_diretorio;
    he->profundidade_global++;
    return escrever_indice(he);
}

static bool dividir_bucket(HashExtensivelImpl *he, const char *chave_referencia) {
    int64_t offset_antigo;
    int64_t offset_novo;
    BucketDisco *bucket_antigo;
    BucketDisco *bucket_novo;
    unsigned char *temporario;
    size_t quantidade_antiga;
    size_t bit_divisao;
    size_t tamanho_diretorio;
    uint64_t profundidade_local_antiga;
    size_t i;

    if (he == NULL || he->diretorio == NULL || chave_referencia == NULL) {
        return false;
    }

    offset_antigo = he->diretorio[indice_diretorio(he, chave_referencia)];
    bucket_antigo = bucket_carregar(he, offset_antigo);
    bucket_novo = bucket_criar(he, 0);
    if (bucket_antigo == NULL || bucket_novo == NULL) {
        free(bucket_antigo);
        free(bucket_novo);
        return false;
    }

    if (bucket_antigo->profundidade_local == he->profundidade_global &&
        !duplicar_diretorio(he)) {
        free(bucket_antigo);
        free(bucket_novo);
        return false;
    }

    quantidade_antiga = (size_t)bucket_antigo->quantidade;
    profundidade_local_antiga = bucket_antigo->profundidade_local;
    temporario = (unsigned char *)malloc(quantidade_antiga * bytes_entrada(he));
    if (temporario == NULL) {
        free(bucket_antigo);
        free(bucket_novo);
        return false;
    }

    memcpy(temporario, bucket_antigo->dados, quantidade_antiga * bytes_entrada(he));
    bucket_novo->profundidade_local = ++bucket_antigo->profundidade_local;
    bucket_antigo->quantidade = 0;
    memset(bucket_antigo->dados, 0, (size_t)he->capacidade_bucket * bytes_entrada(he));
    memset(bucket_novo->dados, 0, (size_t)he->capacidade_bucket * bytes_entrada(he));

    offset_novo = bucket_anexar(he, bucket_novo);
    if (offset_novo < 0) {
        free(temporario);
        free(bucket_antigo);
        free(bucket_novo);
        return false;
    }

    bit_divisao = ((size_t)1u) << (bucket_antigo->profundidade_local - 1u);
    tamanho_diretorio = potencia2(he->profundidade_global);
    for (i = 0; i < tamanho_diretorio; i++) {
        if (he->diretorio[i] == offset_antigo && (i & bit_divisao) != 0u) {
            he->diretorio[i] = offset_novo;
        }
    }

    for (i = 0; i < quantidade_antiga; i++) {
        const unsigned char *registro_origem = temporario + i * bytes_entrada(he);
        char chave[HE_TAMANHO_CHAVE_MAX + 1];
        BucketDisco *destino;
        unsigned char *registro_destino;

        if (!extrair_chave_do_registro(he, registro_origem, chave)) {
            free(temporario);
            free(bucket_antigo);
            free(bucket_novo);
            return false;
        }

        destino = he->diretorio[indice_diretorio(he, chave)] == offset_novo ?
                      bucket_novo : bucket_antigo;
        registro_destino = entrada_ptr(he, destino, (size_t)destino->quantidade);
        memcpy(registro_destino, registro_origem, bytes_entrada(he));
        destino->quantidade++;
    }

    if (!registrar_expansao(he, offset_antigo, offset_novo, profundidade_local_antiga,
                            bucket_antigo->profundidade_local)) {
        free(temporario);
        free(bucket_antigo);
        free(bucket_novo);
        return false;
    }

    i = (size_t)(bucket_salvar(he, offset_antigo, bucket_antigo) &&
                 bucket_salvar(he, offset_novo, bucket_novo) &&
                 escrever_indice(he));
    free(temporario);
    free(bucket_antigo);
    free(bucket_novo);
    return i != 0u;
}

HashExtensivel he_criar(const char *caminho_hf, size_t capacidade_bucket,
                        size_t tamanho_registro) {
    HashExtensivelImpl *he;
    BucketDisco *bucket_inicial;
    int64_t offset_inicial;

    if (caminho_hf == NULL || !termina_com(caminho_hf, ".hf") || capacidade_bucket == 0 ||
        tamanho_registro == 0 || tamanho_registro < 2) {
        return NULL;
    }

    he = (HashExtensivelImpl *)calloc(1, sizeof(HashExtensivelImpl));
    if (he == NULL) {
        return NULL;
    }

    he->caminho_hf = duplicar_texto(caminho_hf);
    he->caminho_hfc = montar_caminho_auxiliar(caminho_hf, ".hfc");
    if (he->caminho_hf == NULL || he->caminho_hfc == NULL) {
        he_fechar(he);
        return NULL;
    }

    he->arquivo_dados = fopen(he->caminho_hf, "w+b");
    he->arquivo_indice = fopen(he->caminho_hfc, "w+b");
    if (he->arquivo_dados == NULL || he->arquivo_indice == NULL) {
        he_fechar(he);
        return NULL;
    }

    he->profundidade_global = 0;
    he->capacidade_bucket = capacidade_bucket;
    he->tamanho_registro = tamanho_registro;
    he->tamanho = 0;
    he->diretorio = (int64_t *)malloc(sizeof(int64_t));
    if (he->diretorio == NULL) {
        he_fechar(he);
        return NULL;
    }

    bucket_inicial = bucket_criar(he, 0);
    if (bucket_inicial == NULL) {
        he_fechar(he);
        return NULL;
    }

    offset_inicial = bucket_anexar(he, bucket_inicial);
    free(bucket_inicial);
    if (offset_inicial < 0) {
        he_fechar(he);
        return NULL;
    }

    he->diretorio[0] = offset_inicial;
    if (!escrever_indice(he)) {
        he_fechar(he);
        return NULL;
    }

    return he;
}

HashExtensivel he_abrir(const char *caminho_hf, size_t capacidade_bucket,
                        size_t tamanho_registro) {
    HashExtensivelImpl *he;

    if (caminho_hf == NULL || !termina_com(caminho_hf, ".hf") || capacidade_bucket == 0 ||
        tamanho_registro == 0 || tamanho_registro < 2) {
        return NULL;
    }

    he = (HashExtensivelImpl *)calloc(1, sizeof(HashExtensivelImpl));
    if (he == NULL) {
        return NULL;
    }

    he->caminho_hf = duplicar_texto(caminho_hf);
    he->caminho_hfc = montar_caminho_auxiliar(caminho_hf, ".hfc");
    if (he->caminho_hf == NULL || he->caminho_hfc == NULL) {
        he_fechar(he);
        return NULL;
    }

    he->arquivo_dados = fopen(he->caminho_hf, "r+b");
    he->arquivo_indice = fopen(he->caminho_hfc, "r+b");
    if (he->arquivo_dados == NULL || he->arquivo_indice == NULL ||
        !carregar_indice(he, capacidade_bucket, tamanho_registro)) {
        he_fechar(he);
        return NULL;
    }

    return he;
}

void he_fechar(HashExtensivel he_handle) {
    HashExtensivelImpl *he = he_impl(he_handle);

    if (he == NULL) {
        return;
    }

    if (he->diretorio != NULL && he->arquivo_indice != NULL) {
        escrever_indice(he);
        free(he->diretorio);
    }
    free(he->expansoes);
    if (he->arquivo_dados != NULL) {
        fclose(he->arquivo_dados);
    }
    if (he->arquivo_indice != NULL) {
        fclose(he->arquivo_indice);
    }
    free(he->caminho_hf);
    free(he->caminho_hfc);
    free(he);
}

bool he_buscar(HashExtensivel he_handle, const char *chave, void *registro_out) {
    HashExtensivelImpl *he = he_impl(he_handle);
    BucketDisco *bucket;
    int posicao;

    if (he == NULL || !chave_valida_texto(chave)) {
        return false;
    }

    bucket = bucket_carregar(he, he->diretorio[indice_diretorio(he, chave)]);
    if (bucket == NULL) {
        return false;
    }

    posicao = bucket_buscar_posicao(he, bucket, chave);
    if (posicao < 0) {
        free(bucket);
        return false;
    }

    if (registro_out != NULL) {
        memcpy(registro_out, entrada_ptr_const(he, bucket, (size_t)posicao),
               (size_t)he->tamanho_registro);
    }

    free(bucket);
    return true;
}

bool he_contem(HashExtensivel he, const char *chave) {
    return he_buscar(he, chave, NULL);
}

bool he_inserir(HashExtensivel he_handle, const void *registro) {
    HashExtensivelImpl *he = he_impl(he_handle);
    char chave[HE_TAMANHO_CHAVE_MAX + 1];

    if (he == NULL || registro == NULL || !extrair_chave_do_registro(he, registro, chave)) {
        return false;
    }

    while (1) {
        int64_t offset_bucket = he->diretorio[indice_diretorio(he, chave)];
        BucketDisco *bucket = bucket_carregar(he, offset_bucket);
        unsigned char *entrada;

        if (bucket == NULL) {
            return false;
        }

        if (bucket_buscar_posicao(he, bucket, chave) >= 0) {
            free(bucket);
            return false;
        }

        if ((size_t)bucket->quantidade < he->capacidade_bucket) {
            entrada = entrada_ptr(he, bucket, (size_t)bucket->quantidade);
            memcpy(entrada, registro, (size_t)he->tamanho_registro);
            bucket->quantidade++;
            he->tamanho++;

            if (!bucket_salvar(he, offset_bucket, bucket) || !escrever_indice(he)) {
                free(bucket);
                return false;
            }

            free(bucket);
            return true;
        }

        free(bucket);
        if (!dividir_bucket(he, chave)) {
            return false;
        }
    }
}

bool he_atualizar(HashExtensivel he_handle, const void *registro) {
    HashExtensivelImpl *he = he_impl(he_handle);
    char chave[HE_TAMANHO_CHAVE_MAX + 1];
    int64_t offset_bucket;
    BucketDisco *bucket;
    int posicao;
    unsigned char *entrada;

    if (he == NULL || registro == NULL || !extrair_chave_do_registro(he, registro, chave)) {
        return false;
    }

    offset_bucket = he->diretorio[indice_diretorio(he, chave)];
    bucket = bucket_carregar(he, offset_bucket);
    if (bucket == NULL) {
        return false;
    }

    posicao = bucket_buscar_posicao(he, bucket, chave);
    if (posicao < 0) {
        free(bucket);
        return false;
    }

    entrada = entrada_ptr(he, bucket, (size_t)posicao);
    memcpy(entrada, registro, (size_t)he->tamanho_registro);
    posicao = bucket_salvar(he, offset_bucket, bucket) ? 1 : 0;
    free(bucket);
    return posicao != 0;
}

bool he_remover(HashExtensivel he_handle, const char *chave) {
    HashExtensivelImpl *he = he_impl(he_handle);
    int64_t offset_bucket;
    BucketDisco *bucket;
    int posicao;

    if (he == NULL || !chave_valida_texto(chave)) {
        return false;
    }

    offset_bucket = he->diretorio[indice_diretorio(he, chave)];
    bucket = bucket_carregar(he, offset_bucket);
    if (bucket == NULL) {
        return false;
    }

    posicao = bucket_buscar_posicao(he, bucket, chave);
    if (posicao < 0) {
        free(bucket);
        return false;
    }

    bucket->quantidade--;
    if ((size_t)posicao != (size_t)bucket->quantidade) {
        memcpy(entrada_ptr(he, bucket, (size_t)posicao),
               entrada_ptr_const(he, bucket, (size_t)bucket->quantidade), bytes_entrada(he));
    }
    memset(entrada_ptr(he, bucket, (size_t)bucket->quantidade), 0, bytes_entrada(he));
    he->tamanho--;

    posicao = bucket_salvar(he, offset_bucket, bucket) && escrever_indice(he) ? 1 : 0;
    free(bucket);
    return posicao != 0;
}

size_t he_tamanho(HashExtensivel he_handle) {
    HashExtensivelImpl *he = he_impl(he_handle);
    return he == NULL ? 0u : (size_t)he->tamanho;
}

size_t he_profundidade_global(HashExtensivel he_handle) {
    HashExtensivelImpl *he = he_impl(he_handle);
    return he == NULL ? 0u : (size_t)he->profundidade_global;
}

size_t he_tamanho_diretorio(HashExtensivel he_handle) {
    HashExtensivelImpl *he = he_impl(he_handle);
    return he == NULL ? 0u : potencia2(he->profundidade_global);
}

size_t he_tamanho_registro(HashExtensivel he_handle) {
    HashExtensivelImpl *he = he_impl(he_handle);
    return he == NULL ? 0u : (size_t)he->tamanho_registro;
}

void he_dump(HashExtensivel he_handle, const char *caminho_dump) {
    HashExtensivelImpl *he = he_impl(he_handle);
    FILE *saida;
    size_t tamanho_diretorio;
    size_t i;

    if (he == NULL || he->diretorio == NULL || caminho_dump == NULL) {
        return;
    }

    saida = fopen(caminho_dump, "w");
    if (saida == NULL) {
        return;
    }

    tamanho_diretorio = potencia2(he->profundidade_global);
    fprintf(saida, "Hash extensivel\n");
    fprintf(saida, "Arquivo .hf: %s\n", he->caminho_hf != NULL ? he->caminho_hf : "(desconhecido)");
    fprintf(saida, "Arquivo .hfc: %s\n",
            he->caminho_hfc != NULL ? he->caminho_hfc : "(desconhecido)");
    fprintf(saida, "Profundidade global: %llu\n", (unsigned long long)he->profundidade_global);
    fprintf(saida, "Capacidade bucket: %llu\n", (unsigned long long)he->capacidade_bucket);
    fprintf(saida, "Tamanho registro: %llu\n", (unsigned long long)he->tamanho_registro);
    fprintf(saida, "Quantidade registros: %llu\n", (unsigned long long)he->tamanho);
    fprintf(saida, "Quantidade expansoes: %llu\n",
            (unsigned long long)he->quantidade_expansoes);
    fprintf(saida, "Diretorio:\n");

    for (i = 0; i < tamanho_diretorio; i++) {
        fprintf(saida, "  [%zu] -> %lld\n", i, (long long)he->diretorio[i]);
    }

    fprintf(saida, "Expansoes:\n");
    if (he->quantidade_expansoes == 0) {
        fprintf(saida, "  (nenhuma expansao registrada)\n");
    }
    for (i = 0; i < (size_t)he->quantidade_expansoes; i++) {
        const ExpansaoBucketDisco *expansao = &he->expansoes[i];

        fprintf(saida,
                "  [%zu] bucket %lld -> %lld | prof_local %llu -> %llu | prof_global %llu\n",
                i, (long long)expansao->offset_bucket_antigo,
                (long long)expansao->offset_bucket_novo,
                (unsigned long long)expansao->profundidade_local_antiga,
                (unsigned long long)expansao->profundidade_local_nova,
                (unsigned long long)expansao->profundidade_global_apos);
    }

    fprintf(saida, "Buckets:\n");
    for (i = 0; i < tamanho_diretorio; i++) {
        BucketDisco *bucket;
        size_t j;
        int repetido = 0;
        size_t k;
        char chave[HE_TAMANHO_CHAVE_MAX + 1];

        for (j = 0; j < i; j++) {
            if (he->diretorio[j] == he->diretorio[i]) {
                repetido = 1;
                break;
            }
        }
        if (repetido) {
            continue;
        }

        bucket = bucket_carregar(he, he->diretorio[i]);
        if (bucket == NULL) {
            continue;
        }

        fprintf(saida, "  Bucket @ %lld\n", (long long)he->diretorio[i]);
        fprintf(saida, "    Profundidade local: %llu\n",
                (unsigned long long)bucket->profundidade_local);
        fprintf(saida, "    Quantidade: %llu\n",
                (unsigned long long)bucket->quantidade);
        for (k = 0; k < bucket->quantidade; k++) {
            if (extrair_chave_do_registro(he, entrada_ptr_const(he, bucket, k), chave)) {
                fprintf(saida, "    - %s\n", chave);
            }
        }

        free(bucket);
    }

    fclose(saida);
}
