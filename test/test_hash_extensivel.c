#include "../unity/unity.h"
#include "../include/hash_extensivel.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct HE_PACKED {
    char chave[HE_TAMANHO_CHAVE_MAX + 1];
    char nome[40];
    char sobrenome[40];
    char sexo;
    char nasc[11];
    char cep[20];
    char face;
    int num;
    char compl[32];
} RegistroPessoaTeste;

static const char *ARQ_HASH = "hash_extensivel_teste.hf";
static const char *ARQ_CONTROLE = "hash_extensivel_teste.hfc";
static const char *ARQ_DUMP = "hash_extensivel_teste.hfd";
static HashExtensivel he;

static RegistroPessoaTeste criar_registro(const char *chave, const char *nome,
                                          const char *sobrenome, char sexo,
                                          const char *nasc, const char *cep,
                                          char face, int num,
                                          const char *compl) {
    RegistroPessoaTeste registro;

    memset(&registro, 0, sizeof(registro));
    strncpy(registro.chave, chave, sizeof(registro.chave) - 1);
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

static void assert_registros_iguais(const RegistroPessoaTeste *esperado,
                                    const RegistroPessoaTeste *obtido) {
    TEST_ASSERT_EQUAL_STRING(esperado->chave, obtido->chave);
    TEST_ASSERT_EQUAL_STRING(esperado->nome, obtido->nome);
    TEST_ASSERT_EQUAL_STRING(esperado->sobrenome, obtido->sobrenome);
    TEST_ASSERT_EQUAL_CHAR(esperado->sexo, obtido->sexo);
    TEST_ASSERT_EQUAL_STRING(esperado->nasc, obtido->nasc);
    TEST_ASSERT_EQUAL_STRING(esperado->cep, obtido->cep);
    TEST_ASSERT_EQUAL_CHAR(esperado->face, obtido->face);
    TEST_ASSERT_EQUAL_INT(esperado->num, obtido->num);
    TEST_ASSERT_EQUAL_STRING(esperado->compl, obtido->compl);
}

static uint64_t hash_texto_teste(const char *texto) {
    uint64_t hash = 0;
    int i;

    for (i = 0; texto[i] != '\0'; i++) {
        hash = hash * 31u + (unsigned char)texto[i];
    }

    return hash;
}

static int gerar_chave_com_mascara(const char *prefixo, size_t profundidade,
                                   size_t mascara_alvo, int *contador_io,
                                   char *saida, size_t tamanho_saida) {
    size_t mascara;
    int contador;
    int tentativas;

    mascara = profundidade == 0 ? 0u : (((size_t)1u) << profundidade) - 1u;
    contador = *contador_io;

    for (tentativas = 0; tentativas < 200000; tentativas++, contador++) {
        snprintf(saida, tamanho_saida, "%s%d", prefixo, contador);
        if (((size_t)hash_texto_teste(saida) & mascara) == mascara_alvo) {
            *contador_io = contador + 1;
            return 1;
        }
    }

    return 0;
}

static int ler_profundidade_do_dump(const char *caminho_dump) {
    FILE *arquivo = fopen(caminho_dump, "r");
    char linha[256];
    int profundidade = -1;

    if (arquivo == NULL) {
        return -1;
    }

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        if (sscanf(linha, "Profundidade global: %d", &profundidade) == 1) {
            break;
        }
    }

    fclose(arquivo);
    return profundidade;
}

static int ler_quantidade_expansoes_do_dump(const char *caminho_dump) {
    FILE *arquivo = fopen(caminho_dump, "r");
    char linha[256];
    int quantidade = -1;

    if (arquivo == NULL) {
        return -1;
    }

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        if (sscanf(linha, "Quantidade expansoes: %d", &quantidade) == 1) {
            break;
        }
    }

    fclose(arquivo);
    return quantidade;
}

static char *ler_arquivo_texto(const char *caminho) {
    FILE *arquivo = fopen(caminho, "r");
    long tamanho;
    char *conteudo;

    if (arquivo == NULL) {
        return NULL;
    }

    TEST_ASSERT_EQUAL_INT(0, fseek(arquivo, 0L, SEEK_END));
    tamanho = ftell(arquivo);
    TEST_ASSERT_TRUE(tamanho >= 0);
    TEST_ASSERT_EQUAL_INT(0, fseek(arquivo, 0L, SEEK_SET));

    conteudo = (char *)calloc((size_t)tamanho + 1u, sizeof(char));
    TEST_ASSERT_NOT_NULL(conteudo);
    if (tamanho > 0) {
        TEST_ASSERT_EQUAL_size_t((size_t)tamanho,
                                 fread(conteudo, 1u, (size_t)tamanho, arquivo));
    }

    fclose(arquivo);
    return conteudo;
}

void setUp(void) {
    remove(ARQ_HASH);
    remove(ARQ_CONTROLE);
    remove(ARQ_DUMP);
    he = he_criar(ARQ_HASH, 2, sizeof(RegistroPessoaTeste));
}

void tearDown(void) {
    he_fechar(he);
    he = NULL;
    remove(ARQ_HASH);
    remove(ARQ_CONTROLE);
    remove(ARQ_DUMP);
}

void test_he_criar_deve_inicializar_estrutura_vazia(void) {
    FILE *arquivo_hash;
    FILE *arquivo_controle;

    TEST_ASSERT_NOT_NULL(he);
    TEST_ASSERT_EQUAL_UINT(sizeof(RegistroPessoaTeste), he_tamanho_registro(he));
    TEST_ASSERT_EQUAL_UINT(0, he_tamanho(he));
    TEST_ASSERT_EQUAL_UINT(1, he_tamanho_diretorio(he));
    TEST_ASSERT_FALSE(he_contem(he, "nao-existe"));

    arquivo_hash = fopen(ARQ_HASH, "rb");
    arquivo_controle = fopen(ARQ_CONTROLE, "rb");
    TEST_ASSERT_NOT_NULL(arquivo_hash);
    TEST_ASSERT_NOT_NULL(arquivo_controle);
    fclose(arquivo_hash);
    fclose(arquivo_controle);
}

void test_he_inserir_e_buscar_devem_funcionar(void) {
    RegistroPessoaTeste esperado =
        criar_registro("cpf-1", "Ana", "Silva", 'F', "10/01/1999", "cep-10", 'N', 10, "apto");
    RegistroPessoaTeste obtido;

    memset(&obtido, 0, sizeof(obtido));
    TEST_ASSERT_TRUE(he_inserir(he, &esperado));
    TEST_ASSERT_TRUE(he_buscar(he, "cpf-1", &obtido));
    assert_registros_iguais(&esperado, &obtido);
}

void test_he_insercao_duplicada_deve_falhar(void) {
    RegistroPessoaTeste registro =
        criar_registro("cpf-2", "Bia", "Souza", 'F', "01/02/2000", "cep-20", 'S', 20, "fundos");

    TEST_ASSERT_TRUE(he_inserir(he, &registro));
    TEST_ASSERT_FALSE(he_inserir(he, &registro));
}

void test_he_atualizar_deve_substituir_registro_completo(void) {
    RegistroPessoaTeste antigo =
        criar_registro("cpf-3", "Caio", "Lima", 'M', "03/03/1998", "cep-01", 'O', 10, "sobrado");
    RegistroPessoaTeste novo =
        criar_registro("cpf-3", "Carlos", "Lima", 'M', "03/03/1998", "cep-55", 'L', 55, "casa");
    RegistroPessoaTeste obtido;

    memset(&obtido, 0, sizeof(obtido));
    TEST_ASSERT_TRUE(he_inserir(he, &antigo));
    TEST_ASSERT_TRUE(he_atualizar(he, &novo));
    TEST_ASSERT_TRUE(he_buscar(he, "cpf-3", &obtido));
    assert_registros_iguais(&novo, &obtido);
}

void test_he_remover_deve_eliminar_registro(void) {
    RegistroPessoaTeste registro =
        criar_registro("cpf-4", "Dora", "Pires", 'F', "05/05/1995", "cep-55", 'L', 55, "blocoB");

    TEST_ASSERT_TRUE(he_inserir(he, &registro));
    TEST_ASSERT_TRUE(he_remover(he, "cpf-4"));
    TEST_ASSERT_FALSE(he_contem(he, "cpf-4"));
    TEST_ASSERT_FALSE(he_remover(he, "cpf-4"));
}

void test_he_split_e_reabertura_devem_preservar_registros_completos(void) {
    char chaves[6][32];
    RegistroPessoaTeste esperados[6];
    RegistroPessoaTeste obtido;
    char *dump;
    int contador = 0;
    int profundidade_dump;
    int quantidade_expansoes_dump;
    int i;

    for (i = 0; i < 6; i++) {
        TEST_ASSERT_TRUE(gerar_chave_com_mascara("col-", 3, 0u, &contador,
                                                 chaves[i], sizeof(chaves[i])));
        esperados[i] = criar_registro(chaves[i], "Nome", "Teste", 'M', "04/04/1997",
                                      "cep-01", 'N', i + 1, chaves[i]);
        TEST_ASSERT_TRUE(he_inserir(he, &esperados[i]));
    }

    he_dump(he, ARQ_DUMP);
    dump = ler_arquivo_texto(ARQ_DUMP);
    TEST_ASSERT_NOT_NULL(dump);
    TEST_ASSERT_NOT_NULL(strstr(dump, "DUMP"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "*Dump cabecalho"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "* Dump table"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "*Dump buckets"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "*Dump expansoes"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "*Resumo hash extensivel"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "BLOCO: 0"));
    free(dump);

    profundidade_dump = ler_profundidade_do_dump(ARQ_DUMP);
    quantidade_expansoes_dump = ler_quantidade_expansoes_do_dump(ARQ_DUMP);
    TEST_ASSERT_GREATER_OR_EQUAL(1, profundidade_dump);
    TEST_ASSERT_GREATER_OR_EQUAL(1, quantidade_expansoes_dump);
    TEST_ASSERT_GREATER_THAN_UINT(1, he_tamanho_diretorio(he));

    he_fechar(he);
    he = NULL;

    he = he_abrir(ARQ_HASH, 2, sizeof(RegistroPessoaTeste));
    TEST_ASSERT_NOT_NULL(he);
    TEST_ASSERT_EQUAL_UINT(6, he_tamanho(he));

    for (i = 0; i < 6; i++) {
        memset(&obtido, 0, sizeof(obtido));
        TEST_ASSERT_TRUE(he_buscar(he, chaves[i], &obtido));
        assert_registros_iguais(&esperados[i], &obtido);
    }
}

void test_he_parametros_invalidos_devem_falhar(void) {
    RegistroPessoaTeste registro =
        criar_registro("cpf-5", "Eva", "Melo", 'F', "01/01/2001", "cep-10", 'N', 10, "casa");
    RegistroPessoaTeste sem_terminador;

    memset(&sem_terminador, 'A', sizeof(sem_terminador));

    TEST_ASSERT_NULL(he_criar(NULL, 2, sizeof(RegistroPessoaTeste)));
    TEST_ASSERT_NULL(he_criar("hash_sem_extensao.bin", 2, sizeof(RegistroPessoaTeste)));
    TEST_ASSERT_NULL(he_criar(ARQ_HASH, 0, sizeof(RegistroPessoaTeste)));
    TEST_ASSERT_NULL(he_criar(ARQ_HASH, 2, 0));
    TEST_ASSERT_FALSE(he_inserir(NULL, &registro));
    TEST_ASSERT_FALSE(he_inserir(he, NULL));
    registro.chave[0] = '\0';
    TEST_ASSERT_FALSE(he_inserir(he, &registro));
    strncpy(registro.chave, "cpf@erro", sizeof(registro.chave) - 1);
    TEST_ASSERT_FALSE(he_inserir(he, &registro));
    TEST_ASSERT_FALSE(he_inserir(he, &sem_terminador));
    TEST_ASSERT_FALSE(he_buscar(NULL, "x", &registro));
    TEST_ASSERT_FALSE(he_buscar(he, NULL, &registro));
    TEST_ASSERT_FALSE(he_remover(NULL, "x"));
    TEST_ASSERT_FALSE(he_remover(he, NULL));
    TEST_ASSERT_FALSE(he_atualizar(NULL, &registro));
    TEST_ASSERT_FALSE(he_atualizar(he, NULL));
    TEST_ASSERT_NULL(he_abrir("hash_sem_extensao.bin", 2, sizeof(RegistroPessoaTeste)));
    TEST_ASSERT_NULL(he_abrir(ARQ_HASH, 3, sizeof(RegistroPessoaTeste)));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_he_criar_deve_inicializar_estrutura_vazia);
    RUN_TEST(test_he_inserir_e_buscar_devem_funcionar);
    RUN_TEST(test_he_insercao_duplicada_deve_falhar);
    RUN_TEST(test_he_atualizar_deve_substituir_registro_completo);
    RUN_TEST(test_he_remover_deve_eliminar_registro);
    RUN_TEST(test_he_split_e_reabertura_devem_preservar_registros_completos);
    RUN_TEST(test_he_parametros_invalidos_devem_falhar);

    return UNITY_END();
}
