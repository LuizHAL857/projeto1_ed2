#include "unity.h"
#include "hash_extensivel.h"

#include <stdio.h>

static HashExtensivel he;
static const char *ARQ_TESTE = "hash_extensivel_teste.dat";

void setUp(void) {
    remove(ARQ_TESTE);
    he = he_criar(ARQ_TESTE, 2);
}

void tearDown(void) {
    he_fechar(he);
    he = NULL;
    remove(ARQ_TESTE);
}

void test_criacao_deve_funcionar(void) {
    TEST_ASSERT_NOT_NULL(he);
    TEST_ASSERT_EQUAL_UINT(0, he_tamanho(he));
    TEST_ASSERT_EQUAL_UINT(0, he_profundidade_global(he));
    TEST_ASSERT_EQUAL_UINT(1, he_tamanho_diretorio(he));
}

void test_abrir_arquivo_inexistente_deve_retornar_null(void) {
    HashExtensivel outro = he_abrir("arquivo_que_nao_existe_123.dat");
    TEST_ASSERT_NULL(outro);
}

void test_inserir_e_buscar_deve_funcionar(void) {
    int valor = 0;

    TEST_ASSERT_TRUE(he_inserir(he, 10, 100));
    TEST_ASSERT_TRUE(he_buscar(he, 10, &valor));
    TEST_ASSERT_EQUAL_INT(100, valor);
}

void test_atualizacao_deve_funcionar(void) {
    int valor = 0;

    TEST_ASSERT_TRUE(he_inserir(he, 7, 70));
    TEST_ASSERT_TRUE(he_inserir(he, 7, 700));

    TEST_ASSERT_EQUAL_UINT(1, he_tamanho(he));
    TEST_ASSERT_TRUE(he_buscar(he, 7, &valor));
    TEST_ASSERT_EQUAL_INT(700, valor);
}

void test_remocao_deve_funcionar(void) {
    TEST_ASSERT_TRUE(he_inserir(he, 1, 10));
    TEST_ASSERT_TRUE(he_remover(he, 1));
    TEST_ASSERT_FALSE(he_contem(he, 1));
    TEST_ASSERT_EQUAL_UINT(0, he_tamanho(he));
}

void test_split_e_duplicacao_de_diretorio_devem_funcionar(void) {
    int valor = 0;

    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_TRUE(he_inserir(he, i, i * 10));
    }

    TEST_ASSERT_GREATER_THAN(0, he_profundidade_global(he));
    TEST_ASSERT_EQUAL_UINT(10, he_tamanho(he));

    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_TRUE(he_buscar(he, i, &valor));
        TEST_ASSERT_EQUAL_INT(i * 10, valor);
    }
}

void test_persistencia_apos_fechar_e_abrir(void) {
    int valor = 0;

    TEST_ASSERT_TRUE(he_inserir(he, 3, 30));
    TEST_ASSERT_TRUE(he_inserir(he, 4, 40));
    TEST_ASSERT_TRUE(he_inserir(he, 5, 50));

    he_fechar(he);
    he = NULL;

    he = he_abrir(ARQ_TESTE);
    TEST_ASSERT_NOT_NULL(he);

    TEST_ASSERT_EQUAL_UINT(3, he_tamanho(he));
    TEST_ASSERT_TRUE(he_buscar(he, 3, &valor));
    TEST_ASSERT_EQUAL_INT(30, valor);
    TEST_ASSERT_TRUE(he_buscar(he, 4, &valor));
    TEST_ASSERT_EQUAL_INT(40, valor);
    TEST_ASSERT_TRUE(he_buscar(he, 5, &valor));
    TEST_ASSERT_EQUAL_INT(50, valor);
}

void test_parametros_invalidos_devem_falhar(void) {
    int valor = 0;

    TEST_ASSERT_NULL(he_criar(NULL, 2));
    TEST_ASSERT_NULL(he_criar("x.dat", 0));
    TEST_ASSERT_FALSE(he_inserir(NULL, 1, 1));
    TEST_ASSERT_FALSE(he_buscar(NULL, 1, &valor));
    TEST_ASSERT_FALSE(he_buscar(he, 1, NULL));
    TEST_ASSERT_FALSE(he_remover(NULL, 1));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_criacao_deve_funcionar);
    RUN_TEST(test_abrir_arquivo_inexistente_deve_retornar_null);
    RUN_TEST(test_inserir_e_buscar_deve_funcionar);
    RUN_TEST(test_atualizacao_deve_funcionar);
    RUN_TEST(test_remocao_deve_funcionar);
    RUN_TEST(test_split_e_duplicacao_de_diretorio_devem_funcionar);
    RUN_TEST(test_persistencia_apos_fechar_e_abrir);
    RUN_TEST(test_parametros_invalidos_devem_falhar);

    return UNITY_END();
}