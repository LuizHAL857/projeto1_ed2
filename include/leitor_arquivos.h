#ifndef LEITOR_ARQUIVOS_H
#define LEITOR_ARQUIVOS_H

/*
 * Módulo de leitura de arquivos texto para memória.
 *
 * O módulo encapsula os dados de um arquivo lido do disco, preservando seu
 * caminho, nome e uma lista com as linhas do conteúdo. Ele também centraliza a
 * criação e a destruição dessas estruturas auxiliares.
 */

#include <stdio.h>
#include "../include/lista.h"
/* Tipo opaco para os dados carregados de um arquivo. */
typedef void* DadosDoArquivo;
 
 /**
  * Cria uma nova instância de DadosDoArquivo e lê o conteúdo do arquivo.
  *
  * @param caminhoArquivo Caminho completo para o arquivo.
  * @return Instância de DadosDoArquivo ou NULL em caso de erro.
  */
DadosDoArquivo criar_dados_arquivo(char *caminhoArquivo);
 
 /**
  * Destroi uma instância de DadosDoArquivo e libera toda a memória associada.
  *
  * @param dadosArquivo A instância a ser destruída.
  */
void destruir_dados_arquivo(DadosDoArquivo dadosArquivo);
 
 /**
  * Obtém o caminho completo do arquivo.
  *
  * @param dadosArquivo Instância de DadosDoArquivo.
  * @return Ponteiro para string com o caminho do arquivo.
  */
char *obter_caminho_arquivo(DadosDoArquivo dadosArquivo);
 
 /**
  * Obtém o nome do arquivo (sem o caminho).
  *
  * @param dadosArquivo Instância de DadosDoArquivo.
  * @return Ponteiro para string com o nome do arquivo.
  */
char *obter_nome_arquivo(DadosDoArquivo dadosArquivo);
 
 /**
  * Obtém a fila com as linhas do arquivo.
  *
  * @param dadosArquivo Instância de DadosDoArquivo.
  * @return Lista contendo as linhas do arquivo.
  */
Lista obter_lista_linhas(DadosDoArquivo dadosArquivo);

#endif
