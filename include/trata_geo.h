#ifndef TRATA_GEO_H
#define TRATA_GEO_H

/*
 * Modulo responsavel por processar integralmente um arquivo `.geo`.
 
 * - `processa_geo(...)` executa todos os comandos do `.geo` e gera o SVG
 *   inicial
 * - `trata_geo_destruir(...)` desaloca o estado criado
 * - `trata_geo_obter_quadra(...)` permite acesso por CEP
 * - `trata_geo_obter_nome_geo(...)` expoe o nome-base do `.geo`, util para os
 *   proximos arquivos de saida do projeto
 */

#include "leitor_arquivos.h"
#include "lista.h"
#include "quadra.h"

typedef void *TrataGeo;

/*
 * Processa um arquivo `.geo` e gera os artefatos iniciais no diretorio de
 * saida.
 *
 * Regras adotadas:
 * - o nome do SVG inicial e `<nome-base-do-geo>.svg`
 * - as quadras sao mantidas em hashfile no diretorio de saida
 * - o dump textual `<nome-base-do-geo>-quadras.hfd` e gerado apenas ao final
 *   da execucao, durante `trata_geo_destruir(...)`
 * - se houver um `.qry`, os arquivos `.hf`, `.hfc` e `.hfd` passam a usar o
 *   prefixo `<nome-base-do-geo>-<nome-base-do-qry>`
 * - em caso de falha, retorna NULL
 *
 * Parametros:
 * - dados_geo: arquivo `.geo` previamente carregado
 * - caminho_output: diretorio onde os arquivos de saida devem ser criados
 * - nome_qry: nome-base opcional do `.qry`, usado apenas para os arquivos
 *   persistentes `.hf`, `.hfc` e `.hfd`
 */
TrataGeo processa_geo(DadosDoArquivo dados_geo, const char *caminho_output,
                      const char *nome_qry);

/*
 * Libera os recursos em memoria e fecha arquivos associados ao processamento.
 *
 * Os arquivos gerados em disco nao sao removidos por esta funcao.
 */
void trata_geo_destruir(TrataGeo trata_geo);

/*
 * Busca uma quadra pelo CEP.
 *
 * Retorna uma nova instancia independente da quadra encontrada. O chamador deve
 * libera-la com `quadra_destruir(...)`. Retorna NULL se o CEP nao existir ou
 * em caso de erro.
 */
Quadra trata_geo_obter_quadra(TrataGeo trata_geo, const char *cep);

/*
 * Retorna o nome-base do arquivo `.geo`, sem extensao.
 */
const char *trata_geo_obter_nome_geo(TrataGeo trata_geo);

/*
 * Retorna uma nova lista com clones de todas as quadras ativas.
 *
 * Ownership:
 * - a lista retornada pertence ao chamador
 * - cada elemento deve ser liberado com `quadra_destruir(...)`
 * - a propria lista deve ser liberada com `liberaLista(...)`
 *
 * Retorna NULL em caso de erro.
 */
Lista trata_geo_listar_quadras(TrataGeo trata_geo);

/*
 * Remove a quadra de um CEP do estado atual.
 *
 * Retorna true quando a quadra existia e foi removida da hash e das estruturas
 * em memoria. Retorna false se o CEP nao existir ou em caso de erro.
 */
bool trata_geo_remover_quadra(TrataGeo trata_geo, const char *cep);

#endif
