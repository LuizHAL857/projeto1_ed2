#include "../unity/unity.h"
#include "../include/leitor_arquivos.h"

#include <stdio.h>

static const char *ARQ_TESTE = "test_leitor_arquivos_entrada.txt";

void setUp(void) {
    FILE *arquivo = fopen(ARQ_TESTE, "w");

    TEST_ASSERT_NOT_NULL(arquivo);
    fputs("linha 1\n", arquivo);
    fputs("linha 2\n", arquivo);
    fputs("linha 3\n", arquivo);
    fclose(arquivo);
}

void tearDown(void) {
    remove(ARQ_TESTE);
}

void test_criar_dados_arquivo_deve_ler_arquivo_com_sucesso(void) {
    DadosDoArquivo dados = criar_dados_arquivo((char *)ARQ_TESTE);
    Lista linhas;

    TEST_ASSERT_NOT_NULL(dados);

    linhas = obter_lista_linhas(dados);
    TEST_ASSERT_NOT_NULL(linhas);
    TEST_ASSERT_EQUAL_INT(3, getTamanhoLista(linhas));

    destruir_dados_arquivo(dados);
}

void test_obter_caminho_e_nome_arquivo_devem_retornar_valores_esperados(void) {
    DadosDoArquivo dados = criar_dados_arquivo((char *)ARQ_TESTE);

    TEST_ASSERT_NOT_NULL(dados);
    TEST_ASSERT_EQUAL_STRING(ARQ_TESTE, obter_caminho_arquivo(dados));
    TEST_ASSERT_EQUAL_STRING(ARQ_TESTE, obter_nome_arquivo(dados));

    destruir_dados_arquivo(dados);
}

void test_obter_lista_linhas_deve_manter_ordem_do_arquivo(void) {
    DadosDoArquivo dados = criar_dados_arquivo((char *)ARQ_TESTE);
    Lista linhas;
    Celula atual;

    TEST_ASSERT_NOT_NULL(dados);

    linhas = obter_lista_linhas(dados);
    atual = getInicioLista(linhas);

    TEST_ASSERT_EQUAL_STRING("linha 1", (char *)getConteudoCelula(atual));
    atual = getProxCelula(atual);
    TEST_ASSERT_EQUAL_STRING("linha 2", (char *)getConteudoCelula(atual));
    atual = getProxCelula(atual);
    TEST_ASSERT_EQUAL_STRING("linha 3", (char *)getConteudoCelula(atual));
    TEST_ASSERT_NULL(getProxCelula(atual));

    destruir_dados_arquivo(dados);
}

void test_criar_dados_arquivo_deve_retornar_null_para_arquivo_inexistente(void) {
    DadosDoArquivo dados = criar_dados_arquivo("arquivo_inexistente_123.txt");

    TEST_ASSERT_NULL(dados);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_criar_dados_arquivo_deve_ler_arquivo_com_sucesso);
    RUN_TEST(test_obter_caminho_e_nome_arquivo_devem_retornar_valores_esperados);
    RUN_TEST(test_obter_lista_linhas_deve_manter_ordem_do_arquivo);
    RUN_TEST(test_criar_dados_arquivo_deve_retornar_null_para_arquivo_inexistente);

    return UNITY_END();
}
