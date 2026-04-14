#include "../include/trata_argumentos.h"



#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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