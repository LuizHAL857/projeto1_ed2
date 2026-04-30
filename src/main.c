#include "../include/leitor_arquivos.h"
#include "../include/trata_argumentos.h"
#include "../include/trata_geo.h"
#include "../include/trata_pm.h"
#include "../include/trata_qry.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    char *caminho_output = obter_valor_opcao(argc, argv, "o");
    char *caminho_geo = obter_valor_opcao(argc, argv, "f");
    char *caminho_pm = obter_valor_opcao(argc, argv, "pm");
    char *caminho_qry = obter_valor_opcao(argc, argv, "q");
    char *caminho_prefixo = obter_valor_opcao(argc, argv, "e");

    char *caminho_completo_geo = NULL;
    char *caminho_completo_pm = NULL;
    char *caminho_completo_qry = NULL;

    DadosDoArquivo arqGeo = NULL;
    DadosDoArquivo arqPm = NULL;
    DadosDoArquivo arqQry = NULL;
    
    TrataGeo cidade = NULL;
    TrataPm cadastro = NULL;
    TrataQry consultas = NULL;

    if (caminho_geo == NULL || caminho_output == NULL) {
        fprintf(stderr, "Erro de argumentos\n");
        return 1;
    }

    if (caminho_prefixo != NULL) {
        caminho_completo_geo = montar_caminho_entrada(caminho_prefixo, caminho_geo);
        caminho_geo = caminho_completo_geo;
    } else {
        caminho_completo_geo = montar_caminho_entrada(NULL, caminho_geo);
        caminho_geo = caminho_completo_geo;
    }

    if (caminho_geo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel montar o caminho do arquivo .geo\n");
        return 1;
    }

    arqGeo = criar_dados_arquivo(caminho_geo);
    if (arqGeo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo .geo: %s\n", caminho_geo);
        free(caminho_completo_geo);
        return 1;
    }

    cidade = processa_geo(arqGeo, caminho_output);
    if (cidade == NULL) {
        fprintf(stderr, "Erro: falha ao processar o arquivo .geo\n");
        destruir_dados_arquivo(arqGeo);
        free(caminho_completo_geo);
        return 1;
    }

    if (caminho_pm != NULL) {
        caminho_completo_pm = montar_caminho_entrada(caminho_prefixo, caminho_pm);
        if (caminho_completo_pm == NULL) {
            fprintf(stderr, "Erro: nao foi possivel montar o caminho do arquivo .pm\n");
            destruir_dados_arquivo(arqGeo);
            trata_geo_destruir(cidade);
            free(caminho_completo_geo);
            return 1;
        }

        arqPm = criar_dados_arquivo(caminho_completo_pm);
        if (arqPm == NULL) {
            fprintf(stderr, "Erro: nao foi possivel abrir o arquivo .pm: %s\n",
                    caminho_completo_pm);
            destruir_dados_arquivo(arqGeo);
            trata_geo_destruir(cidade);
            free(caminho_completo_pm);
            free(caminho_completo_geo);
            return 1;
        }

        cadastro = processa_pm(arqPm, cidade, caminho_output);
        if (cadastro == NULL) {
            fprintf(stderr, "Erro: falha ao processar o arquivo .pm\n");
            destruir_dados_arquivo(arqPm);
            destruir_dados_arquivo(arqGeo);
            trata_geo_destruir(cidade);
            free(caminho_completo_pm);
            free(caminho_completo_geo);
            return 1;
        }
    }

    if (caminho_qry != NULL) {
        caminho_completo_qry = montar_caminho_entrada(caminho_prefixo, caminho_qry);
        if (caminho_completo_qry == NULL) {
            fprintf(stderr, "Erro: nao foi possivel montar o caminho do arquivo .qry\n");
            trata_pm_destruir(cadastro);
            destruir_dados_arquivo(arqPm);
            destruir_dados_arquivo(arqGeo);
            trata_geo_destruir(cidade);
            free(caminho_completo_pm);
            free(caminho_completo_geo);
            return 1;
        }

        arqQry = criar_dados_arquivo(caminho_completo_qry);
        if (arqQry == NULL) {
            fprintf(stderr, "Erro: nao foi possivel abrir o arquivo .qry: %s\n",
                    caminho_completo_qry);
            trata_pm_destruir(cadastro);
            destruir_dados_arquivo(arqPm);
            destruir_dados_arquivo(arqGeo);
            trata_geo_destruir(cidade);
            free(caminho_completo_qry);
            free(caminho_completo_pm);
            free(caminho_completo_geo);
            return 1;
        }

        consultas = processa_qry(arqQry, cidade, cadastro, caminho_output);
        if (consultas == NULL) {
            fprintf(stderr, "Erro: falha ao processar o arquivo .qry\n");
            destruir_dados_arquivo(arqQry);
            trata_pm_destruir(cadastro);
            destruir_dados_arquivo(arqPm);
            destruir_dados_arquivo(arqGeo);
            trata_geo_destruir(cidade);
            free(caminho_completo_qry);
            free(caminho_completo_pm);
            free(caminho_completo_geo);
            return 1;
        }
    }

    trata_qry_destruir(consultas);
    destruir_dados_arquivo(arqQry);
    trata_pm_destruir(cadastro);
    destruir_dados_arquivo(arqPm);
    destruir_dados_arquivo(arqGeo);
    trata_geo_destruir(cidade);
    free(caminho_completo_qry);
    free(caminho_completo_pm);
    free(caminho_completo_geo);

    return 0;
}
