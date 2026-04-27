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
 * - opcionalmente, um endereco de moradia (`cep`, `face`, `num`, `compl`)
 *
 * A interface publica usa apenas um tipo opaco.
 *
 *
 * A conversao entre essas duas representacoes acontece por meio de:
 * - `habitante_escrever_registro(...)`
 * - `habitante_criar_de_bytes(...)`
 */

#include "hash_extensivel.h"

#include <stdbool.h>
#include <stddef.h>

#define HABITANTE_TAMANHO_NOME_MAX 63u
#define HABITANTE_TAMANHO_SOBRENOME_MAX 63u
#define HABITANTE_TAMANHO_NASC_MAX 15u
#define HABITANTE_TAMANHO_COMPLEMENTO_MAX 63u

typedef void *Habitante;

/*
 * Cria um habitante.
 *
 * Retorna NULL quando algum campo textual for invalido, quando o CPF contiver
 * caracteres fora do formato aceito (alfanumericos, `-` e `.`), quando o sexo
 * nao for `M` nem `F`, ou em caso de falta de memoria.
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

/*
 * Define ou atualiza a moradia do habitante.
 *
 * Retorna false quando o habitante for invalido, quando o CEP for invalido,
 * quando a face nao pertencer ao conjunto `{N, S, L, O}`, quando `num` for
 * negativo ou quando o complemento for invalido.
 */
bool habitante_definir_moradia(Habitante habitante, const char *cep, char face,
                               int num, const char *compl);

/*
 * Remove a associacao de moradia do habitante, tornando-o sem-teto.
 */
void habitante_remover_moradia(Habitante habitante);

/*
 * Indica se o habitante possui uma moradia associada.
 */
bool habitante_eh_morador(Habitante habitante);

/*
 * Funcoes de acesso aos atributos do habitante.
 *
 * Para strings:
 * - pode retornar NULL se o habitante for invalido
 *
 * Para sexo:
 * - retorna 'M' ou 'F'
 * - retorna '\0' se habitante for NULL
 */

 const char *habitante_obter_cpf(Habitante habitante);
 const char *habitante_obter_nome(Habitante habitante);
 const char *habitante_obter_sobrenome(Habitante habitante);
 char habitante_obter_sexo(Habitante habitante);
 const char *habitante_obter_nasc(Habitante habitante);
 const char *habitante_obter_cep(Habitante habitante);
 char habitante_obter_face(Habitante habitante);
 int habitante_obter_num(Habitante habitante);
 const char *habitante_obter_compl(Habitante habitante);

#endif
