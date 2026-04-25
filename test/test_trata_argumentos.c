#include "../unity/unity.h"
#include "../include/trata_argumentos.h"

#include <stdlib.h>

static int argc_atual;
static char **argv_atual;

void setUp(void) {
    argc_atual = 0;
    argv_atual = 0;
}

void tearDown(void) {
    argc_atual = 0;
    argv_atual = 0;
}

void test_obter_valor_opcao_deve_retornar_valor_quando_opcao_existe(void) {
    char *argv[] = {"siguel", "-e", "/tmp/entrada", "-q", "consultas.qry"};

    argc_atual = 5;
    argv_atual = argv;

    TEST_ASSERT_EQUAL_STRING("/tmp/entrada", obter_valor_opcao(argc_atual, argv_atual, "e"));
    TEST_ASSERT_EQUAL_STRING("consultas.qry", obter_valor_opcao(argc_atual, argv_atual, "q"));
}

void test_obter_valor_opcao_deve_retornar_null_quando_opcao_nao_existe(void) {
    char *argv[] = {"siguel", "-e", "/tmp/entrada"};

    argc_atual = 3;
    argv_atual = argv;

    TEST_ASSERT_NULL(obter_valor_opcao(argc_atual, argv_atual, "q"));
}

void test_obter_valor_opcao_deve_retornar_null_quando_nao_ha_valor_apos_opcao(void) {
    char *argv[] = {"siguel", "-e"};

    argc_atual = 2;
    argv_atual = argv;

    TEST_ASSERT_NULL(obter_valor_opcao(argc_atual, argv_atual, "e"));
}

void test_obter_sufixo_comando_deve_retornar_ultimo_argumento_sem_hifen(void) {
    char *argv[] = {"siguel", "-e", "/tmp/entrada", "-q", "consultas.qry", "saida"};

    argc_atual = 6;
    argv_atual = argv;

    TEST_ASSERT_EQUAL_STRING("saida", obter_sufixo_comando(argc_atual, argv_atual));
}

void test_obter_sufixo_comando_deve_retornar_null_quando_nao_ha_sufixo(void) {
    char *argv[] = {"siguel", "-e", "/tmp/entrada", "-q", "consultas.qry"};

    argc_atual = 5;
    argv_atual = argv;

    TEST_ASSERT_NULL(obter_sufixo_comando(argc_atual, argv_atual));
}

void test_obter_sufixo_comando_deve_retornar_sufixo_quando_apenas_programa_e_sufixo_existem(void) {
    char *argv[] = {"siguel", "saida"};

    argc_atual = 2;
    argv_atual = argv;

    TEST_ASSERT_EQUAL_STRING("saida", obter_sufixo_comando(argc_atual, argv_atual));
}

void test_montar_caminho_entrada_deve_concatenar_bed_e_nome_relativo(void) {
    char *caminho = montar_caminho_entrada("/tmp/entrada", "cidade.geo");

    TEST_ASSERT_NOT_NULL(caminho);
    TEST_ASSERT_EQUAL_STRING("/tmp/entrada/cidade.geo", caminho);
    free(caminho);
}

void test_montar_caminho_entrada_deve_respeitar_caminho_absoluto(void) {
    char *caminho = montar_caminho_entrada("/tmp/entrada", "/dados/cidade.geo");

    TEST_ASSERT_NOT_NULL(caminho);
    TEST_ASSERT_EQUAL_STRING("/dados/cidade.geo", caminho);
    free(caminho);
}

void test_montar_caminho_entrada_deve_usar_diretorio_corrente_quando_bed_ausente(void) {
    char *caminho = montar_caminho_entrada(NULL, "cidade.geo");

    TEST_ASSERT_NOT_NULL(caminho);
    TEST_ASSERT_EQUAL_STRING("./cidade.geo", caminho);
    free(caminho);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_obter_valor_opcao_deve_retornar_valor_quando_opcao_existe);
    RUN_TEST(test_obter_valor_opcao_deve_retornar_null_quando_opcao_nao_existe);
    RUN_TEST(test_obter_valor_opcao_deve_retornar_null_quando_nao_ha_valor_apos_opcao);
    RUN_TEST(test_obter_sufixo_comando_deve_retornar_ultimo_argumento_sem_hifen);
    RUN_TEST(test_obter_sufixo_comando_deve_retornar_null_quando_nao_ha_sufixo);
    RUN_TEST(test_obter_sufixo_comando_deve_retornar_sufixo_quando_apenas_programa_e_sufixo_existem);
    RUN_TEST(test_montar_caminho_entrada_deve_concatenar_bed_e_nome_relativo);
    RUN_TEST(test_montar_caminho_entrada_deve_respeitar_caminho_absoluto);
    RUN_TEST(test_montar_caminho_entrada_deve_usar_diretorio_corrente_quando_bed_ausente);

    return UNITY_END();
}
