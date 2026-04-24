#ifndef HABITANTE_H
#define HABITANTE_H

/*
 * Modulo que representa um habitante de Bitnópolis.
 *
 * Cada habitante possui:
 * - cpf, usado como chave textual
 * - nome
 * - sobrenome
 * - sexo (`M` ou `F`)
 * - data de nascimento
 *
 * A interface publica usa apenas um tipo opaco. O formato persistido do
 * habitante fica escondido na implementacao e pode ser convertido para um
 * bloco fixo de bytes compativel com o modulo `hash_extensivel`.
 */

#include "hash_extensivel.h"

#include <stdbool.h>
#include <stddef.h>

#define HABITANTE_TAMANHO_NOME_MAX 63u
#define HABITANTE_TAMANHO_SOBRENOME_MAX 63u
#define HABITANTE_TAMANHO_NASC_MAX 15u

typedef void *Habitante;

/*
 * Cria um habitante.
 *
 * Retorna NULL quando algum campo textual for invalido, quando o sexo nao for
 * `M` nem `F`, ou em caso de falta de memoria.
 */
Habitante habitante_criar(const char *cpf, const char *nome, const char *sobrenome,
                          char sexo, const char *nasc);

/*
 * Cria um habitante a partir de um bloco persistido.
 */
Habitante habitante_criar_de_bytes(const void *dados_registro, size_t tamanho_registro);

/*
 * Libera toda a memoria associada ao habitante.
 */
void habitante_destruir(Habitante habitante);

/*
 * Copia o estado atual do habitante para um bloco persistido.
 */
bool habitante_escrever_registro(Habitante habitante, void *registro_out,
                                 size_t tamanho_registro);

/*
 * Retorna o tamanho do registro persistido do habitante.
 */
size_t habitante_tamanho_registro(void);

const char *habitante_obter_cpf(Habitante habitante);
const char *habitante_obter_nome(Habitante habitante);
const char *habitante_obter_sobrenome(Habitante habitante);
char habitante_obter_sexo(Habitante habitante);
const char *habitante_obter_nasc(Habitante habitante);

#endif
