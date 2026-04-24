#ifndef QUADRA_H
#define QUADRA_H

/*
 * Modulo que representa uma quadra de Bitnópolis.
 *
 * A quadra e identificada pelo CEP e guarda:
 * - coordenadas de ancoragem `x` e `y`
 * - largura `w`
 * - altura `h`
 * - espessura da borda `sw`
 * - cor de preenchimento `cfill`
 * - cor da borda `cstrk`
 *
 * A interface publica expõe apenas um tipo opaco para uso no restante do
 * projeto. O formato persistido da quadra fica escondido na implementacao e e
 * acessado por meio de funcoes que escrevem e leem um bloco fixo de bytes,
 * compativel com o modulo `hash_extensivel`.
 */

#include "hash_extensivel.h"

#include <stdbool.h>
#include <stddef.h>

#define QUADRA_TAMANHO_ESPESSURA_MAX 31u
#define QUADRA_TAMANHO_COR_MAX 31u

typedef void *Quadra;

/*
 * Cria uma quadra.
 *
 * Retorna NULL quando algum parametro textual for invalido, quando `w` ou `h`
 * forem menores ou iguais a zero, ou em caso de falta de memoria.
 */
Quadra quadra_criar(const char *cep, double x, double y, double w, double h,
                    const char *sw, const char *cfill, const char *cstrk);

/*
 * Cria uma quadra a partir de um registro persistido.
 */
Quadra quadra_criar_de_bytes(const void *dados_registro, size_t tamanho_registro);

/*
 * Libera toda a memoria associada a quadra.
 */
void quadra_destruir(Quadra quadra);

/*
 * Atualiza o estilo visual da quadra.
 */
bool quadra_definir_estilo(Quadra quadra, const char *sw, const char *cfill,
                           const char *cstrk);

/*
 * Copia o estado atual da quadra para um registro persistido.
 *
 * Parametros:
 * - quadra: quadra a ser serializada
 * - registro_out: buffer onde os dados serao escritos
 * - tamanho_registro: tamanho do buffer fornecido
 *
 * Retorno:
 * - true se a escrita foi realizada com sucesso
 * - false se quadra for NULL, buffer for NULL ou tamanho insuficiente
 */
bool quadra_escrever_registro(Quadra quadra, void *registro_out, size_t tamanho_registro);

/*
 * Retorna o tamanho, em bytes, necessario para armazenar uma quadra
 * em formato persistido.
 *
 * Esse valor deve ser usado para alocar buffers e para integracao
 * com o modulo de hash extensivel.
 */
size_t quadra_tamanho_registro(void);

/*
 * Retorna o CEP da quadra.
 */
const char *quadra_obter_cep(Quadra quadra);

/*
 * Retornam as propriedades geometricas da quadra.
 */
double quadra_obter_x(Quadra quadra);
double quadra_obter_y(Quadra quadra);
double quadra_obter_w(Quadra quadra);
double quadra_obter_h(Quadra quadra);

/*
 * Retornam as propriedades visuais da quadra.
 */
const char *quadra_obter_sw(Quadra quadra);
const char *quadra_obter_cfill(Quadra quadra);
const char *quadra_obter_cstrk(Quadra quadra);

#endif