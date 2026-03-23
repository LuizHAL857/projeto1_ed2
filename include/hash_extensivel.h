#ifndef HASH_EXTENSIVEL_H
#define HASH_EXTENSIVEL_H

#include <stddef.h>
#include <stdbool.h>

/*
 * Tipo opaco do módulo.
 */
typedef void *HashExtensivel;

/*
 * Cria um novo hash file extensível em um arquivo binário.
 *
 * Se o arquivo já existir, seu conteúdo anterior pode ser sobrescrito.
 *
 * Parâmetros:
 * - caminho_arquivo: caminho do arquivo 
 * - capacidade_bucket: quantidade máxima de registros por bucket
 *
 * Retorno:
 * - handle válido em caso de sucesso
 * - NULL em caso de erro
 */
HashExtensivel he_criar(const char *caminho_arquivo, size_t capacidade_bucket);

/*
 * Abre um hash file extensível já existente.
 *
 * Parâmetros:
 * - caminho_arquivo: caminho do arquivo binário
 *
 * Retorno:
 * - handle válido em caso de sucesso
 * - NULL em caso de erro
 */
HashExtensivel he_abrir(const char *caminho_arquivo);

/*
 * Fecha o arquivo e libera todos os recursos da estrutura.
 * Aceita NULL.
 */
void he_fechar(HashExtensivel he);

/*
 * Insere ou atualiza uma chave.
 *
 * Se a chave já existir, apenas atualiza o valor.
 *
 * Retorno:
 * - true em caso de sucesso
 * - false em caso de erro
 */
bool he_inserir(HashExtensivel he, int chave, int valor);

/*
 * Busca o valor associado a uma chave.
 *
 * Retorno:
 * - true se encontrou
 * - false caso contrário
 */
bool he_buscar(HashExtensivel he, int chave, int *valor_out);

/*
 * Verifica se a chave existe.
 */
bool he_contem(HashExtensivel he, int chave);

/*
 * Remove uma chave.
 *
 * Não realiza merge de buckets.
 *
 * Retorno:
 * - true se removeu
 * - false se a chave não existir ou em caso de erro
 */
bool he_remover(HashExtensivel he, int chave);

/*
 * Retorna a quantidade total de registros.
 */
size_t he_tamanho(HashExtensivel he);

/*
 * Retorna a profundidade global.
 */
size_t he_profundidade_global(HashExtensivel he);

/*
 * Retorna o tamanho atual do diretório.
 */
size_t he_tamanho_diretorio(HashExtensivel he);

#endif