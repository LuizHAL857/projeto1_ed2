#include "../unity/unity.h"
#include "../include/habitante.h"
#include "../include/leitor_arquivos.h"
#include "../include/quadra.h"
#include "../include/trata_geo.h"
#include "../include/trata_pm.h"
#include "../include/trata_qry.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static const char *ARQ_GEO = "trata_qry_entrada.geo";
static const char *ARQ_PM = "trata_qry_pessoas.pm";
static const char *ARQ_QRY = "trata_qry_consultas.qry";
static const char *DIR_SAIDA = "trata_qry_saida";
static const char *ARQ_SVG_INICIAL = "trata_qry_saida/trata_qry_entrada.svg";
static const char *ARQ_SVG_FINAL = "trata_qry_saida/trata_qry_entrada-trata_qry_consultas.svg";
static const char *ARQ_TXT_FINAL = "trata_qry_saida/trata_qry_entrada-trata_qry_consultas.txt";
static const char *ARQ_GEO_HASH = "trata_qry_saida/trata_qry_entrada-quadras.hf";
static const char *ARQ_GEO_CONTROLE = "trata_qry_saida/trata_qry_entrada-quadras.hfc";
static const char *ARQ_GEO_DUMP = "trata_qry_saida/trata_qry_entrada-quadras.hfd";
static const char *ARQ_PM_HASH = "trata_qry_saida/trata_qry_pessoas-habitantes.hf";
static const char *ARQ_PM_CONTROLE = "trata_qry_saida/trata_qry_pessoas-habitantes.hfc";
static const char *ARQ_PM_DUMP = "trata_qry_saida/trata_qry_pessoas-habitantes.hfd";

static TrataGeo processamento_geo;
static TrataPm processamento_pm;
static TrataQry processamento_qry;
static DadosDoArquivo dados_geo;
static DadosDoArquivo dados_pm;
static DadosDoArquivo dados_qry;

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
    processamento_qry = NULL;
    dados_geo = NULL;
    dados_pm = NULL;
    dados_qry = NULL;

    remove(ARQ_SVG_INICIAL);
    remove(ARQ_SVG_FINAL);
    remove(ARQ_TXT_FINAL);
    remove(ARQ_GEO_HASH);
    remove(ARQ_GEO_CONTROLE);
    remove(ARQ_GEO_DUMP);
    remove(ARQ_PM_HASH);
    remove(ARQ_PM_CONTROLE);
    remove(ARQ_PM_DUMP);
    remove(ARQ_GEO);
    remove(ARQ_PM);
    remove(ARQ_QRY);
    rmdir(DIR_SAIDA);

    TEST_ASSERT_TRUE(mkdir(DIR_SAIDA, 0777) == 0);
}

void tearDown(void) {
    trata_qry_destruir(processamento_qry);
    processamento_qry = NULL;
    trata_pm_destruir(processamento_pm);
    processamento_pm = NULL;
    trata_geo_destruir(processamento_geo);
    processamento_geo = NULL;
    destruir_dados_arquivo(dados_qry);
    dados_qry = NULL;
    destruir_dados_arquivo(dados_pm);
    dados_pm = NULL;
    destruir_dados_arquivo(dados_geo);
    dados_geo = NULL;

    remove(ARQ_SVG_INICIAL);
    remove(ARQ_SVG_FINAL);
    remove(ARQ_TXT_FINAL);
    remove(ARQ_GEO_HASH);
    remove(ARQ_GEO_CONTROLE);
    remove(ARQ_GEO_DUMP);
    remove(ARQ_PM_HASH);
    remove(ARQ_PM_CONTROLE);
    remove(ARQ_PM_DUMP);
    remove(ARQ_GEO);
    remove(ARQ_PM);
    remove(ARQ_QRY);
    rmdir(DIR_SAIDA);
}

void test_processa_qry_deve_executar_rq_pq_e_censo(void) {
    Habitante habitante;
    Quadra quadra;
    char *svg;
    char *txt;

    sobrescrever_arquivo(
        ARQ_GEO,
        "cq 1px beige brown\n"
        "q b01.1 100 100 40 20\n"
        "q b01.2 180 100 40 20\n");
    sobrescrever_arquivo(
        ARQ_PM,
        "p cpf-001 Ana Silva F 10/01/1999\n"
        "p cpf-002 Beto Souza M 11/02/2000\n"
        "p cpf-003 Caio Lima M 12/03/2001\n"
        "m cpf-001 b01.1 N 10 apto1\n"
        "m cpf-002 b01.1 S 11 casa\n"
        "m cpf-003 b01.2 O 7 fundos\n");
    sobrescrever_arquivo(
        ARQ_QRY,
        "pq b01.1\n"
        "rq b01.1\n"
        "censo\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);
    processamento_geo = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_geo);

    dados_pm = criar_dados_arquivo((char *)ARQ_PM);
    TEST_ASSERT_NOT_NULL(dados_pm);
    processamento_pm = processa_pm(dados_pm, processamento_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_pm);

    dados_qry = criar_dados_arquivo((char *)ARQ_QRY);
    TEST_ASSERT_NOT_NULL(dados_qry);
    processamento_qry = processa_qry(dados_qry, processamento_geo, processamento_pm,
                                     DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_qry);
    TEST_ASSERT_EQUAL_STRING("trata_qry_consultas",
                             trata_qry_obter_nome_qry(processamento_qry));

    TEST_ASSERT_TRUE(arquivo_existe(ARQ_SVG_FINAL));
    TEST_ASSERT_TRUE(arquivo_existe(ARQ_TXT_FINAL));
    TEST_ASSERT_FALSE(arquivo_existe(ARQ_GEO_DUMP));
    TEST_ASSERT_FALSE(arquivo_existe(ARQ_PM_DUMP));

    quadra = trata_geo_obter_quadra(processamento_geo, "b01.1");
    TEST_ASSERT_NULL(quadra);

    quadra = trata_geo_obter_quadra(processamento_geo, "b01.2");
    TEST_ASSERT_NOT_NULL(quadra);
    quadra_destruir(quadra);

    habitante = trata_pm_obter_habitante(processamento_pm, "cpf-001");
    TEST_ASSERT_NOT_NULL(habitante);
    TEST_ASSERT_FALSE(habitante_eh_morador(habitante));
    habitante_destruir(habitante);

    habitante = trata_pm_obter_habitante(processamento_pm, "cpf-002");
    TEST_ASSERT_NOT_NULL(habitante);
    TEST_ASSERT_FALSE(habitante_eh_morador(habitante));
    habitante_destruir(habitante);

    habitante = trata_pm_obter_habitante(processamento_pm, "cpf-003");
    TEST_ASSERT_NOT_NULL(habitante);
    TEST_ASSERT_TRUE(habitante_eh_morador(habitante));
    TEST_ASSERT_EQUAL_STRING("b01.2", habitante_obter_cep(habitante));
    habitante_destruir(habitante);

    svg = ler_arquivo_texto(ARQ_SVG_FINAL);
    TEST_ASSERT_NOT_NULL(svg);
    TEST_ASSERT_NOT_NULL(strstr(svg, "<svg"));
    TEST_ASSERT_NOT_NULL(strstr(svg, "b01.2"));
    TEST_ASSERT_NULL(strstr(svg, ">b01.1</text>"));
    TEST_ASSERT_NOT_NULL(strstr(svg, "stroke=\"red\""));
    TEST_ASSERT_NOT_NULL(strstr(svg, ">1</text>"));
    free(svg);

    txt = ler_arquivo_texto(ARQ_TXT_FINAL);
    TEST_ASSERT_NOT_NULL(txt);
    TEST_ASSERT_NOT_NULL(strstr(txt, "[*] pq b01.1"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "[*] rq b01.1"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "cpf-001 Ana Silva"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "cpf-002 Beto Souza"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "[*] censo"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "habitantes: 3"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "moradores: 1"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "sem-tetos: 2"));
    free(txt);

    trata_qry_destruir(processamento_qry);
    processamento_qry = NULL;
    trata_pm_destruir(processamento_pm);
    processamento_pm = NULL;
    trata_geo_destruir(processamento_geo);
    processamento_geo = NULL;

    TEST_ASSERT_TRUE(arquivo_existe(ARQ_GEO_DUMP));
    TEST_ASSERT_TRUE(arquivo_existe(ARQ_PM_DUMP));

    txt = ler_arquivo_texto(ARQ_GEO_DUMP);
    TEST_ASSERT_NOT_NULL(txt);
    TEST_ASSERT_NULL(strstr(txt, "| b01.1 |"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "| b01.2 |"));
    free(txt);

    txt = ler_arquivo_texto(ARQ_PM_DUMP);
    TEST_ASSERT_NOT_NULL(txt);
    TEST_ASSERT_NOT_NULL(strstr(txt, "| cpf-001 |"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "| cpf-002 |"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "| cpf-003 |"));
    free(txt);
}

void test_processa_qry_deve_ignorar_linhas_vazias_e_comentarios(void) {
    char *txt;

    sobrescrever_arquivo(
        ARQ_GEO,
        "q b01.1 100 100 40 20\n");
    sobrescrever_arquivo(
        ARQ_PM,
        "p cpf-001 Ana Silva F 10/01/1999\n");
    sobrescrever_arquivo(
        ARQ_QRY,
        "\n"
        "# comentario\n"
        "   \n"
        "censo\n"
        "   # outro comentario\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);
    processamento_geo = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_geo);

    dados_pm = criar_dados_arquivo((char *)ARQ_PM);
    TEST_ASSERT_NOT_NULL(dados_pm);
    processamento_pm = processa_pm(dados_pm, processamento_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_pm);

    dados_qry = criar_dados_arquivo((char *)ARQ_QRY);
    TEST_ASSERT_NOT_NULL(dados_qry);
    processamento_qry = processa_qry(dados_qry, processamento_geo, processamento_pm,
                                     DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_qry);

    txt = ler_arquivo_texto(ARQ_TXT_FINAL);
    TEST_ASSERT_NOT_NULL(txt);
    TEST_ASSERT_NOT_NULL(strstr(txt, "[*] censo"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "habitantes: 1"));
    free(txt);
}

void test_processa_qry_deve_posicionar_faces_conforme_orientacao_visual_do_svg(void) {
    char *svg;

    sobrescrever_arquivo(
        ARQ_GEO,
        "q b01.1 100 100 40 20\n");
    sobrescrever_arquivo(
        ARQ_PM,
        "p cpf-001 Ana Silva F 10/01/1999\n"
        "p cpf-002 Beto Souza M 11/02/2000\n"
        "m cpf-001 b01.1 N 10 apto1\n"
        "m cpf-002 b01.1 S 11 casa\n");
    sobrescrever_arquivo(
        ARQ_QRY,
        "pq b01.1\n"
        "dspj cpf-001\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);
    processamento_geo = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_geo);

    dados_pm = criar_dados_arquivo((char *)ARQ_PM);
    TEST_ASSERT_NOT_NULL(dados_pm);
    processamento_pm = processa_pm(dados_pm, processamento_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_pm);

    dados_qry = criar_dados_arquivo((char *)ARQ_QRY);
    TEST_ASSERT_NOT_NULL(dados_qry);
    processamento_qry = processa_qry(dados_qry, processamento_geo, processamento_pm,
                                     DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_qry);

    svg = ler_arquivo_texto(ARQ_SVG_FINAL);
    TEST_ASSERT_NOT_NULL(svg);
    TEST_ASSERT_NOT_NULL(strstr(svg, "<circle cx=\"70.00\" cy=\"100.00\""));
    TEST_ASSERT_NULL(strstr(svg, "<circle cx=\"70.00\" cy=\"80.00\""));
    TEST_ASSERT_NOT_NULL(strstr(svg, "<text x=\"80.00\" y=\"112.00\""));
    TEST_ASSERT_NOT_NULL(strstr(svg, "<text x=\"80.00\" y=\"77.00\""));
    free(svg);
}

void test_processa_qry_deve_executar_h_nasc_rip_mud_e_dspj(void) {
    Habitante habitante;
    char *svg;
    char *txt;

    sobrescrever_arquivo(
        ARQ_GEO,
        "cq 1px beige brown\n"
        "q b01.1 100 100 40 20\n"
        "q b01.2 180 100 40 20\n");
    sobrescrever_arquivo(
        ARQ_PM,
        "p cpf-001 Ana Silva F 10/01/1999\n"
        "p cpf-002 Beto Souza M 11/02/2000\n"
        "p cpf-004 Dora Melo F 13/04/2002\n"
        "m cpf-001 b01.1 N 10 apto1\n"
        "m cpf-004 b01.1 O 8 casa\n");
    sobrescrever_arquivo(
        ARQ_QRY,
        "h? cpf-001\n"
        "nasc cpf-003 Caio Lima M 12/03/2001\n"
        "mud cpf-001 b01.2 S 5 fundos\n"
        "dspj cpf-001\n"
        "rip cpf-004\n"
        "h? cpf-003\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);
    processamento_geo = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_geo);

    dados_pm = criar_dados_arquivo((char *)ARQ_PM);
    TEST_ASSERT_NOT_NULL(dados_pm);
    processamento_pm = processa_pm(dados_pm, processamento_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_pm);

    dados_qry = criar_dados_arquivo((char *)ARQ_QRY);
    TEST_ASSERT_NOT_NULL(dados_qry);
    processamento_qry = processa_qry(dados_qry, processamento_geo, processamento_pm,
                                     DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_qry);

    habitante = trata_pm_obter_habitante(processamento_pm, "cpf-001");
    TEST_ASSERT_NOT_NULL(habitante);
    TEST_ASSERT_FALSE(habitante_eh_morador(habitante));
    habitante_destruir(habitante);

    habitante = trata_pm_obter_habitante(processamento_pm, "cpf-003");
    TEST_ASSERT_NOT_NULL(habitante);
    TEST_ASSERT_EQUAL_STRING("Caio", habitante_obter_nome(habitante));
    TEST_ASSERT_FALSE(habitante_eh_morador(habitante));
    habitante_destruir(habitante);

    habitante = trata_pm_obter_habitante(processamento_pm, "cpf-004");
    TEST_ASSERT_NULL(habitante);

    txt = ler_arquivo_texto(ARQ_TXT_FINAL);
    TEST_ASSERT_NOT_NULL(txt);
    TEST_ASSERT_NOT_NULL(strstr(txt, "[*] h? cpf-001"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "cpf: cpf-001"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "nome: Ana Silva"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "endereco: b01.1/N/10 apto1"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "[*] nasc cpf-003 Caio Lima M 12/03/2001"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "[*] mud cpf-001 b01.2 S 5 fundos"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "[*] dspj cpf-001"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "endereco do despejo: b01.2/S/5 fundos"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "[*] rip cpf-004"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "endereco do falecimento: b01.1/O/8 casa"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "[*] h? cpf-003"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "situacao: sem-teto"));
    TEST_ASSERT_NULL(strstr(txt, "habitante inserido"));
    TEST_ASSERT_NULL(strstr(txt, "novo endereco: b01.2/S/5 fundos"));
    free(txt);

    svg = ler_arquivo_texto(ARQ_SVG_FINAL);
    TEST_ASSERT_NOT_NULL(svg);
    TEST_ASSERT_NOT_NULL(strstr(svg, "<rect"));
    TEST_ASSERT_NOT_NULL(strstr(svg, "cpf-001"));
    TEST_ASSERT_NOT_NULL(strstr(svg, "<circle"));
    TEST_ASSERT_NOT_NULL(strstr(svg, "fill=\"black\""));
    TEST_ASSERT_NOT_NULL(strstr(svg, "stroke=\"red\""));
    free(svg);
}

void test_processa_qry_deve_aceitar_dspj_repetido_sem_abortar_processamento(void) {
    Habitante habitante;
    char *svg;
    char *txt;

    sobrescrever_arquivo(
        ARQ_GEO,
        "q b01.1 100 100 40 20\n");
    sobrescrever_arquivo(
        ARQ_PM,
        "p cpf-001 Ana Silva F 10/01/1999\n"
        "m cpf-001 b01.1 N 10 apto1\n");
    sobrescrever_arquivo(
        ARQ_QRY,
        "dspj cpf-001\n"
        "dspj cpf-001\n"
        "h? cpf-001\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);
    processamento_geo = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_geo);

    dados_pm = criar_dados_arquivo((char *)ARQ_PM);
    TEST_ASSERT_NOT_NULL(dados_pm);
    processamento_pm = processa_pm(dados_pm, processamento_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_pm);

    dados_qry = criar_dados_arquivo((char *)ARQ_QRY);
    TEST_ASSERT_NOT_NULL(dados_qry);
    processamento_qry = processa_qry(dados_qry, processamento_geo, processamento_pm,
                                     DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_qry);

    habitante = trata_pm_obter_habitante(processamento_pm, "cpf-001");
    TEST_ASSERT_NOT_NULL(habitante);
    TEST_ASSERT_FALSE(habitante_eh_morador(habitante));
    habitante_destruir(habitante);

    txt = ler_arquivo_texto(ARQ_TXT_FINAL);
    TEST_ASSERT_NOT_NULL(txt);
    TEST_ASSERT_NOT_NULL(strstr(txt, "[*] dspj cpf-001"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "[*] h? cpf-001"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "situacao: sem-teto"));
    free(txt);

    svg = ler_arquivo_texto(ARQ_SVG_FINAL);
    TEST_ASSERT_NOT_NULL(svg);
    TEST_ASSERT_NOT_NULL(strstr(svg, "<circle"));
    free(svg);
}

void test_processa_qry_deve_aceitar_rip_repetido_sem_abortar_processamento(void) {
    Habitante habitante;
    char *svg;
    char *txt;

    sobrescrever_arquivo(
        ARQ_GEO,
        "q b01.1 100 100 40 20\n");
    sobrescrever_arquivo(
        ARQ_PM,
        "p cpf-001 Ana Silva F 10/01/1999\n"
        "m cpf-001 b01.1 N 10 apto1\n");
    sobrescrever_arquivo(
        ARQ_QRY,
        "rip cpf-001\n"
        "rip cpf-001\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);
    processamento_geo = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_geo);

    dados_pm = criar_dados_arquivo((char *)ARQ_PM);
    TEST_ASSERT_NOT_NULL(dados_pm);
    processamento_pm = processa_pm(dados_pm, processamento_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_pm);

    dados_qry = criar_dados_arquivo((char *)ARQ_QRY);
    TEST_ASSERT_NOT_NULL(dados_qry);
    processamento_qry = processa_qry(dados_qry, processamento_geo, processamento_pm,
                                     DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_qry);

    habitante = trata_pm_obter_habitante(processamento_pm, "cpf-001");
    TEST_ASSERT_NULL(habitante);

    txt = ler_arquivo_texto(ARQ_TXT_FINAL);
    TEST_ASSERT_NOT_NULL(txt);
    TEST_ASSERT_NOT_NULL(strstr(txt, "[*] rip cpf-001"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "endereco do falecimento: b01.1/N/10 apto1"));
    TEST_ASSERT_NOT_NULL(strstr(txt, "habitante inexistente"));
    free(txt);

    svg = ler_arquivo_texto(ARQ_SVG_FINAL);
    TEST_ASSERT_NOT_NULL(svg);
    TEST_ASSERT_NOT_NULL(strstr(svg, "stroke=\"red\""));
    free(svg);
}

void test_processa_qry_deve_falhar_para_comando_invalido_sem_deixar_saidas(void) {
    sobrescrever_arquivo(
        ARQ_GEO,
        "q b01.1 100 100 40 20\n");
    sobrescrever_arquivo(
        ARQ_PM,
        "p cpf-001 Ana Silva F 10/01/1999\n");
    sobrescrever_arquivo(
        ARQ_QRY,
        "zz comando invalido\n");

    dados_geo = criar_dados_arquivo((char *)ARQ_GEO);
    TEST_ASSERT_NOT_NULL(dados_geo);
    processamento_geo = processa_geo(dados_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_geo);

    dados_pm = criar_dados_arquivo((char *)ARQ_PM);
    TEST_ASSERT_NOT_NULL(dados_pm);
    processamento_pm = processa_pm(dados_pm, processamento_geo, DIR_SAIDA);
    TEST_ASSERT_NOT_NULL(processamento_pm);

    dados_qry = criar_dados_arquivo((char *)ARQ_QRY);
    TEST_ASSERT_NOT_NULL(dados_qry);
    processamento_qry = processa_qry(dados_qry, processamento_geo, processamento_pm,
                                     DIR_SAIDA);
    TEST_ASSERT_NULL(processamento_qry);
    TEST_ASSERT_FALSE(arquivo_existe(ARQ_SVG_FINAL));
    TEST_ASSERT_FALSE(arquivo_existe(ARQ_TXT_FINAL));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_processa_qry_deve_executar_rq_pq_e_censo);
    RUN_TEST(test_processa_qry_deve_ignorar_linhas_vazias_e_comentarios);
    RUN_TEST(test_processa_qry_deve_posicionar_faces_conforme_orientacao_visual_do_svg);
    RUN_TEST(test_processa_qry_deve_executar_h_nasc_rip_mud_e_dspj);
    RUN_TEST(test_processa_qry_deve_aceitar_dspj_repetido_sem_abortar_processamento);
    RUN_TEST(test_processa_qry_deve_aceitar_rip_repetido_sem_abortar_processamento);
    RUN_TEST(test_processa_qry_deve_falhar_para_comando_invalido_sem_deixar_saidas);

    return UNITY_END();
}
