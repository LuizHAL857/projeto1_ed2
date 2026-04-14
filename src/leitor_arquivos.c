#include "leitor_arquivos.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct DadosDoArquivo {
    char *caminhoArquivo;
    char *nomeArquivo;
    Lista listaDeLinhas;
    Lista listaLinhasParaLiberar;
};

struct ListaDeLinhas {
    Lista listaDeLinhas;
    Lista listaLinhasParaLiberar;
};

static char *ler_linha(FILE *arquivo, char *buffer, size_t tamanho);
static struct ListaDeLinhas *ler_arquivo_para_lista(char *caminhoArquivo);
static char *duplicaString(char *s);

DadosDoArquivo criar_dados_arquivo(char *caminhoArquivo) {
    struct DadosDoArquivo *arquivo = malloc(sizeof(struct DadosDoArquivo));

    if (caminhoArquivo == NULL) {
        return NULL;
    }

    if (arquivo == NULL) {
        printf("Erro: Falha ao alocar memoria para DadosDoArquivo\n");
        return NULL;
    }

    arquivo->caminhoArquivo = caminhoArquivo;
    arquivo->nomeArquivo =
        strrchr(caminhoArquivo, '/') ? strrchr(caminhoArquivo, '/') + 1 : caminhoArquivo;

    struct ListaDeLinhas *lista = ler_arquivo_para_lista(caminhoArquivo);

    if (lista == NULL || lista->listaDeLinhas == NULL ||
        lista->listaLinhasParaLiberar == NULL) {
        printf("Erro: Falha ao ler as linhas do arquivo\n");
        free(lista);
        free(arquivo);
        return NULL;
    }

    arquivo->listaDeLinhas = lista->listaDeLinhas;
    arquivo->listaLinhasParaLiberar = lista->listaLinhasParaLiberar;
    free(lista);
    return (DadosDoArquivo)arquivo;
}

static struct ListaDeLinhas *ler_arquivo_para_lista(char *caminhoArquivo) {
    struct ListaDeLinhas *lista = malloc(sizeof(struct ListaDeLinhas));
    if (lista == NULL) {
        printf("Erro: Falha ao alocar memoria para Lista de linhas\n");
        return NULL;
    }

    Lista lista_linhas = criaLista();
    Lista lista_liberar = criaLista();
    FILE *arquivo = fopen(caminhoArquivo, "r");

    if (arquivo == NULL) {
        if (lista_linhas != NULL) {
            liberaLista(lista_linhas);
        }
        if (lista_liberar != NULL) {
            liberaLista(lista_liberar);
        }
        free(lista);
        return NULL;
    }

    char buffer[1024];
    while (ler_linha(arquivo, buffer, sizeof(buffer)) != NULL) {
        char *linha = duplicaString(buffer);

        if (linha == NULL) {
            fclose(arquivo);

            while (!listaVazia(lista_liberar)) {
                free(removeInicioLista(lista_liberar));
            }

            liberaLista(lista_liberar);
            liberaLista(lista_linhas);
            free(lista);
            return NULL;
        }

        insereFinalLista(lista_linhas, linha);
        insereFinalLista(lista_liberar, linha);
    }

    fclose(arquivo);

    lista->listaDeLinhas = lista_linhas;
    lista->listaLinhasParaLiberar = lista_liberar;
    return lista;
}

void destruir_dados_arquivo(DadosDoArquivo dadosArquivo) {
    if (dadosArquivo != NULL) {
        struct DadosDoArquivo *arquivo = (struct DadosDoArquivo *)dadosArquivo;

        while (!listaVazia(arquivo->listaLinhasParaLiberar)) {
            void *linha = removeInicioLista(arquivo->listaLinhasParaLiberar);

            if (linha != NULL) {
                free(linha);
            }
        }

        liberaLista(arquivo->listaLinhasParaLiberar);
        liberaLista(arquivo->listaDeLinhas);
        free(dadosArquivo);
    }
}

char *obter_caminho_arquivo(DadosDoArquivo dadosArquivo) {
    struct DadosDoArquivo *arquivo = (struct DadosDoArquivo *)dadosArquivo;
    if (arquivo == NULL) {
        return NULL;
    }
    return arquivo->caminhoArquivo;
}

char *obter_nome_arquivo(DadosDoArquivo dadosArquivo) {
    struct DadosDoArquivo *arquivo = (struct DadosDoArquivo *)dadosArquivo;
    if (arquivo == NULL) {
        return NULL;
    }
    return arquivo->nomeArquivo;
}

Lista obter_lista_linhas(DadosDoArquivo dadosArquivo) {
    struct DadosDoArquivo *arquivo = (struct DadosDoArquivo *)dadosArquivo;
    if (arquivo == NULL) {
        return NULL;
    }
    return arquivo->listaDeLinhas;
}

static char *ler_linha(FILE *arquivo, char *buffer, size_t tamanho) {
    if (fgets(buffer, tamanho, arquivo) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        return buffer;
    }
    return NULL;
}

static char *duplicaString(char *s) {
    if (s == NULL) {
        return NULL;
    }

    size_t len = strlen(s) + 1;
    char *dup = malloc(len);
    if (dup != NULL) {
        strcpy(dup, s);
    }
    return dup;
}
