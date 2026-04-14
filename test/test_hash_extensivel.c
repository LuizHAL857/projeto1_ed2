#include "../unity/unity.h"
#include "../include/hash_extensivel.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char cpf[20];
    char nome[40];
    char sobrenome[40];
    char sexo;
    char nasc[11];
    char cep[20];
    char face;
    int num;
    char compl[32];
} RegistroMoradorTeste;

typedef struct {
    char cpf[20];
    char nome[40];
    char cor_borda[32];
    int numero;
} RegistroOpacoDiscoTeste;

typedef struct {
    char *cpf;
    char *nome;
    char *cor_borda;
    int numero;
} RegistroOpacoTeste;

static const HEInfoRegistro INFO_MORADOR = {
    sizeof(RegistroMoradorTeste),
    offsetof(RegistroMoradorTeste, cpf),
    sizeof(((RegistroMoradorTeste *)0)->cpf),
    he_serializar_bloco,
    he_desserializar_bloco
};

static const HEInfoRegistro INFO_OPACO = {
    sizeof(RegistroOpacoDiscoTeste),
    offsetof(RegistroOpacoDiscoTeste, cpf),
    sizeof(((RegistroOpacoDiscoTeste *)0)->cpf),
    NULL,
    NULL
};

static HashExtensivel he;
static const char *ARQ_TESTE = "hash_extensivel_teste.dat";
static const char *ARQ_TESTE_CAP1 = "hash_extensivel_cap1_teste.dat";
static const char *ARQ_TESTE_CAP1_PERSIST = "hash_extensivel_cap1_persist_teste.dat";

static RegistroMoradorTeste criar_registro(const char *cpf, const char *nome,
                                           const char *sobrenome, char sexo,
                                           const char *nasc, const char *cep,
                                           char face, int num,
                                           const char *compl) {
    RegistroMoradorTeste registro;

    memset(&registro, 0, sizeof(registro));
    strncpy(registro.cpf, cpf, sizeof(registro.cpf) - 1);
    strncpy(registro.nome, nome, sizeof(registro.nome) - 1);
    strncpy(registro.sobrenome, sobrenome, sizeof(registro.sobrenome) - 1);
    registro.sexo = sexo;
    strncpy(registro.nasc, nasc, sizeof(registro.nasc) - 1);
    strncpy(registro.cep, cep, sizeof(registro.cep) - 1);
    registro.face = face;
    registro.num = num;
    strncpy(registro.compl, compl, sizeof(registro.compl) - 1);

    return registro;
}

static void assert_registros_iguais(const RegistroMoradorTeste *esperado,
                                    const RegistroMoradorTeste *obtido) {
    TEST_ASSERT_EQUAL_STRING(esperado->cpf, obtido->cpf);
    TEST_ASSERT_EQUAL_STRING(esperado->nome, obtido->nome);
    TEST_ASSERT_EQUAL_STRING(esperado->sobrenome, obtido->sobrenome);
    TEST_ASSERT_EQUAL_CHAR(esperado->sexo, obtido->sexo);
    TEST_ASSERT_EQUAL_STRING(esperado->nasc, obtido->nasc);
    TEST_ASSERT_EQUAL_STRING(esperado->cep, obtido->cep);
    TEST_ASSERT_EQUAL_CHAR(esperado->face, obtido->face);
    TEST_ASSERT_EQUAL_INT(esperado->num, obtido->num);
    TEST_ASSERT_EQUAL_STRING(esperado->compl, obtido->compl);
}

static char *duplica_string_teste(const char *texto) {
    size_t tamanho;
    char *resultado;

    if (texto == NULL) {
        return NULL;
    }

    tamanho = strlen(texto) + 1;
    resultado = (char *)malloc(tamanho);
    if (resultado != NULL) {
        memcpy(resultado, texto, tamanho);
    }

    return resultado;
}

static RegistroOpacoTeste criar_registro_opaco(const char *cpf, const char *nome,
                                               const char *cor_borda, int numero) {
    RegistroOpacoTeste registro;

    registro.cpf = duplica_string_teste(cpf);
    registro.nome = duplica_string_teste(nome);
    registro.cor_borda = duplica_string_teste(cor_borda);
    registro.numero = numero;
    return registro;
}

static void destruir_registro_opaco(RegistroOpacoTeste *registro) {
    if (registro == NULL) {
        return;
    }

    free(registro->cpf);
    free(registro->nome);
    free(registro->cor_borda);
    registro->cpf = NULL;
    registro->nome = NULL;
    registro->cor_borda = NULL;
}

static bool serializar_registro_opaco(const void *registro, void *buffer_out,
                                      size_t tamanho_registro) {
    const RegistroOpacoTeste *origem = (const RegistroOpacoTeste *)registro;
    RegistroOpacoDiscoTeste disco;

    if (origem == NULL || buffer_out == NULL ||
        tamanho_registro != sizeof(RegistroOpacoDiscoTeste) ||
        origem->cpf == NULL || origem->nome == NULL || origem->cor_borda == NULL) {
        return false;
    }

    memset(&disco, 0, sizeof(disco));
    strncpy(disco.cpf, origem->cpf, sizeof(disco.cpf) - 1);
    strncpy(disco.nome, origem->nome, sizeof(disco.nome) - 1);
    strncpy(disco.cor_borda, origem->cor_borda, sizeof(disco.cor_borda) - 1);
    disco.numero = origem->numero;

    memcpy(buffer_out, &disco, sizeof(disco));
    return true;
}

static bool desserializar_registro_opaco(const void *buffer, size_t tamanho_registro,
                                         void *registro_out) {
    const RegistroOpacoDiscoTeste *disco = (const RegistroOpacoDiscoTeste *)buffer;
    RegistroOpacoTeste *registro = (RegistroOpacoTeste *)registro_out;

    if (buffer == NULL || registro_out == NULL ||
        tamanho_registro != sizeof(RegistroOpacoDiscoTeste)) {
        return false;
    }

    registro->cpf = duplica_string_teste(disco->cpf);
    registro->nome = duplica_string_teste(disco->nome);
    registro->cor_borda = duplica_string_teste(disco->cor_borda);
    registro->numero = disco->numero;

    if (registro->cpf == NULL || registro->nome == NULL || registro->cor_borda == NULL) {
        destruir_registro_opaco(registro);
        return false;
    }

    return true;
}

static size_t hash_chave_teste(const char *chave) {
    uint64_t hash = 1469598103934665603ULL;

    while (*chave != '\0') {
        hash ^= (unsigned char)(*chave);
        hash *= 1099511628211ULL;
        chave++;
    }

    return (size_t)hash;
}

static bool gerar_chave_com_mascara(const char *prefixo, size_t profundidade,
                                    size_t mascara_alvo, int *contador_io,
                                    char *saida, size_t saida_tam) {
    size_t mascara = profundidade == 0 ? 0u : (((size_t)1) << profundidade) - 1u;
    int contador;
    int tentativas;

    if (prefixo == NULL || contador_io == NULL || saida == NULL || saida_tam == 0) {
        return false;
    }

    contador = *contador_io;
    for (tentativas = 0; tentativas < 200000; tentativas++, contador++) {
        snprintf(saida, saida_tam, "%s%d", prefixo, contador);
        if ((hash_chave_teste(saida) & mascara) == mascara_alvo) {
            *contador_io = contador + 1;
            return true;
        }
    }

    return false;
}

static long tamanho_arquivo(const char *caminho) {
    FILE *arquivo = fopen(caminho, "rb");
    long tamanho;

    if (arquivo == NULL) {
        return -1L;
    }

    if (fseek(arquivo, 0L, SEEK_END) != 0) {
        fclose(arquivo);
        return -1L;
    }

    tamanho = ftell(arquivo);
    fclose(arquivo);
    return tamanho;
}

void setUp(void) {
    remove(ARQ_TESTE);
    he = he_criar(ARQ_TESTE, 2, &INFO_MORADOR);
}

void tearDown(void) {
    he_fechar(he);
    he = NULL;
    remove(ARQ_TESTE);
    remove(ARQ_TESTE_CAP1);
    remove(ARQ_TESTE_CAP1_PERSIST);
    remove("hash_extensivel_opaco_teste.dat");
}

void test_criacao_deve_funcionar(void) {
    TEST_ASSERT_NOT_NULL(he);
    TEST_ASSERT_EQUAL_UINT(0, he_tamanho(he));
    TEST_ASSERT_EQUAL_UINT(0, he_profundidade_global(he));
    TEST_ASSERT_EQUAL_UINT(1, he_tamanho_diretorio(he));
    TEST_ASSERT_EQUAL_UINT(sizeof(RegistroMoradorTeste), he_tamanho_registro(he));
}

void test_abrir_arquivo_inexistente_deve_retornar_null(void) {
    HashExtensivel outro = he_abrir("arquivo_que_nao_existe_123.dat", &INFO_MORADOR);
    TEST_ASSERT_NULL(outro);
}

void test_inserir_e_buscar_registro_completo_deve_funcionar(void) {
    RegistroMoradorTeste esperado =
        criar_registro("12345678901", "Ana", "Silva", 'F', "10/01/1999",
                       "cep-15", 'N', 100, "apto101");
    RegistroMoradorTeste obtido;

    memset(&obtido, 0, sizeof(obtido));

    TEST_ASSERT_TRUE(he_inserir(he, &esperado));
    TEST_ASSERT_TRUE(he_buscar(he, "12345678901", &obtido));
    assert_registros_iguais(&esperado, &obtido);
}

void test_atualizacao_deve_substituir_registro_completo(void) {
    RegistroMoradorTeste antigo =
        criar_registro("11122233344", "Bia", "Souza", 'F', "01/02/2000",
                       "cep-20", 'S', 20, "fundos");
    RegistroMoradorTeste novo =
        criar_registro("11122233344", "Beatriz", "Souza", 'F', "01/02/2000",
                       "cep-77", 'L', 77, "casa");
    RegistroMoradorTeste obtido;

    memset(&obtido, 0, sizeof(obtido));

    TEST_ASSERT_TRUE(he_inserir(he, &antigo));
    TEST_ASSERT_TRUE(he_inserir(he, &novo));
    TEST_ASSERT_EQUAL_UINT(1, he_tamanho(he));
    TEST_ASSERT_TRUE(he_buscar(he, "11122233344", &obtido));
    assert_registros_iguais(&novo, &obtido);
}

void test_persistencia_controlada_do_diretorio_nao_deve_crescer_arquivo_sem_split(void) {
    RegistroMoradorTeste primeiro =
        criar_registro("10020030040", "Eva", "Melo", 'F', "01/01/2001",
                       "cep-10", 'N', 10, "casa");
    RegistroMoradorTeste segundo =
        criar_registro("10020030041", "Ivo", "Melo", 'M', "02/02/2002",
                       "cep-11", 'S', 20, "apto");
    long tamanho_inicial = tamanho_arquivo(ARQ_TESTE);
    long tamanho_apos_primeira;
    long tamanho_apos_segunda;

    TEST_ASSERT_GREATER_OR_EQUAL(0, tamanho_inicial);
    TEST_ASSERT_TRUE(he_inserir(he, &primeiro));
    tamanho_apos_primeira = tamanho_arquivo(ARQ_TESTE);
    TEST_ASSERT_EQUAL_INT64(tamanho_inicial, tamanho_apos_primeira);

    TEST_ASSERT_TRUE(he_inserir(he, &segundo));
    tamanho_apos_segunda = tamanho_arquivo(ARQ_TESTE);
    TEST_ASSERT_EQUAL_INT64(tamanho_inicial, tamanho_apos_segunda);
}

void test_remocao_deve_funcionar(void) {
    RegistroMoradorTeste registro =
        criar_registro("22233344455", "Caio", "Lima", 'M', "03/03/1998",
                       "cep-01", 'O', 10, "sobrado");

    TEST_ASSERT_TRUE(he_inserir(he, &registro));
    TEST_ASSERT_TRUE(he_remover(he, "22233344455"));
    TEST_ASSERT_FALSE(he_contem(he, "22233344455"));
    TEST_ASSERT_EQUAL_UINT(0, he_tamanho(he));
}

void test_split_e_duplicacao_de_diretorio_devem_funcionar(void) {
    RegistroMoradorTeste esperado;
    RegistroMoradorTeste obtido;
    char chave[32];
    char cep[32];
    char compl[32];

    for (int i = 0; i < 10; i++) {
        snprintf(chave, sizeof(chave), "cpf-%02d", i);
        snprintf(cep, sizeof(cep), "cep-%02d", i);
        snprintf(compl, sizeof(compl), "apto-%02d", i);

        esperado = criar_registro(chave, "Nome", "Teste", 'M',
                                  "04/04/1997", cep, 'N', i * 10, compl);
        TEST_ASSERT_TRUE(he_inserir(he, &esperado));
    }

    TEST_ASSERT_GREATER_THAN(0, he_profundidade_global(he));
    TEST_ASSERT_EQUAL_UINT(10, he_tamanho(he));

    for (int i = 0; i < 10; i++) {
        snprintf(chave, sizeof(chave), "cpf-%02d", i);
        snprintf(cep, sizeof(cep), "cep-%02d", i);
        snprintf(compl, sizeof(compl), "apto-%02d", i);

        esperado = criar_registro(chave, "Nome", "Teste", 'M',
                                  "04/04/1997", cep, 'N', i * 10, compl);
        memset(&obtido, 0, sizeof(obtido));

        TEST_ASSERT_TRUE(he_buscar(he, chave, &obtido));
        assert_registros_iguais(&esperado, &obtido);
    }
}

void test_bucket_com_capacidade_1_deve_suportar_colisoes_e_split_sucessivo(void) {
    HashExtensivel he_local;
    RegistroMoradorTeste esperado[4];
    RegistroMoradorTeste obtido;
    char chaves[4][32];
    size_t mascaras[4] = {0u, 4u, 8u, 12u};
    int contador = 0;

    remove(ARQ_TESTE_CAP1);
    he_local = he_criar(ARQ_TESTE_CAP1, 1, &INFO_MORADOR);
    TEST_ASSERT_NOT_NULL(he_local);

    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_TRUE(gerar_chave_com_mascara("col-", 4, mascaras[i],
                                                 &contador, chaves[i], sizeof(chaves[i])));
        esperado[i] = criar_registro(chaves[i], "Nome", "Cap1", 'M',
                                     "08/08/1998", "cep-01", 'N', i + 1, "colisao");
        TEST_ASSERT_TRUE(he_inserir(he_local, &esperado[i]));
    }

    TEST_ASSERT_EQUAL_UINT(4, he_tamanho(he_local));
    TEST_ASSERT_GREATER_OR_EQUAL_UINT(4, he_profundidade_global(he_local));
    TEST_ASSERT_GREATER_OR_EQUAL_UINT(16, he_tamanho_diretorio(he_local));

    for (int i = 0; i < 4; i++) {
        memset(&obtido, 0, sizeof(obtido));
        TEST_ASSERT_TRUE(he_buscar(he_local, chaves[i], &obtido));
        assert_registros_iguais(&esperado[i], &obtido);
    }

    he_fechar(he_local);
}

void test_duplicacao_do_diretorio_seguida_de_split_deve_ocorrer_com_capacidade_1(void) {
    HashExtensivel he_local;
    RegistroMoradorTeste esperado_a;
    RegistroMoradorTeste esperado_b;
    RegistroMoradorTeste obtido;
    char chave_a[32];
    char chave_b[32];
    int contador = 0;

    remove(ARQ_TESTE_CAP1);
    he_local = he_criar(ARQ_TESTE_CAP1, 1, &INFO_MORADOR);
    TEST_ASSERT_NOT_NULL(he_local);

    TEST_ASSERT_TRUE(gerar_chave_com_mascara("dup-", 6, 0u, &contador,
                                             chave_a, sizeof(chave_a)));
    TEST_ASSERT_TRUE(gerar_chave_com_mascara("dup-", 6, 32u, &contador,
                                             chave_b, sizeof(chave_b)));

    esperado_a = criar_registro(chave_a, "Primeiro", "Dup", 'F',
                                "09/09/1999", "cep-02", 'S', 10, "a");
    esperado_b = criar_registro(chave_b, "Segundo", "Dup", 'M',
                                "10/10/2000", "cep-03", 'L', 20, "b");

    TEST_ASSERT_TRUE(he_inserir(he_local, &esperado_a));
    TEST_ASSERT_TRUE(he_inserir(he_local, &esperado_b));

    TEST_ASSERT_EQUAL_UINT(2, he_tamanho(he_local));
    TEST_ASSERT_GREATER_OR_EQUAL_UINT(6, he_profundidade_global(he_local));
    TEST_ASSERT_GREATER_OR_EQUAL_UINT(64, he_tamanho_diretorio(he_local));

    memset(&obtido, 0, sizeof(obtido));
    TEST_ASSERT_TRUE(he_buscar(he_local, chave_a, &obtido));
    assert_registros_iguais(&esperado_a, &obtido);

    memset(&obtido, 0, sizeof(obtido));
    TEST_ASSERT_TRUE(he_buscar(he_local, chave_b, &obtido));
    assert_registros_iguais(&esperado_b, &obtido);

    he_fechar(he_local);
}

void test_reabertura_deve_manter_persistencia_dos_dados(void) {
    RegistroMoradorTeste esperado =
        criar_registro("55566677788", "Dora", "Pires", 'F', "05/05/1995",
                       "cep-55", 'L', 55, "blocoB");
    RegistroMoradorTeste obtido;

    memset(&obtido, 0, sizeof(obtido));

    TEST_ASSERT_TRUE(he_inserir(he, &esperado));

    he_fechar(he);
    he = NULL;

    he = he_abrir(ARQ_TESTE, &INFO_MORADOR);
    TEST_ASSERT_NOT_NULL(he);
    TEST_ASSERT_EQUAL_UINT(sizeof(RegistroMoradorTeste), he_tamanho_registro(he));
    TEST_ASSERT_EQUAL_UINT(1, he_tamanho(he));
    TEST_ASSERT_TRUE(he_buscar(he, "55566677788", &obtido));
    assert_registros_iguais(&esperado, &obtido);
}

void test_persistencia_apos_fechar_e_abrir_novamente_deve_preservar_colisoes_e_splits(void) {
    HashExtensivel he_local;
    RegistroMoradorTeste esperado[5];
    RegistroMoradorTeste obtido;
    char chaves[5][32];
    size_t mascaras[5] = {0u, 8u, 16u, 24u, 32u};
    int contador = 0;

    remove(ARQ_TESTE_CAP1_PERSIST);
    he_local = he_criar(ARQ_TESTE_CAP1_PERSIST, 1, &INFO_MORADOR);
    TEST_ASSERT_NOT_NULL(he_local);

    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_TRUE(gerar_chave_com_mascara("prs-", 6, mascaras[i],
                                                 &contador, chaves[i], sizeof(chaves[i])));
        esperado[i] = criar_registro(chaves[i], "Persist", "Split", 'F',
                                     "11/11/2001", "cep-04", 'O', i + 100, "persist");
        TEST_ASSERT_TRUE(he_inserir(he_local, &esperado[i]));
    }

    TEST_ASSERT_GREATER_OR_EQUAL_UINT(5, he_profundidade_global(he_local));
    he_fechar(he_local);

    he_local = he_abrir(ARQ_TESTE_CAP1_PERSIST, &INFO_MORADOR);
    TEST_ASSERT_NOT_NULL(he_local);
    TEST_ASSERT_EQUAL_UINT(5, he_tamanho(he_local));

    for (int i = 0; i < 5; i++) {
        memset(&obtido, 0, sizeof(obtido));
        TEST_ASSERT_TRUE(he_buscar(he_local, chaves[i], &obtido));
        assert_registros_iguais(&esperado[i], &obtido);
    }

    he_fechar(he_local);

    he_local = he_abrir(ARQ_TESTE_CAP1_PERSIST, &INFO_MORADOR);
    TEST_ASSERT_NOT_NULL(he_local);
    TEST_ASSERT_EQUAL_UINT(5, he_tamanho(he_local));

    for (int i = 0; i < 5; i++) {
        memset(&obtido, 0, sizeof(obtido));
        TEST_ASSERT_TRUE(he_buscar(he_local, chaves[i], &obtido));
        assert_registros_iguais(&esperado[i], &obtido);
    }

    he_fechar(he_local);
}

void test_registro_com_ponteiros_deve_funcionar_com_serializacao_customizada(void) {
    const char *arquivo_opaco = "hash_extensivel_opaco_teste.dat";
    HEInfoRegistro info_opaco = INFO_OPACO;
    HashExtensivel he_opaco;
    RegistroOpacoTeste esperado;
    RegistroOpacoTeste obtido;

    info_opaco.serializar = serializar_registro_opaco;
    info_opaco.desserializar = desserializar_registro_opaco;

    remove(arquivo_opaco);
    he_opaco = he_criar(arquivo_opaco, 2, &info_opaco);
    TEST_ASSERT_NOT_NULL(he_opaco);

    esperado = criar_registro_opaco("cpf-opaque", "Quadra Azul", "red", 42);
    obtido.cpf = NULL;
    obtido.nome = NULL;
    obtido.cor_borda = NULL;
    obtido.numero = 0;

    TEST_ASSERT_TRUE(he_inserir(he_opaco, &esperado));
    TEST_ASSERT_TRUE(he_buscar(he_opaco, "cpf-opaque", &obtido));
    TEST_ASSERT_EQUAL_STRING(esperado.cpf, obtido.cpf);
    TEST_ASSERT_EQUAL_STRING(esperado.nome, obtido.nome);
    TEST_ASSERT_EQUAL_STRING(esperado.cor_borda, obtido.cor_borda);
    TEST_ASSERT_EQUAL_INT(esperado.numero, obtido.numero);

    destruir_registro_opaco(&obtido);
    he_fechar(he_opaco);

    he_opaco = he_abrir(arquivo_opaco, &info_opaco);
    TEST_ASSERT_NOT_NULL(he_opaco);

    obtido.cpf = NULL;
    obtido.nome = NULL;
    obtido.cor_borda = NULL;
    obtido.numero = 0;

    TEST_ASSERT_TRUE(he_buscar(he_opaco, "cpf-opaque", &obtido));
    TEST_ASSERT_EQUAL_STRING(esperado.cpf, obtido.cpf);
    TEST_ASSERT_EQUAL_STRING(esperado.nome, obtido.nome);
    TEST_ASSERT_EQUAL_STRING(esperado.cor_borda, obtido.cor_borda);
    TEST_ASSERT_EQUAL_INT(esperado.numero, obtido.numero);

    destruir_registro_opaco(&esperado);
    destruir_registro_opaco(&obtido);
    he_fechar(he_opaco);
}

void test_chave_alfanumerica_com_hifen_no_limite_deve_funcionar(void) {
    RegistroMoradorTeste esperado =
        criar_registro("99988877766", "Eli", "Costa", 'M', "06/06/1996",
                       "cep-limite", 'S', 66, "fundos");
    RegistroMoradorTeste obtido;

    memset(esperado.cpf, 'A', sizeof(esperado.cpf) - 1);
    esperado.cpf[0] = 'C';
    esperado.cpf[1] = 'E';
    esperado.cpf[2] = 'P';
    esperado.cpf[3] = '-';
    esperado.cpf[sizeof(esperado.cpf) - 1] = '\0';
    memset(&obtido, 0, sizeof(obtido));

    TEST_ASSERT_TRUE(he_inserir(he, &esperado));
    TEST_ASSERT_TRUE(he_contem(he, esperado.cpf));

    he_fechar(he);
    he = NULL;

    he = he_abrir(ARQ_TESTE, &INFO_MORADOR);
    TEST_ASSERT_NOT_NULL(he);
    TEST_ASSERT_TRUE(he_buscar(he, esperado.cpf, &obtido));
    assert_registros_iguais(&esperado, &obtido);
}

void test_parametros_invalidos_devem_falhar(void) {
    HEInfoRegistro info_invalida = INFO_MORADOR;
    RegistroMoradorTeste registro =
        criar_registro("00011122233", "Fabi", "Rocha", 'F', "07/07/1997",
                       "cep-10", 'N', 10, "casa");
    RegistroMoradorTeste obtido;

    memset(&obtido, 0, sizeof(obtido));

    TEST_ASSERT_NULL(he_criar(NULL, 2, &INFO_MORADOR));
    TEST_ASSERT_NULL(he_criar("x.dat", 0, &INFO_MORADOR));
    info_invalida.tamanho_registro = 0;
    TEST_ASSERT_NULL(he_criar("x.dat", 2, &info_invalida));
    info_invalida = INFO_MORADOR;
    info_invalida.chave_offset = sizeof(RegistroMoradorTeste);
    TEST_ASSERT_NULL(he_criar("x.dat", 2, &info_invalida));
    info_invalida = INFO_MORADOR;
    info_invalida.chave_tamanho = 0;
    TEST_ASSERT_NULL(he_criar("x.dat", 2, &info_invalida));
    info_invalida = INFO_MORADOR;
    info_invalida.serializar = NULL;
    TEST_ASSERT_NULL(he_criar("x.dat", 2, &info_invalida));
    info_invalida = INFO_MORADOR;
    info_invalida.desserializar = NULL;
    TEST_ASSERT_NULL(he_criar("x.dat", 2, &info_invalida));

    TEST_ASSERT_FALSE(he_inserir(NULL, &registro));
    TEST_ASSERT_FALSE(he_inserir(he, NULL));
    memset(registro.cpf, 0, sizeof(registro.cpf));
    TEST_ASSERT_FALSE(he_inserir(he, &registro));
    strncpy(registro.cpf, "cpf@invalido", sizeof(registro.cpf) - 1);
    TEST_ASSERT_FALSE(he_inserir(he, &registro));
    memset(registro.cpf, 'A', sizeof(registro.cpf));
    TEST_ASSERT_FALSE(he_inserir(he, &registro));

    TEST_ASSERT_FALSE(he_buscar(NULL, "x", &obtido));
    TEST_ASSERT_FALSE(he_buscar(he, "x", NULL));
    TEST_ASSERT_FALSE(he_buscar(he, NULL, &obtido));
    TEST_ASSERT_FALSE(he_buscar(he, "cpf@invalido", &obtido));
    TEST_ASSERT_FALSE(he_remover(NULL, "x"));
    TEST_ASSERT_FALSE(he_remover(he, "cpf@invalido"));
    TEST_ASSERT_NULL(he_abrir(ARQ_TESTE, NULL));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_criacao_deve_funcionar);
    RUN_TEST(test_abrir_arquivo_inexistente_deve_retornar_null);
    RUN_TEST(test_inserir_e_buscar_registro_completo_deve_funcionar);
    RUN_TEST(test_atualizacao_deve_substituir_registro_completo);
    RUN_TEST(test_persistencia_controlada_do_diretorio_nao_deve_crescer_arquivo_sem_split);
    RUN_TEST(test_remocao_deve_funcionar);
    RUN_TEST(test_split_e_duplicacao_de_diretorio_devem_funcionar);
    RUN_TEST(test_bucket_com_capacidade_1_deve_suportar_colisoes_e_split_sucessivo);
    RUN_TEST(test_duplicacao_do_diretorio_seguida_de_split_deve_ocorrer_com_capacidade_1);
    RUN_TEST(test_reabertura_deve_manter_persistencia_dos_dados);
    RUN_TEST(test_persistencia_apos_fechar_e_abrir_novamente_deve_preservar_colisoes_e_splits);
    RUN_TEST(test_registro_com_ponteiros_deve_funcionar_com_serializacao_customizada);
    RUN_TEST(test_chave_alfanumerica_com_hifen_no_limite_deve_funcionar);
    RUN_TEST(test_parametros_invalidos_devem_falhar);

    return UNITY_END();
}
