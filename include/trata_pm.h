#ifndef TRATA_PM_H
#define TRATA_PM_H

/*
 * Modulo responsavel por processar integralmente um arquivo `.pm`.
 *
 * - `processa_pm(...)` executa todos os comandos do `.pm`
 * - `trata_pm_destruir(...)` desaloca o estado criado
 * - `trata_pm_obter_habitante(...)` permite consultas por CPF
 * - `trata_pm_obter_nome_pm(...)` expoe o nome-base do arquivo `.pm`
 *
 * O processamento cria um hash persistente de habitantes, incluindo a
 * eventual moradia associada a cada CPF, alem do dump `.hfd` correspondente.
 */

#include "habitante.h"
#include "leitor_arquivos.h"
#include "lista.h"
#include "trata_geo.h"

typedef void *TrataPm;

/*
 * Processa um arquivo `.pm` e gera os artefatos persistentes associados.
 *
 * Regras adotadas:
 * - o nome-base do `.pm` e usado como prefixo dos arquivos gerados
 * - habitantes sao persistidos em `<base>-habitantes.hf`
 * - o dump textual `<base>-habitantes.hfd` e gerado apenas ao final da
 *   execucao, durante `trata_pm_destruir(...)`
 * - comandos `m` validam se o habitante existe
 * - comandos `m` validam se a quadra do CEP informado existe no estado
 *   carregado do `.geo`
 * - em caso de falha, retorna NULL
 *
 * Parametros:
 * - dados_pm: arquivo `.pm` previamente carregado
 * - trata_geo: estado do `.geo`, obrigatorio para validar CEPs dos moradores
 * - caminho_output: diretorio onde os arquivos persistentes devem ser criados
 */
TrataPm processa_pm(DadosDoArquivo dados_pm, TrataGeo trata_geo,
                    const char *caminho_output);

/*
 * Libera os recursos em memoria e fecha arquivos associados ao processamento.
 *
 * Os arquivos gerados em disco nao sao removidos por esta funcao.
 */
void trata_pm_destruir(TrataPm trata_pm);

/*
 * Busca um habitante pelo CPF.
 *
 * Retorna uma nova instancia independente do habitante encontrado. O chamador
 * deve libera-la com `habitante_destruir(...)`.
 */
Habitante trata_pm_obter_habitante(TrataPm trata_pm, const char *cpf);

/*
 * Retorna o nome-base do arquivo `.pm`, sem extensao.
 */
const char *trata_pm_obter_nome_pm(TrataPm trata_pm);

/*
 * Insere um novo habitante no estado atual.
 *
 * Retorna false se o CPF ja existir, se algum dado for invalido ou em caso de
 * erro de persistencia.
 */
bool trata_pm_inserir_habitante(TrataPm trata_pm, const char *cpf, const char *nome,
                                const char *sobrenome, char sexo, const char *nasc);

/*
 * Define ou atualiza a moradia de um habitante existente.
 *
 * O CEP informado precisa existir no estado carregado do `.geo`.
 */
bool trata_pm_definir_moradia(TrataPm trata_pm, const char *cpf, const char *cep,
                              char face, int num, const char *compl);

/*
 * Remove a moradia de um habitante existente.
 *
 * Retorna false se o CPF nao existir, se o habitante nao for morador ou em
 * caso de erro de persistencia.
 */
bool trata_pm_remover_moradia(TrataPm trata_pm, const char *cpf);

/*
 * Remove um habitante existente do estado atual.
 *
 * Retorna false se o CPF nao existir ou em caso de erro.
 */
bool trata_pm_remover_habitante(TrataPm trata_pm, const char *cpf);

/*
 * Retorna uma nova lista com clones de todos os habitantes carregados.
 *
 * Ownership:
 * - a lista retornada pertence ao chamador
 * - cada elemento deve ser liberado com `habitante_destruir(...)`
 * - a propria lista deve ser liberada com `liberaLista(...)`
 *
 * Retorna NULL em caso de erro.
 */
Lista trata_pm_listar_habitantes(TrataPm trata_pm);

/*
 * Remove a moradia de todos os moradores vinculados a um CEP.
 *
 * A funcao atualiza o estado em memoria e o hash persistente, retornando uma
 * nova lista com clones dos habitantes afetados no estado anterior.
 *
 * Ownership:
 * - a lista retornada pertence ao chamador
 * - cada elemento deve ser liberado com `habitante_destruir(...)`
 * - a propria lista deve ser liberada com `liberaLista(...)`
 *
 * Retorna NULL em caso de erro. Se nenhum morador for afetado, retorna uma
 * lista vazia.
 */
Lista trata_pm_tornar_sem_teto_por_cep(TrataPm trata_pm, const char *cep);

#endif
