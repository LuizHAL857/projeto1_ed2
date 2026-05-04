#include "leitor_arquivos.h"

#include <ctype.h>
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
static const char *obter_nome_arquivo_do_caminho(const char *caminho);

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

const char *arquivo_pular_espacos(const char *texto) {
    while (texto != NULL && *texto != '\0' && isspace((unsigned char)*texto) != 0) {
        texto++;
    }

    return texto;
}

bool arquivo_linha_ignorada(const char *linha) {
    const char *cursor = arquivo_pular_espacos(linha);
    return cursor != NULL && (*cursor == '\0' || *cursor == '#');
}

bool arquivo_resto_valido(const char *trecho) {
    trecho = arquivo_pular_espacos(trecho);
    return trecho != NULL && (*trecho == '\0' || *trecho == '#');
}

bool arquivo_termina_com(const char *texto, const char *sufixo) {
    size_t tamanho_texto;
    size_t tamanho_sufixo;

    if (texto == NULL || sufixo == NULL) {
        return false;
    }

    tamanho_texto = strlen(texto);
    tamanho_sufixo = strlen(sufixo);
    if (tamanho_texto < tamanho_sufixo) {
        return false;
    }

    return strcmp(texto + tamanho_texto - tamanho_sufixo, sufixo) == 0;
}

char *arquivo_extrair_nome_base(const char *caminho_ou_nome, const char *extensao) {
    const char *nome_arquivo;
    size_t tamanho_base;
    char *nome_base;

    if (caminho_ou_nome == NULL || extensao == NULL) {
        return NULL;
    }

    nome_arquivo = obter_nome_arquivo_do_caminho(caminho_ou_nome);
    if (!arquivo_termina_com(nome_arquivo, extensao)) {
        return NULL;
    }

    tamanho_base = strlen(nome_arquivo) - strlen(extensao);
    if (tamanho_base == 0u) {
        return NULL;
    }

    nome_base = (char *)calloc(tamanho_base + 1u, sizeof(char));
    if (nome_base != NULL) {
        memcpy(nome_base, nome_arquivo, tamanho_base);
    }

    return nome_base;
}

char *arquivo_montar_nome_composto(const char *base1, const char *separador,
                                   const char *base2) {
    size_t tamanho_base1;
    size_t tamanho_separador;
    size_t tamanho_base2;
    char *nome_composto;

    if (base1 == NULL) {
        return NULL;
    }

    tamanho_base1 = strlen(base1);
    if (base2 == NULL || base2[0] == '\0') {
        nome_composto = (char *)calloc(tamanho_base1 + 1u, sizeof(char));
        if (nome_composto != NULL) {
            memcpy(nome_composto, base1, tamanho_base1);
        }
        return nome_composto;
    }

    if (separador == NULL) {
        return NULL;
    }

    tamanho_separador = strlen(separador);
    tamanho_base2 = strlen(base2);
    nome_composto = (char *)malloc(tamanho_base1 + tamanho_separador + tamanho_base2 + 1u);
    if (nome_composto == NULL) {
        return NULL;
    }

    memcpy(nome_composto, base1, tamanho_base1);
    memcpy(nome_composto + tamanho_base1, separador, tamanho_separador);
    memcpy(nome_composto + tamanho_base1 + tamanho_separador, base2, tamanho_base2 + 1u);
    return nome_composto;
}

char *arquivo_montar_caminho_saida(const char *diretorio, const char *nome_base,
                                   const char *sufixo) {
    size_t tamanho_diretorio;
    size_t tamanho_base;
    size_t tamanho_sufixo;
    int precisa_barra;
    char *caminho;

    if (diretorio == NULL || nome_base == NULL || sufixo == NULL) {
        return NULL;
    }

    tamanho_diretorio = strlen(diretorio);
    tamanho_base = strlen(nome_base);
    tamanho_sufixo = strlen(sufixo);
    precisa_barra = tamanho_diretorio > 0u && diretorio[tamanho_diretorio - 1u] != '/';

    caminho = (char *)malloc(tamanho_diretorio + (precisa_barra ? 1u : 0u) +
                             tamanho_base + tamanho_sufixo + 1u);
    if (caminho == NULL) {
        return NULL;
    }

    memcpy(caminho, diretorio, tamanho_diretorio);
    if (precisa_barra) {
        caminho[tamanho_diretorio] = '/';
        tamanho_diretorio++;
    }
    memcpy(caminho + tamanho_diretorio, nome_base, tamanho_base);
    memcpy(caminho + tamanho_diretorio + tamanho_base, sufixo, tamanho_sufixo + 1u);
    return caminho;
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

static const char *obter_nome_arquivo_do_caminho(const char *caminho) {
    const char *nome_arquivo;

    if (caminho == NULL) {
        return NULL;
    }

    nome_arquivo = strrchr(caminho, '/');
    return nome_arquivo == NULL ? caminho : nome_arquivo + 1;
}
