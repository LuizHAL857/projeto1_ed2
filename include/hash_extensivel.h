#ifndef HASH_EXTENSIVEL_H
#define HASH_EXTENSIVEL_H

/*
 * Modulo de hash extensivel persistente em arquivo binario.
 *
 * O modulo armazena registros completos associados a chaves textuais, mas a
 * chave nao e passada separadamente na insercao. Em vez disso, o hash recebe o
 * layout da representacao persistida do registro e extrai a chave sozinho a
 * partir de `chave_offset` e `chave_tamanho`.
 *
 * Essa abordagem segue a ideia de um hash generico orientado ao layout do
 * registro. Ela continua compativel com tipos opacos ou structs que tenham
 * ponteiros em memoria, porque a persistencia continua sendo feita por meio de
 * callbacks de serializacao e desserializacao.
 */

#include <stdbool.h>
#include <stddef.h>

#define HE_TAMANHO_CHAVE_MAX 63u

typedef void *HashExtensivel;

typedef bool (*HESerializarRegistro)(const void *registro, void *buffer_out,
                                     size_t tamanho_registro);

typedef bool (*HEDesserializarRegistro)(const void *buffer,
                                        size_t tamanho_registro,
                                        void *registro_out);

/*
 * Configuracao do layout persistido do registro.
 *
 * `tamanho_registro` e o tamanho fixo da representacao em disco.
 * `chave_offset` indica em qual byte dessa representacao a chave textual comeca.
 * `chave_tamanho` indica quantos bytes estao reservados para a chave, incluindo
 * o terminador `'\0'`.
 */
typedef struct {
    size_t tamanho_registro;
    size_t chave_offset;
    size_t chave_tamanho;
    HESerializarRegistro serializar;
    HEDesserializarRegistro desserializar;
} HEInfoRegistro;

bool he_serializar_bloco(const void *registro, void *buffer_out,
                         size_t tamanho_registro);

bool he_desserializar_bloco(const void *buffer, size_t tamanho_registro,
                            void *registro_out);

/*
 * Cria um novo arquivo de hash extensivel.
 *
 * Parametros:
 * - caminho_arquivo: caminho do arquivo binario
 * - capacidade_bucket: quantidade maxima de registros por bucket
 * - info: descricao do layout persistido do registro
 */
HashExtensivel he_criar(const char *caminho_arquivo, size_t capacidade_bucket,
                        const HEInfoRegistro *info);

/*
 * Abre um arquivo de hash extensivel ja existente.
 *
 * O tamanho do registro e carregado do arquivo e deve ser compativel com a
 * configuracao informada em `info`.
 */
HashExtensivel he_abrir(const char *caminho_arquivo, const HEInfoRegistro *info);

void he_fechar(HashExtensivel he);

/*
 * Insere ou atualiza um registro completo.
 *
 * A chave textual e extraida da representacao persistida do proprio registro,
 * usando `chave_offset` e `chave_tamanho`.
 */
bool he_inserir(HashExtensivel he, const void *registro);

/*
 * Busca um registro pela chave textual.
 */
bool he_buscar(HashExtensivel he, const char *chave, void *registro_out);

bool he_contem(HashExtensivel he, const char *chave);
bool he_remover(HashExtensivel he, const char *chave);
size_t he_tamanho(HashExtensivel he);
size_t he_profundidade_global(HashExtensivel he);
size_t he_tamanho_diretorio(HashExtensivel he);
size_t he_tamanho_registro(HashExtensivel he);

#endif
