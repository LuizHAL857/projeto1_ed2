#include "../unity/unity.h"
#include "../include/habitante.h"
#include "../include/leitor_arquivos.h"
#include "../include/trata_geo.h"
#include "../include/trata_pm.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static const char *ARQ_GEO = "trata_pm_entrada.geo";
static const char *ARQ_PM = "trata_pm_entrada.pm";
static const char *DIR_SAIDA = "trata_pm_saida";
static const char *ARQ_GEO_SVG = "trata_pm_saida/trata_pm_entrada.svg";
static const char *ARQ_GEO_HASH = "trata_pm_saida/trata_pm_entrada-quadras.hf";
static const char *ARQ_GEO_CONTROLE = "trata_pm_saida/trata_pm_entrada-quadras.hfc";
static const char *ARQ_GEO_DUMP = "trata_pm_saida/trata_pm_entrada-quadras.hfd";
static const char *ARQ_PM_HAB_HASH = "trata_pm_saida/trata_pm_entrada-habitantes.hf";
static const char *ARQ_PM_HAB_CONTROLE = "trata_pm_saida/trata_pm_entrada-habitantes.hfc";
static const char *ARQ_PM_HAB_DUMP = "trata_pm_saida/trata_pm_entrada-habitantes.hfd";

static TrataGeo processamento_geo;
static TrataPm processamento_pm;
static DadosDoArquivo dados_geo;
static DadosDoArquivo dados_pm;

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

static void sobrescrever_arquivo(const char *caminho, const char *conteudo) {
    FILE *arquivo = fopen(caminho, "w");

    TEST_ASSERT_NOT_NULL(arquivo);
    TEST_ASSERT_TRUE(fputs(conteudo, arquivo) >= 0);
    fclose(arquivo);
}

void setUp(void) {
    processamento_geo = NULL;
    processamento_pm = NULL;
    dados_geo = NULL;
    dados_pm = NULL;

    remove(ARQ_GEO_SVG);
    remove(ARQ_GEO_HASH);
    remove(ARQ_GEO_CONTROLE);
    remove(ARQ_GEO_DUMP);
    remove(ARQ_PM_HAB_HASH);
    remove(ARQ_PM_HAB_CONTROLE);
    remove(ARQ_PM_HAB_DUMP);
    remove(ARQ_GEO);
    remove(ARQ_PM);
    rmdir(DIR_SAIDA);

    TEST_ASSERT_TRUE(mkdir(DIR_SAIDA, 0777) == 0);
}

void tearDown(void) {
    trata_pm_destruir(processamento_pm);
    processamento_pm = NULL;
    trata_geo_destruir(processamento_geo);
    processamento_geo = NULL;
    destruir_dados_arquivo(dados_pm);
    dados_pm = NULL;
    destruir_dados_arquivo(dados_geo);
    dados_geo = NULL;

    remove(ARQ_GEO_SVG);
    remove(ARQ_GEO_HASH);
    remove(ARQ_GEO_CONTROLE);
    remove(ARQ_GEO_DUMP);
    remove(ARQ_PM_HAB_HASH);
    remove(ARQ_PM_HAB_CONTROLE);
    remove(ARQ_PM_HAB_DUMP);
    remove(ARQ_GEO);
    remove(ARQ_PM);
    rmdir(DIR_SAIDA);
}

void test_processa_pm_deve_persistir_habitantes_e_moradias(void) {
    Habitante habitante;
    char *dump_habitantes;

    sobrescrever_arquivo(
        ARQ_GEO,
        "cq 1px beige brown\n"
        "q b01.1 95 95 120 80\n"
        "q b01.2 230 95 120 80\n");
    sobrescrever_arquivo(
        ARQ_PM,
        "p cpf-001 Ana Silva F 10/01/1999\n"
        "p cpf-002 Beto Souza M 11/02/2000\n"
        "m cpf-001 b01.1 N 12 apto1\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);
    processamento_geo = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_geo);

    dados_pm = criar_dados_arquivo((char *)ARQ_PM);
    TEST_ASSERT_NOT_NULL(dados_pm);
    processamento_pm = processa_pm(dados_pm, processamento_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_pm);
    TEST_ASSERT_EQUAL_STRING("trata_pm_entrada", trata_pm_obter_nome_pm(processamento_pm));

    TEST_ASSERT_TRUE(arquivo_existe(ARQ_PM_HAB_HASH));
    TEST_ASSERT_TRUE(arquivo_existe(ARQ_PM_HAB_CONTROLE));
    TEST_ASSERT_FALSE(arquivo_existe(ARQ_PM_HAB_DUMP));

    habitante = trata_pm_obter_habitante(processamento_pm, "cpf-001");
    TEST_ASSERT_NOT_NULL(habitante);
    TEST_ASSERT_EQUAL_STRING("Ana", habitante_obter_nome(habitante));
    TEST_ASSERT_EQUAL_STRING("Silva", habitante_obter_sobrenome(habitante));
    TEST_ASSERT_EQUAL_CHAR('F', habitante_obter_sexo(habitante));
    TEST_ASSERT_TRUE(habitante_eh_morador(habitante));
    TEST_ASSERT_EQUAL_STRING("b01.1", habitante_obter_cep(habitante));
    TEST_ASSERT_EQUAL_CHAR('N', habitante_obter_face(habitante));
    TEST_ASSERT_EQUAL_INT(12, habitante_obter_num(habitante));
    TEST_ASSERT_EQUAL_STRING("apto1", habitante_obter_compl(habitante));
    habitante_destruir(habitante);

    habitante = trata_pm_obter_habitante(processamento_pm, "cpf-002");
    TEST_ASSERT_NOT_NULL(habitante);
    TEST_ASSERT_FALSE(habitante_eh_morador(habitante));
    habitante_destruir(habitante);

    trata_pm_destruir(processamento_pm);
    processamento_pm = NULL;
    TEST_ASSERT_TRUE(arquivo_existe(ARQ_PM_HAB_DUMP));

    dump_habitantes = ler_arquivo_texto(ARQ_PM_HAB_DUMP);
    TEST_ASSERT_NOT_NULL(dump_habitantes);
    TEST_ASSERT_NOT_NULL(strstr(dump_habitantes, "DUMP"));
    TEST_ASSERT_NOT_NULL(strstr(dump_habitantes, "*Dump buckets"));
    TEST_ASSERT_NOT_NULL(strstr(dump_habitantes, "| cpf-001 |"));
    TEST_ASSERT_NOT_NULL(strstr(dump_habitantes, "| cpf-002 |"));
    free(dump_habitantes);
}

void test_processa_pm_deve_ignorar_linhas_vazias_e_comentarios(void) {
    Habitante habitante;

    sobrescrever_arquivo(
        ARQ_GEO,
        "q b01.1 95 95 120 80\n");
    sobrescrever_arquivo(
        ARQ_PM,
        "\n"
        "# comentario\n"
        "   \n"
        "p cpf-003 Carla Lima F 12/03/2001\n"
        "   # outro comentario\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);
    processamento_geo = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_geo);

    dados_pm = criar_dados_arquivo((char *)ARQ_PM);
    TEST_ASSERT_NOT_NULL(dados_pm);
    processamento_pm = processa_pm(dados_pm, processamento_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_pm);

    habitante = trata_pm_obter_habitante(processamento_pm, "cpf-003");
    TEST_ASSERT_NOT_NULL(habitante);
    TEST_ASSERT_EQUAL_STRING("Carla", habitante_obter_nome(habitante));
    habitante_destruir(habitante);
}

void test_processa_pm_deve_aceitar_cpf_formatado_com_pontos(void) {
    Habitante habitante;

    sobrescrever_arquivo(
        ARQ_GEO,
        "q b01.1 95 95 120 80\n");
    sobrescrever_arquivo(
        ARQ_PM,
        "p 000.000.001-91 Ana Silva F 10/01/1999\n"
        "m 000.000.001-91 b01.1 N 12 apto1\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);
    processamento_geo = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_geo);

    dados_pm = criar_dados_arquivo((char *)ARQ_PM);
    TEST_ASSERT_NOT_NULL(dados_pm);
    processamento_pm = processa_pm(dados_pm, processamento_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_pm);

    habitante = trata_pm_obter_habitante(processamento_pm, "000.000.001-91");
    TEST_ASSERT_NOT_NULL(habitante);
    TEST_ASSERT_TRUE(habitante_eh_morador(habitante));
    TEST_ASSERT_EQUAL_STRING("b01.1", habitante_obter_cep(habitante));
    habitante_destruir(habitante);
}

void test_processa_pm_deve_falhar_quando_moradia_referencia_habitante_inexistente(void) {
    sobrescrever_arquivo(
        ARQ_GEO,
        "q b01.1 95 95 120 80\n");
    sobrescrever_arquivo(
        ARQ_PM,
        "m cpf-404 b01.1 N 10 apto\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);
    processamento_geo = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_geo);

    dados_pm = criar_dados_arquivo((char *)ARQ_PM);
    TEST_ASSERT_NOT_NULL(dados_pm);
    processamento_pm = processa_pm(dados_pm, processamento_geo, DIR_SAIDA);
    TEST_ASSERT_NULL(processamento_pm);
    TEST_ASSERT_FALSE(arquivo_existe(ARQ_PM_HAB_HASH));
}

void test_processa_pm_deve_falhar_quando_moradia_referencia_cep_inexistente(void) {
    sobrescrever_arquivo(
        ARQ_GEO,
        "q b01.1 95 95 120 80\n");
    sobrescrever_arquivo(
        ARQ_PM,
        "p cpf-001 Ana Silva F 10/01/1999\n"
        "m cpf-001 b99.9 N 10 apto\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);
    processamento_geo = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_geo);

    dados_pm = criar_dados_arquivo((char *)ARQ_PM);
    TEST_ASSERT_NOT_NULL(dados_pm);
    processamento_pm = processa_pm(dados_pm, processamento_geo, DIR_SAIDA);
    TEST_ASSERT_NULL(processamento_pm);
    TEST_ASSERT_FALSE(arquivo_existe(ARQ_PM_HAB_HASH));
}

void test_processa_pm_deve_falhar_para_cpf_duplicado(void) {
    sobrescrever_arquivo(
        ARQ_GEO,
        "q b01.1 95 95 120 80\n");
    sobrescrever_arquivo(
        ARQ_PM,
        "p cpf-001 Ana Silva F 10/01/1999\n"
        "p cpf-001 Bia Souza F 11/02/2000\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);
    processamento_geo = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_geo);

    dados_pm = criar_dados_arquivo((char *)ARQ_PM);
    TEST_ASSERT_NOT_NULL(dados_pm);
    processamento_pm = processa_pm(dados_pm, processamento_geo, DIR_SAIDA);
    TEST_ASSERT_NULL(processamento_pm);
    TEST_ASSERT_FALSE(arquivo_existe(ARQ_PM_HAB_HASH));
}

void test_processa_pm_deve_falhar_sem_trata_geo(void) {
    sobrescrever_arquivo(
        ARQ_PM,
        "p cpf-001 Ana Silva F 10/01/1999\n");

    dados_pm = criar_dados_arquivo((char *)ARQ_PM);
    TEST_ASSERT_NOT_NULL(dados_pm);
    processamento_pm = processa_pm(dados_pm, NULL, DIR_SAIDA);
    TEST_ASSERT_NULL(processamento_pm);
    TEST_ASSERT_FALSE(arquivo_existe(ARQ_PM_HAB_HASH));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_processa_pm_deve_persistir_habitantes_e_moradias);
    RUN_TEST(test_processa_pm_deve_ignorar_linhas_vazias_e_comentarios);
    RUN_TEST(test_processa_pm_deve_aceitar_cpf_formatado_com_pontos);
    RUN_TEST(test_processa_pm_deve_falhar_quando_moradia_referencia_habitante_inexistente);
    RUN_TEST(test_processa_pm_deve_falhar_quando_moradia_referencia_cep_inexistente);
    RUN_TEST(test_processa_pm_deve_falhar_para_cpf_duplicado);
    RUN_TEST(test_processa_pm_deve_falhar_sem_trata_geo);

    return UNITY_END();
}
