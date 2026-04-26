#include "../unity/unity.h"
#include "../include/leitor_arquivos.h"
#include "../include/quadra.h"
#include "../include/trata_geo.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static const char *ARQ_GEO = "trata_geo_entrada.geo";
static const char *DIR_SAIDA = "trata_geo_saida";
static const char *ARQ_SVG = "trata_geo_saida/trata_geo_entrada.svg";
static const char *ARQ_HASH = "trata_geo_saida/trata_geo_entrada-quadras.hf";
static const char *ARQ_CONTROLE = "trata_geo_saida/trata_geo_entrada-quadras.hfc";
static const char *ARQ_DUMP = "trata_geo_saida/trata_geo_entrada-quadras.hfd";

static TrataGeo processamento;
static DadosDoArquivo dados_geo;

static bool arquivo_existe(const char *caminho) {
    FILE *arquivo = fopen(caminho, "r");

    if (arquivo == NULL) {
        return false;
    }

    fclose(arquivo);
    return true;
}

static char *ler_arquivo_texto(const char *caminho) {
    FILE *arquivo;
    long tamanho;
    char *conteudo;

    arquivo = fopen(caminho, "r");
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

static void sobrescrever_geo(const char *conteudo) {
    FILE *arquivo = fopen(ARQ_GEO, "w");

    TEST_ASSERT_NOT_NULL(arquivo);
    TEST_ASSERT_TRUE(fputs(conteudo, arquivo) >= 0);
    fclose(arquivo);
}

void setUp(void) {
    processamento = NULL;
    dados_geo = NULL;

    remove(ARQ_SVG);
    remove(ARQ_HASH);
    remove(ARQ_CONTROLE);
    remove(ARQ_DUMP);
    remove(ARQ_GEO);
    rmdir(DIR_SAIDA);

    TEST_ASSERT_TRUE(mkdir(DIR_SAIDA, 0777) == 0);
}

void tearDown(void) {
    trata_geo_destruir(processamento);
    processamento = NULL;
    destruir_dados_arquivo(dados_geo);
    dados_geo = NULL;

    remove(ARQ_SVG);
    remove(ARQ_HASH);
    remove(ARQ_CONTROLE);
    remove(ARQ_DUMP);
    remove(ARQ_GEO);
    rmdir(DIR_SAIDA);
}

void test_processa_geo_deve_gerar_svg_inicial_e_permitir_busca_por_cep(void) {
    Quadra quadra;
    char *dump;
    char *svg;

    sobrescrever_geo(
        "cq 2px beige brown\n"
        "q Q1 10 20 30 40\n"
        "cq 3px gold navy\n"
        "q Q2 100 120 50 60\n"
        "q Q3 150 180 30 30\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);

    processamento = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento);
    TEST_ASSERT_EQUAL_STRING("trata_geo_entrada", trata_geo_obter_nome_geo(processamento));

    TEST_ASSERT_TRUE(arquivo_existe(ARQ_SVG));
    TEST_ASSERT_TRUE(arquivo_existe(ARQ_HASH));
    TEST_ASSERT_TRUE(arquivo_existe(ARQ_CONTROLE));
    TEST_ASSERT_TRUE(arquivo_existe(ARQ_DUMP));

    quadra = trata_geo_obter_quadra(processamento, "Q2");
    TEST_ASSERT_NOT_NULL(quadra);
    TEST_ASSERT_EQUAL_STRING("Q2", quadra_obter_cep(quadra));
    TEST_ASSERT_TRUE(fabs(quadra_obter_x(quadra) - 100.0) <= 0.0001);
    TEST_ASSERT_TRUE(fabs(quadra_obter_y(quadra) - 120.0) <= 0.0001);
    TEST_ASSERT_TRUE(fabs(quadra_obter_w(quadra) - 50.0) <= 0.0001);
    TEST_ASSERT_TRUE(fabs(quadra_obter_h(quadra) - 60.0) <= 0.0001);
    TEST_ASSERT_EQUAL_STRING("3px", quadra_obter_sw(quadra));
    TEST_ASSERT_EQUAL_STRING("gold", quadra_obter_cfill(quadra));
    TEST_ASSERT_EQUAL_STRING("navy", quadra_obter_cstrk(quadra));
    quadra_destruir(quadra);

    svg = ler_arquivo_texto(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(svg);
    TEST_ASSERT_NOT_NULL(strstr(svg, "<svg"));
    TEST_ASSERT_NOT_NULL(strstr(svg, "Q1"));
    TEST_ASSERT_NOT_NULL(strstr(svg, "Q2"));
    TEST_ASSERT_NOT_NULL(strstr(svg, "gold"));
    TEST_ASSERT_NOT_NULL(strstr(svg, "navy"));
    TEST_ASSERT_NOT_NULL(strstr(svg, "<text x=\"-16.00\" y=\"-8.00\""));
    TEST_ASSERT_NOT_NULL(strstr(svg, "fill-opacity=\"0.70\""));
    TEST_ASSERT_NOT_NULL(strstr(svg, "font-weight=\"bold\""));
    free(svg);

    dump = ler_arquivo_texto(ARQ_DUMP);
    TEST_ASSERT_NOT_NULL(dump);
    TEST_ASSERT_NOT_NULL(strstr(dump, "DUMP"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "*Dump cabecalho"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "* Dump table"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "*Dump buckets"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "*Dump expansoes"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "*Resumo hash extensivel"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "Arquivo .hf: trata_geo_saida/trata_geo_entrada-quadras.hf"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "BLOCO: 0"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "| Q1 |"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "| Q2 |"));
    TEST_ASSERT_NOT_NULL(strstr(dump, "| Q3 |"));
    free(dump);
}

void test_processa_geo_deve_aceitar_cep_com_ponto(void) {
    Quadra quadra;
    char *svg;

    sobrescrever_geo(
        "cq 1.0px steelblue MistyRose\n"
        "q b01.1 95 95 120 80\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);

    processamento = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento);

    quadra = trata_geo_obter_quadra(processamento, "b01.1");
    TEST_ASSERT_NOT_NULL(quadra);
    TEST_ASSERT_EQUAL_STRING("b01.1", quadra_obter_cep(quadra));
    TEST_ASSERT_EQUAL_STRING("1.0px", quadra_obter_sw(quadra));
    TEST_ASSERT_EQUAL_STRING("steelblue", quadra_obter_cfill(quadra));
    TEST_ASSERT_EQUAL_STRING("MistyRose", quadra_obter_cstrk(quadra));
    quadra_destruir(quadra);

    svg = ler_arquivo_texto(ARQ_SVG);
    TEST_ASSERT_NOT_NULL(svg);
    TEST_ASSERT_NOT_NULL(strstr(svg, "b01.1"));
    free(svg);
}

void test_processa_geo_deve_ignorar_linhas_vazias_e_comentarios(void) {
    Quadra quadra;

    sobrescrever_geo(
        "\n"
        "# comentario\n"
        "   \n"
        "cq 4px white black\n"
        "   # outro comentario\n"
        "q C1 5 10 15 20\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);

    processamento = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento);

    quadra = trata_geo_obter_quadra(processamento, "C1");
    TEST_ASSERT_NOT_NULL(quadra);
    TEST_ASSERT_EQUAL_STRING("4px", quadra_obter_sw(quadra));
    TEST_ASSERT_EQUAL_STRING("white", quadra_obter_cfill(quadra));
    TEST_ASSERT_EQUAL_STRING("black", quadra_obter_cstrk(quadra));
    quadra_destruir(quadra);
}

void test_processa_geo_deve_falhar_para_comando_invalido_sem_deixar_saida_valida(void) {
    sobrescrever_geo(
        "q A1 0 0 10 10\n"
        "zz comando invalido\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);

    processamento = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NULL(processamento);
    TEST_ASSERT_FALSE(arquivo_existe(ARQ_SVG));
    TEST_ASSERT_FALSE(arquivo_existe(ARQ_DUMP));
}

void test_processa_geo_deve_falhar_para_cep_duplicado(void) {
    sobrescrever_geo(
        "q D1 0 0 10 10\n"
        "q D1 5 5 20 20\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);

    processamento = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NULL(processamento);
    TEST_ASSERT_FALSE(arquivo_existe(ARQ_SVG));
    TEST_ASSERT_FALSE(arquivo_existe(ARQ_DUMP));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_processa_geo_deve_gerar_svg_inicial_e_permitir_busca_por_cep);
    RUN_TEST(test_processa_geo_deve_aceitar_cep_com_ponto);
    RUN_TEST(test_processa_geo_deve_ignorar_linhas_vazias_e_comentarios);
    RUN_TEST(test_processa_geo_deve_falhar_para_comando_invalido_sem_deixar_saida_valida);
    RUN_TEST(test_processa_geo_deve_falhar_para_cep_duplicado);

    return UNITY_END();
}
