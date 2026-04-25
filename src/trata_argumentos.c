#include "../include/trata_argumentos.h"



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int caminho_absoluto(const char *caminho) {
  return caminho != NULL && caminho[0] == '/';
}

char *obter_valor_opcao(int argc, char *argv[], char *nome_opcao) {
  char formato_opcao[64];
  snprintf(formato_opcao, sizeof(formato_opcao), "-%s", nome_opcao);

  for (int i = 1; i < argc - 1; ++i) {
    if (strcmp(argv[i], formato_opcao) == 0) {
      if (argv[i + 1] == NULL)
        return NULL;
      return argv[i + 1];
    }
  }
  return NULL;
}


char *obter_sufixo_comando(int argc, char *argv[]) {
 
  char **argv_copia = malloc(argc * sizeof(char *));
  if (argv_copia == NULL)
    return NULL;


  for (int i = 0; i < argc; i++) {
    argv_copia[i] = argv[i];
  }

  int novo_argc = argc;


  for (int i = 1; i < novo_argc - 1; i++) {
    
    if (argv_copia[i][0] == '-') {
    
      if (argv_copia[i + 1][0] != '-') {
        
        for (int j = i; j < novo_argc - 2; j++) {
          argv_copia[j] = argv_copia[j + 2];
        }
        novo_argc -= 2;
        i--; 
      }
    }
  }

 
  char *sufixo = NULL;
  if (novo_argc == 2) {
    sufixo = argv_copia[novo_argc - 1];
  } else if (novo_argc > 2) {
    printf("Erro: Mais de um sufixo encontrado\n");
    free(argv_copia);
    exit(1);
  }

  free(argv_copia);
  return sufixo;
}

char *montar_caminho_entrada(const char *diretorio_base, const char *nome_arquivo) {
  size_t tamanho_base;
  size_t tamanho_arquivo;
  int precisa_barra;
  char *caminho;

  if (nome_arquivo == NULL) {
    return NULL;
  }

  if (caminho_absoluto(nome_arquivo)) {
    caminho = (char *)malloc(strlen(nome_arquivo) + 1u);
    if (caminho != NULL) {
      strcpy(caminho, nome_arquivo);
    }
    return caminho;
  }

  if (diretorio_base == NULL || diretorio_base[0] == '\0') {
    diretorio_base = ".";
  }

  tamanho_base = strlen(diretorio_base);
  tamanho_arquivo = strlen(nome_arquivo);
  precisa_barra = tamanho_base > 0u && diretorio_base[tamanho_base - 1u] != '/';

  caminho = (char *)malloc(tamanho_base + (precisa_barra ? 1u : 0u) +
                           tamanho_arquivo + 1u);
  if (caminho == NULL) {
    return NULL;
  }

  memcpy(caminho, diretorio_base, tamanho_base);
  if (precisa_barra) {
    caminho[tamanho_base] = '/';
    tamanho_base++;
  }
  memcpy(caminho + tamanho_base, nome_arquivo, tamanho_arquivo + 1u);
  return caminho;
}
