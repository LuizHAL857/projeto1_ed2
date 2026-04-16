#ifndef HASH_EXTENSIVEL_H
#define HASH_EXTENSIVEL_H

/*
 * Modulo de hash extensivel persistente em arquivo binario.
 *
 * Convencao adotada neste projeto:
 * - o arquivo principal do hash deve usar a extensao `.hf`
 * - o modulo cria automaticamente um arquivo auxiliar `.hfc` para guardar
 *   metadados, diretorio e historico de expansoes dos buckets
 * - o estado legivel da estrutura pode ser exportado em um arquivo `.hfd`
 *
 * O arquivo `.hf` guarda os buckets fisicos e os registros completos.
 * O arquivo `.hfc` guarda:
 * - profundidade global
 * - capacidade dos buckets
 * - tamanho do registro
 * - tamanho atual da estrutura
 * - offsets do diretorio
 * - log de expansoes de buckets
 *
 * A chave textual faz parte do proprio registro. O contrato do modulo exige
 * que a chave esteja no primeiro campo do registro, como uma string terminada
 * em '\0', com no maximo `HE_TAMANHO_CHAVE_MAX` caracteres.
 *
 * Essa convencao segue a propriedade padrao da linguagem C de que o primeiro
 * campo de uma `struct` comeca exatamente no mesmo endereco da estrutura. Por
 * isso o modulo consegue extrair a chave lendo o inicio do bloco do registro.
 *
 * O hash grava em disco exatamente `tamanho_registro` bytes por registro. Para
 * evitar bytes extras de alinhamento entre os campos, o recomendado e usar
 * structs empacotadas (`packed`) para os registros persistidos do projeto.
 */

#include <stdbool.h>
#include <stddef.h>

#define HE_TAMANHO_CHAVE_MAX 63u

#define HE_PACKED __attribute__((packed))

typedef void *HashExtensivel;

/*
 * Cria uma nova hash extensivel vazia.
 *
 * Parametros:
 * - caminho_hf: caminho do arquivo principal do hash, obrigatoriamente com a
 *   extensao `.hf`
 * - capacidade_bucket: quantidade maxima de registros por bucket
 * - tamanho_registro: quantidade exata de bytes de cada registro
 *
 * Efeitos:
 * - cria ou sobrescreve o arquivo `.hf`
 * - cria ou sobrescreve o arquivo auxiliar `.hfc` correspondente
 *
 * Retorno:
 * - uma instancia valida em caso de sucesso
 * - NULL em caso de erro
 */
HashExtensivel he_criar(const char *caminho_hf, size_t capacidade_bucket,
                        size_t tamanho_registro);

/*
 * Abre uma hash extensivel existente.
 *
 * Parametros:
 * - caminho_hf: caminho do arquivo principal `.hf`
 * - capacidade_bucket: deve ser compativel com o valor usado na criacao
 * - tamanho_registro: deve ser compativel com o valor usado na criacao
 *
 * O modulo tenta abrir `caminho_hf` e o arquivo auxiliar `.hfc` correspondente.
 */
HashExtensivel he_abrir(const char *caminho_hf, size_t capacidade_bucket,
                        size_t tamanho_registro);

/*
 * Fecha a hash, persiste os metadados no arquivo de indice e libera memoria.
 */
void he_fechar(HashExtensivel he);

/*
 * Insere um registro completo.
 *
 * A chave e extraida do inicio do proprio bloco do registro.
 *
 * Requisitos do registro:
 * - deve ter exatamente `he_tamanho_registro(he)` bytes
 * - deve comecar com a chave textual
 * - a chave deve ser alfanumerica, podendo conter tambem '-'
 *
 * Retorna `true` em caso de sucesso e `false` em caso de erro ou chave
 * duplicada.
 */
bool he_inserir(HashExtensivel he, const void *registro);

/*
 * Atualiza um registro existente usando a chave presente no proprio registro.
 */
bool he_atualizar(HashExtensivel he, const void *registro);

/*
 * Busca um registro pela chave textual.
 *
 * Se `registro_out` for NULL, a funcao apenas verifica se a chave existe.
 */
bool he_buscar(HashExtensivel he, const char *chave, void *registro_out);

/*
 * Retorna `true` se a chave existir na estrutura.
 */
bool he_contem(HashExtensivel he, const char *chave);

/*
 * Remove o registro correspondente a chave.
 */
bool he_remover(HashExtensivel he, const char *chave);
#ifndef HASH_EXTENSIVEL_H
#define HASH_EXTENSIVEL_H

/*
 * Modulo de hash extensivel persistente em arquivo binario.
 *
 * Convencao adotada neste projeto:
 * - o arquivo principal do hash deve usar a extensao `.hf`
 * - o modulo cria automaticamente um arquivo auxiliar `.hfc` para guardar
 *   metadados, diretorio e historico de expansoes dos buckets
 * - o estado legivel da estrutura pode ser exportado em um arquivo `.hfd`
 *
 * O arquivo `.hf` guarda os buckets fisicos e os registros completos.
 * O arquivo `.hfc` guarda:
 * - profundidade global
 * - capacidade dos buckets
 * - tamanho do registro
 * - tamanho atual da estrutura
 * - offsets do diretorio
 * - log de expansoes de buckets
 *
 * A chave textual faz parte do proprio registro. O contrato do modulo exige
 * que a chave esteja no primeiro campo do registro, como uma string terminada
 * em '\0', com no maximo `HE_TAMANHO_CHAVE_MAX` caracteres.
 *
 * Essa convencao segue a propriedade padrao da linguagem C de que o primeiro
 * campo de uma `struct` comeca exatamente no mesmo endereco da estrutura. Por
 * isso o modulo consegue extrair a chave lendo o inicio do bloco do registro.
 *
 * O hash grava em disco exatamente `tamanho_registro` bytes por registro. Para
 * evitar bytes extras de alinhamento entre os campos, o recomendado e usar
 * structs empacotadas (`packed`) para os registros persistidos do projeto.
 */

#include <stdbool.h>
#include <stddef.h>

#define HE_TAMANHO_CHAVE_MAX 63u

#if defined(__GNUC__) || defined(__clang__)
#define HE_PACKED __attribute__((packed))
#else
#define HE_PACKED
#endif

typedef void *HashExtensivel;

/*
 * Cria uma nova hash extensivel vazia.
 *
 * Parametros:
 * - caminho_hf: caminho do arquivo principal do hash, obrigatoriamente com a
 *   extensao `.hf`
 * - capacidade_bucket: quantidade maxima de registros por bucket
 * - tamanho_registro: quantidade exata de bytes de cada registro
 *
 * Efeitos:
 * - cria ou sobrescreve o arquivo `.hf`
 * - cria ou sobrescreve o arquivo auxiliar `.hfc` correspondente
 *
 * Retorno:
 * - uma instancia valida em caso de sucesso
 * - NULL em caso de erro
 */
HashExtensivel he_criar(const char *caminho_hf, size_t capacidade_bucket,
                        size_t tamanho_registro);

/*
 * Abre uma hash extensivel existente.
 *
 * Parametros:
 * - caminho_hf: caminho do arquivo principal `.hf`
 * - capacidade_bucket: deve ser compativel com o valor usado na criacao
 * - tamanho_registro: deve ser compativel com o valor usado na criacao
 *
 * O modulo tenta abrir `caminho_hf` e o arquivo auxiliar `.hfc` correspondente.
 */
HashExtensivel he_abrir(const char *caminho_hf, size_t capacidade_bucket,
                        size_t tamanho_registro);

/*
 * Fecha a hash, persiste os metadados no arquivo de indice e libera memoria.
 */
void he_fechar(HashExtensivel he);

/*
 * Insere um registro completo.
 *
 * A chave e extraida do inicio do proprio bloco do registro.
 *
 * Requisitos do registro:
 * - deve ter exatamente `he_tamanho_registro(he)` bytes
 * - deve comecar com a chave textual
 * - a chave deve ser alfanumerica, podendo conter tambem '-'
 *
 * Retorna `true` em caso de sucesso e `false` em caso de erro ou chave
 * duplicada.
 */
bool he_inserir(HashExtensivel he, const void *registro);

/*
 * Atualiza um registro existente usando a chave presente no proprio registro.
 */
bool he_atualizar(HashExtensivel he, const void *registro);

/*
 * Busca um registro pela chave textual.
 *
 * Se `registro_out` for NULL, a funcao apenas verifica se a chave existe.
 */
bool he_buscar(HashExtensivel he, const char *chave, void *registro_out);

/*
 * Retorna `true` se a chave existir na estrutura.
 */
bool he_contem(HashExtensivel he, const char *chave);

/*
 * Remove o registro correspondente a chave.
 */
bool he_remover(HashExtensivel he, const char *chave);

/*
 * Informacoes auxiliares, uteis para testes e depuracao.
 */
size_t he_tamanho(HashExtensivel he);
size_t he_profundidade_global(HashExtensivel he);
size_t he_tamanho_diretorio(HashExtensivel he);
size_t he_tamanho_registro(HashExtensivel he);

/*
 * Gera um dump textual legivel da estrutura.
 *
 * O dump deve ser salvo, preferencialmente, com a extensao `.hfd` e inclui:
 * - informacoes gerais da hash
 * - diretorio atual
 * - buckets fisicos
 * - historico de expansoes de buckets registradas pelo modulo
 */
void he_dump(HashExtensivel he, const char *caminho_dump);

#endif

/*
 * Informacoes auxiliares, uteis para testes e depuracao.
 */
size_t he_tamanho(HashExtensivel he);
size_t he_profundidade_global(HashExtensivel he);
size_t he_tamanho_diretorio(HashExtensivel he);
size_t he_tamanho_registro(HashExtensivel he);

/*
 * Gera um dump textual legivel da estrutura.
 *
 * O dump deve ser salvo, preferencialmente, com a extensao `.hfd` e inclui:
 * - informacoes gerais da hash
 * - diretorio atual
 * - buckets fisicos
 * - historico de expansoes de buckets registradas pelo modulo
 */
void he_dump(HashExtensivel he, const char *caminho_dump);

#endif
