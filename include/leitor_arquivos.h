#ifndef LEITOR_ARQUIVOS_H
#define LEITOR_ARQUIVOS_H

/*
 * Módulo de leitura de arquivos texto para memória.
 *
 * O módulo encapsula os dados de um arquivo lido do disco, preservando seu
 * caminho, nome e uma lista com as linhas do conteúdo. Ele também centraliza a
 * criação e a destruição dessas estruturas auxiliares.
 */

#include <stdbool.h>
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

/*
 * Helpers de texto e caminhos compartilhados entre os modulos de leitura e
 * processamento de arquivos.
 */
const char *arquivo_pular_espacos(const char *texto);
bool arquivo_linha_ignorada(const char *linha);
bool arquivo_resto_valido(const char *trecho);
bool arquivo_termina_com(const char *texto, const char *sufixo);
char *arquivo_extrair_nome_base(const char *caminho_ou_nome, const char *extensao);
char *arquivo_montar_nome_composto(const char *base1, const char *separador,
                                   const char *base2);
char *arquivo_montar_caminho_saida(const char *diretorio, const char *nome_base,
                                   const char *sufixo);

#endif
