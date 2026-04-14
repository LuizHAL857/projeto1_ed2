#include "../include/hash_extensivel.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HE_MAGIC 0x48455835u

typedef struct {
    size_t profundidade_local;
    size_t quantidade;
    unsigned char dados[];
} BucketDisco;

typedef struct {
    uint32_t magic;
    size_t profundidade_global;
    size_t capacidade_bucket;
    size_t tamanho_registro;
    size_t tamanho;
    long offset_diretorio;
} CabecalhoDisco;

typedef struct {
    FILE *fp;
    size_t profundidade_global;
    size_t capacidade_bucket;
    size_t tamanho_registro;
    size_t tamanho;
    long offset_diretorio;
    size_t bytes_diretorio_gravado;
    long *diretorio;
    size_t chave_offset;
    size_t chave_tamanho;
    HESerializarRegistro serializar;
    HEDesserializarRegistro desserializar;
} HashExtensivelImpl;

bool he_serializar_bloco(const void *registro, void *buffer_out,
                         size_t tamanho_registro) {
    if (registro == NULL || buffer_out == NULL || tamanho_registro == 0) {
        return false;
    }

    memcpy(buffer_out, registro, tamanho_registro);
    return true;
}

bool he_desserializar_bloco(const void *buffer, size_t tamanho_registro,
                            void *registro_out) {
    if (buffer == NULL || registro_out == NULL || tamanho_registro == 0) {
        return false;
    }

    memcpy(registro_out, buffer, tamanho_registro);
    return true;
}

static size_t pow2(size_t e) { return ((size_t)1) << e; }
static HashExtensivelImpl *he_impl(HashExtensivel he) { return (HashExtensivelImpl *)he; }
static const HashExtensivelImpl *he_impl_const(HashExtensivel he) {
    return (const HashExtensivelImpl *)he;
}

static bool ir_para(FILE *fp, long offset) { return fseek(fp, offset, SEEK_SET) == 0; }

static size_t bucket_bytes(const HashExtensivelImpl *he) {
    return sizeof(BucketDisco) + he->capacidade_bucket * he->tamanho_registro;
}

static size_t diretorio_bytes(const HashExtensivelImpl *he) {
    return pow2(he->profundidade_global) * sizeof(long);
}

static unsigned char *registro_ptr(const HashExtensivelImpl *he, BucketDisco *bucket,
                                   size_t indice) {
    return bucket->dados + indice * he->tamanho_registro;
}

static const unsigned char *registro_ptr_const(const HashExtensivelImpl *he,
                                               const BucketDisco *bucket,
                                               size_t indice) {
    return bucket->dados + indice * he->tamanho_registro;
}

static size_t bounded_strlen_local(const char *texto, size_t limite) {
    size_t i;

    for (i = 0; i < limite; i++) {
        if (texto[i] == '\0') {
            return i;
        }
    }

    return limite;
}

static bool chave_valida(const char *chave) {
    size_t i;
    size_t tamanho;

    if (chave == NULL) {
        return false;
    }

    tamanho = strlen(chave);
    if (tamanho == 0 || tamanho > HE_TAMANHO_CHAVE_MAX) {
        return false;
    }

    for (i = 0; i < tamanho; i++) {
        unsigned char c = (unsigned char)chave[i];
        if (!isalnum(c) && c != '-') {
            return false;
        }
    }

    return true;
}

static bool extrair_chave_do_buffer(const HashExtensivelImpl *he,
                                    const void *registro_buffer,
                                    char chave_out[HE_TAMANHO_CHAVE_MAX + 1]) {
    const char *inicio;
    size_t tamanho;

    if (he == NULL || registro_buffer == NULL || chave_out == NULL) {
        return false;
    }

    inicio = (const char *)registro_buffer + he->chave_offset;
    tamanho = bounded_strlen_local(inicio, he->chave_tamanho);

    if (tamanho == 0 || tamanho >= he->chave_tamanho || tamanho > HE_TAMANHO_CHAVE_MAX) {
        return false;
    }

    memcpy(chave_out, inicio, tamanho);
    chave_out[tamanho] = '\0';
    return chave_valida(chave_out);
}

static size_t hash_chave(const char *chave) {
    uint64_t hash = 1469598103934665603ULL;

    while (*chave != '\0') {
        hash ^= (unsigned char)(*chave);
        hash *= 1099511628211ULL;
        chave++;
    }

    return (size_t)hash;
}

static size_t indice_diretorio(const HashExtensivelImpl *he, const char *chave) {
    return hash_chave(chave) & (pow2(he->profundidade_global) - 1u);
}

static bool info_valida(const HEInfoRegistro *info) {
    if (info == NULL || info->tamanho_registro == 0 || info->chave_tamanho == 0 ||
        info->serializar == NULL || info->desserializar == NULL) {
        return false;
    }

    if (info->chave_tamanho > HE_TAMANHO_CHAVE_MAX + 1u) {
        return false;
    }

    return info->chave_offset < info->tamanho_registro &&
           info->chave_offset + info->chave_tamanho <= info->tamanho_registro;
}

static void copiar_info(HashExtensivelImpl *he, const HEInfoRegistro *info) {
    he->tamanho_registro = info->tamanho_registro;
    he->chave_offset = info->chave_offset;
    he->chave_tamanho = info->chave_tamanho;
    he->serializar = info->serializar;
    he->desserializar = info->desserializar;
}

static bool escrever_cabecalho(HashExtensivelImpl *he) {
    CabecalhoDisco cab;

    if (he == NULL || he->fp == NULL) {
        return false;
    }

    cab.magic = HE_MAGIC;
    cab.profundidade_global = he->profundidade_global;
    cab.capacidade_bucket = he->capacidade_bucket;
    cab.tamanho_registro = he->tamanho_registro;
    cab.tamanho = he->tamanho;
    cab.offset_diretorio = he->offset_diretorio;

    return ir_para(he->fp, 0L) &&
           fwrite(&cab, sizeof(CabecalhoDisco), 1, he->fp) == 1;
}

static bool salvar_metadados(HashExtensivelImpl *he) {
    size_t bytes_atual;
    long novo_offset;
    size_t dir_tam;

    if (he == NULL || he->fp == NULL || he->diretorio == NULL) {
        return false;
    }

    bytes_atual = diretorio_bytes(he);
    novo_offset = he->offset_diretorio;
    dir_tam = pow2(he->profundidade_global);

    if (novo_offset < 0 || he->bytes_diretorio_gravado < bytes_atual) {
        if (fseek(he->fp, 0L, SEEK_END) != 0) {
            return false;
        }

        novo_offset = ftell(he->fp);
        if (novo_offset < 0) {
            return false;
        }
    }

    he->offset_diretorio = novo_offset;
    he->bytes_diretorio_gravado = bytes_atual;

    return ir_para(he->fp, he->offset_diretorio) &&
           fwrite(he->diretorio, sizeof(long), dir_tam, he->fp) == dir_tam &&
           escrever_cabecalho(he) &&
           fflush(he->fp) == 0;
}

static bool carregar_metadados(HashExtensivelImpl *he, const HEInfoRegistro *info) {
    CabecalhoDisco cab;
    size_t dir_tam;

    if (he == NULL || he->fp == NULL || !info_valida(info) || !ir_para(he->fp, 0L)) {
        return false;
    }

    if (fread(&cab, sizeof(CabecalhoDisco), 1, he->fp) != 1 ||
        cab.magic != HE_MAGIC || cab.capacidade_bucket == 0 || cab.tamanho_registro == 0) {
        return false;
    }

    if (cab.tamanho_registro != info->tamanho_registro) {
        return false;
    }

    copiar_info(he, info);
    he->profundidade_global = cab.profundidade_global;
    he->capacidade_bucket = cab.capacidade_bucket;
    he->tamanho = cab.tamanho;
    he->offset_diretorio = cab.offset_diretorio;
    he->bytes_diretorio_gravado = pow2(cab.profundidade_global) * sizeof(long);

    dir_tam = pow2(he->profundidade_global);
    he->diretorio = (long *)malloc(dir_tam * sizeof(long));
    if (he->diretorio == NULL) {
        return false;
    }

    if (!ir_para(he->fp, he->offset_diretorio) ||
        fread(he->diretorio, sizeof(long), dir_tam, he->fp) != dir_tam) {
        free(he->diretorio);
        he->diretorio = NULL;
        return false;
    }

    return true;
}

static BucketDisco *bucket_criar(const HashExtensivelImpl *he, size_t profundidade_local) {
    BucketDisco *bucket = (BucketDisco *)calloc(1, bucket_bytes(he));

    if (bucket != NULL) {
        bucket->profundidade_local = profundidade_local;
    }

    return bucket;
}

static BucketDisco *bucket_carregar(HashExtensivelImpl *he, long offset) {
    BucketDisco *bucket;

    if (he == NULL || he->fp == NULL) {
        return NULL;
    }

    bucket = (BucketDisco *)malloc(bucket_bytes(he));
    if (bucket == NULL) {
        return NULL;
    }

    if (!ir_para(he->fp, offset) || fread(bucket, bucket_bytes(he), 1, he->fp) != 1) {
        free(bucket);
        return NULL;
    }

    return bucket;
}

static bool bucket_salvar(HashExtensivelImpl *he, long offset, const BucketDisco *bucket) {
    return he != NULL && he->fp != NULL && bucket != NULL &&
           ir_para(he->fp, offset) &&
           fwrite(bucket, bucket_bytes(he), 1, he->fp) == 1 &&
           fflush(he->fp) == 0;
}

static long bucket_append(HashExtensivelImpl *he, const BucketDisco *bucket) {
    long offset;

    if (he == NULL || he->fp == NULL || bucket == NULL || fseek(he->fp, 0L, SEEK_END) != 0) {
        return -1L;
    }

    offset = ftell(he->fp);
    if (offset < 0 || fwrite(bucket, bucket_bytes(he), 1, he->fp) != 1 ||
        fflush(he->fp) != 0) {
        return -1L;
    }

    return offset;
}

static int bucket_buscar_posicao(const HashExtensivelImpl *he,
                                 const BucketDisco *bucket, const char *chave) {
    size_t i;

    for (i = 0; i < bucket->quantidade; i++) {
        char chave_registro[HE_TAMANHO_CHAVE_MAX + 1];

        if (extrair_chave_do_buffer(he, registro_ptr_const(he, bucket, i), chave_registro) &&
            strcmp(chave_registro, chave) == 0) {
            return (int)i;
        }
    }

    return -1;
}

static bool carregar_bucket_da_chave(HashExtensivelImpl *he, const char *chave,
                                     size_t *indice_out, long *offset_out,
                                     BucketDisco **bucket_out) {
    size_t indice = indice_diretorio(he, chave);
    long offset = he->diretorio[indice];
    BucketDisco *bucket = bucket_carregar(he, offset);

    if (bucket == NULL) {
        return false;
    }

    if (indice_out != NULL) {
        *indice_out = indice;
    }
    if (offset_out != NULL) {
        *offset_out = offset;
    }
    *bucket_out = bucket;
    return true;
}

static bool persistir_bucket(HashExtensivelImpl *he, long offset, const BucketDisco *bucket,
                             bool salvar_meta) {
    return bucket_salvar(he, offset, bucket) && (!salvar_meta || salvar_metadados(he));
}

static bool serializar_para_buffer(const HashExtensivelImpl *he, const void *registro,
                                   void *buffer_out) {
    return he != NULL && he->serializar != NULL &&
           he->serializar(registro, buffer_out, he->tamanho_registro);
}

static bool desserializar_do_buffer(const HashExtensivelImpl *he, const void *buffer,
                                    void *registro_out) {
    return he != NULL && he->desserializar != NULL &&
           he->desserializar(buffer, he->tamanho_registro, registro_out);
}

static bool duplicar_diretorio(HashExtensivelImpl *he) {
    size_t antigo_tam;
    size_t i;
    long *novo_dir;

    if (he == NULL || he->diretorio == NULL) {
        return false;
    }

    antigo_tam = pow2(he->profundidade_global);
    novo_dir = (long *)malloc(2 * antigo_tam * sizeof(long));
    if (novo_dir == NULL) {
        return false;
    }

    for (i = 0; i < antigo_tam; i++) {
        novo_dir[i] = he->diretorio[i];
        novo_dir[i + antigo_tam] = he->diretorio[i];
    }

    free(he->diretorio);
    he->diretorio = novo_dir;
    he->profundidade_global++;
    return salvar_metadados(he);
}

static bool split_bucket(HashExtensivelImpl *he, size_t indice) {
    long offset_antigo;
    long offset_novo;
    BucketDisco *bucket_antigo;
    BucketDisco *bucket_novo;
    unsigned char *temporarios;
    size_t qtd_antiga;
    size_t bit_divisao;
    size_t dir_tam;
    size_t i;

    if (he == NULL || he->diretorio == NULL) {
        return false;
    }

    offset_antigo = he->diretorio[indice];
    bucket_antigo = bucket_carregar(he, offset_antigo);
    bucket_novo = bucket_criar(he, 0);
    if (bucket_antigo == NULL || bucket_novo == NULL) {
        free(bucket_antigo);
        free(bucket_novo);
        return false;
    }

    qtd_antiga = bucket_antigo->quantidade;
    temporarios = (unsigned char *)malloc(qtd_antiga * he->tamanho_registro);
    if (temporarios == NULL) {
        free(bucket_antigo);
        free(bucket_novo);
        return false;
    }

    memcpy(temporarios, bucket_antigo->dados, qtd_antiga * he->tamanho_registro);
    bucket_novo->profundidade_local = ++bucket_antigo->profundidade_local;
    memset(bucket_antigo->dados, 0, he->capacidade_bucket * he->tamanho_registro);
    bucket_antigo->quantidade = 0;
    bucket_novo->quantidade = 0;

    offset_novo = bucket_append(he, bucket_novo);
    if (offset_novo < 0) {
        free(temporarios);
        free(bucket_antigo);
        free(bucket_novo);
        return false;
    }

    bit_divisao = ((size_t)1) << (bucket_antigo->profundidade_local - 1u);
    dir_tam = pow2(he->profundidade_global);
    for (i = 0; i < dir_tam; i++) {
        if (he->diretorio[i] == offset_antigo && (i & bit_divisao)) {
            he->diretorio[i] = offset_novo;
        }
    }

    for (i = 0; i < qtd_antiga; i++) {
        unsigned char *origem = temporarios + i * he->tamanho_registro;
        char chave_registro[HE_TAMANHO_CHAVE_MAX + 1];
        BucketDisco *destino;
        unsigned char *destino_registro;

        if (!extrair_chave_do_buffer(he, origem, chave_registro)) {
            free(temporarios);
            free(bucket_antigo);
            free(bucket_novo);
            return false;
        }

        destino = he->diretorio[indice_diretorio(he, chave_registro)] == offset_novo ?
                      bucket_novo : bucket_antigo;
        destino_registro = registro_ptr(he, destino, destino->quantidade++);
        assert(destino->quantidade <= he->capacidade_bucket);
        memcpy(destino_registro, origem, he->tamanho_registro);
    }

    i = persistir_bucket(he, offset_antigo, bucket_antigo, false) &&
        persistir_bucket(he, offset_novo, bucket_novo, true);
    free(temporarios);
    free(bucket_antigo);
    free(bucket_novo);
    return i != 0;
}

HashExtensivel he_criar(const char *caminho_arquivo, size_t capacidade_bucket,
                        const HEInfoRegistro *info) {
    HashExtensivelImpl *he;
    BucketDisco *bucket_inicial;
    long offset_bucket;

    if (caminho_arquivo == NULL || capacidade_bucket == 0 || !info_valida(info)) {
        return NULL;
    }

    he = (HashExtensivelImpl *)malloc(sizeof(HashExtensivelImpl));
    if (he == NULL) {
        return NULL;
    }

    he->fp = fopen(caminho_arquivo, "w+b");
    if (he->fp == NULL) {
        free(he);
        return NULL;
    }

    he->profundidade_global = 0;
    he->capacidade_bucket = capacidade_bucket;
    copiar_info(he, info);
    he->tamanho = 0;
    he->offset_diretorio = -1L;
    he->bytes_diretorio_gravado = 0;
    he->diretorio = (long *)malloc(sizeof(long));

    if (he->diretorio == NULL) {
        fclose(he->fp);
        free(he);
        return NULL;
    }

    he->diretorio[0] = 0L;
    if (fwrite(&(CabecalhoDisco){0}, sizeof(CabecalhoDisco), 1, he->fp) != 1) {
        free(he->diretorio);
        fclose(he->fp);
        free(he);
        return NULL;
    }

    bucket_inicial = bucket_criar(he, 0);
    offset_bucket = bucket_inicial ? bucket_append(he, bucket_inicial) : -1L;
    free(bucket_inicial);

    if (offset_bucket < 0) {
        free(he->diretorio);
        fclose(he->fp);
        free(he);
        return NULL;
    }

    he->diretorio[0] = offset_bucket;
    if (!salvar_metadados(he)) {
        free(he->diretorio);
        fclose(he->fp);
        free(he);
        return NULL;
    }

    return (HashExtensivel)he;
}

HashExtensivel he_abrir(const char *caminho_arquivo, const HEInfoRegistro *info) {
    HashExtensivelImpl *he;

    if (caminho_arquivo == NULL || !info_valida(info)) {
        return NULL;
    }

    he = (HashExtensivelImpl *)malloc(sizeof(HashExtensivelImpl));
    if (he == NULL) {
        return NULL;
    }

    he->fp = fopen(caminho_arquivo, "r+b");
    if (he->fp == NULL) {
        free(he);
        return NULL;
    }

    he->diretorio = NULL;
    he->offset_diretorio = -1L;
    he->bytes_diretorio_gravado = 0;

    if (!carregar_metadados(he, info)) {
        fclose(he->fp);
        free(he);
        return NULL;
    }

    return (HashExtensivel)he;
}

void he_fechar(HashExtensivel he_handle) {
    HashExtensivelImpl *he = he_impl(he_handle);

    if (he == NULL) {
        return;
    }
    if (he->diretorio != NULL) {
        salvar_metadados(he);
        free(he->diretorio);
    }
    if (he->fp != NULL) {
        fclose(he->fp);
    }
    free(he);
}

bool he_inserir(HashExtensivel he_handle, const void *registro) {
    HashExtensivelImpl *he = he_impl(he_handle);
    unsigned char *serializado;
    char chave[HE_TAMANHO_CHAVE_MAX + 1];

    if (he == NULL || registro == NULL) {
        return false;
    }

    serializado = (unsigned char *)malloc(he->tamanho_registro);
    if (serializado == NULL || !serializar_para_buffer(he, registro, serializado) ||
        !extrair_chave_do_buffer(he, serializado, chave)) {
        free(serializado);
        return false;
    }

    while (1) {
        size_t indice;
        long offset;
        BucketDisco *bucket;
        int pos;

        if (!carregar_bucket_da_chave(he, chave, &indice, &offset, &bucket)) {
            free(serializado);
            return false;
        }

        pos = bucket_buscar_posicao(he, bucket, chave);
        if (pos != -1) {
            memcpy(registro_ptr(he, bucket, (size_t)pos), serializado, he->tamanho_registro);
            pos = persistir_bucket(he, offset, bucket, false);
            free(bucket);
            free(serializado);
            return pos != 0;
        }

        if (bucket->quantidade < he->capacidade_bucket) {
            memcpy(registro_ptr(he, bucket, bucket->quantidade++),
                   serializado, he->tamanho_registro);
            he->tamanho++;
            pos = persistir_bucket(he, offset, bucket, true);
            free(bucket);
            free(serializado);
            return pos != 0;
        }

        pos = bucket->profundidade_local == he->profundidade_global ?
                  duplicar_diretorio(he) : 1;
        free(bucket);
        if (!pos || !split_bucket(he, indice)) {
            free(serializado);
            return false;
        }
    }
}

bool he_buscar(HashExtensivel he_handle, const char *chave, void *registro_out) {
    HashExtensivelImpl *he = he_impl(he_handle);
    BucketDisco *bucket;
    int pos;
    bool sucesso = false;

    if (he == NULL || registro_out == NULL || !chave_valida(chave) ||
        !carregar_bucket_da_chave(he, chave, NULL, NULL, &bucket)) {
        return false;
    }

    pos = bucket_buscar_posicao(he, bucket, chave);
    if (pos != -1) {
        sucesso = desserializar_do_buffer(he,
                                          registro_ptr_const(he, bucket, (size_t)pos),
                                          registro_out);
    }

    free(bucket);
    return sucesso;
}

bool he_contem(HashExtensivel he_handle, const char *chave) {
    HashExtensivelImpl *he = he_impl(he_handle);
    BucketDisco *bucket;
    bool contem;

    if (he == NULL || !chave_valida(chave) ||
        !carregar_bucket_da_chave(he, chave, NULL, NULL, &bucket)) {
        return false;
    }

    contem = bucket_buscar_posicao(he, bucket, chave) != -1;
    free(bucket);
    return contem;
}

bool he_remover(HashExtensivel he_handle, const char *chave) {
    HashExtensivelImpl *he = he_impl(he_handle);
    long offset;
    BucketDisco *bucket;
    int pos;

    if (he == NULL || !chave_valida(chave) ||
        !carregar_bucket_da_chave(he, chave, NULL, &offset, &bucket)) {
        return false;
    }

    pos = bucket_buscar_posicao(he, bucket, chave);
    if (pos == -1) {
        free(bucket);
        return false;
    }

    if ((size_t)pos != bucket->quantidade - 1u) {
        memcpy(registro_ptr(he, bucket, (size_t)pos),
               registro_ptr(he, bucket, bucket->quantidade - 1u),
               he->tamanho_registro);
    }

    memset(registro_ptr(he, bucket, bucket->quantidade - 1u), 0, he->tamanho_registro);
    bucket->quantidade--;
    he->tamanho--;
    pos = persistir_bucket(he, offset, bucket, true);
    free(bucket);
    return pos != 0;
}

size_t he_tamanho(HashExtensivel he_handle) {
    const HashExtensivelImpl *he = he_impl_const(he_handle);
    return he ? he->tamanho : 0;
}

size_t he_profundidade_global(HashExtensivel he_handle) {
    const HashExtensivelImpl *he = he_impl_const(he_handle);
    return he ? he->profundidade_global : 0;
}

size_t he_tamanho_diretorio(HashExtensivel he_handle) {
    const HashExtensivelImpl *he = he_impl_const(he_handle);
    return he ? pow2(he->profundidade_global) : 0;
}

size_t he_tamanho_registro(HashExtensivel he_handle) {
    const HashExtensivelImpl *he = he_impl_const(he_handle);
    return he ? he->tamanho_registro : 0;
}
