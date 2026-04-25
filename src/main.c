#include "../include/leitor_arquivos.h"
#include "../include/trata_argumentos.h"
#include "../include/trata_geo.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char *caminho_output = obter_valor_opcao(argc, argv, "o");
    char *caminho_geo = obter_valor_opcao(argc, argv, "f");
    char *caminho_prefixo = obter_valor_opcao(argc, argv, "e");
    char *caminho_completo_geo = NULL;
    DadosDoArquivo arqGeo = NULL;
    TrataGeo cidade = NULL;

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

    destruir_dados_arquivo(arqGeo);
    trata_geo_destruir(cidade);
    free(caminho_completo_geo);

    return 0;
}
