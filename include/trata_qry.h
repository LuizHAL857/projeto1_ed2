#ifndef TRATA_QRY_H
#define TRATA_QRY_H

/*
 * Modulo responsavel por processar integralmente um arquivo `.qry`.
 *
 * O processamento:
 * - executa comandos de consulta e atualizacao sobre os estados do `.geo`
 *   e do `.pm`
 * - gera um arquivo `.txt` com os resultados textuais das consultas
 * - gera o segundo arquivo `.svg`, representando o estado final apos o `.qry`
 */

#include "leitor_arquivos.h"
#include "trata_geo.h"
#include "trata_pm.h"

typedef void *TrataQry;

/*
 * Processa um arquivo `.qry` e gera os artefatos finais no diretorio de saida.
 *
 * Regras adotadas:
 * - o nome do SVG final e `<geo>-<qry>.svg`
 * - o nome do TXT final e `<geo>-<qry>.txt`
 * - o `.txt` copia cada consulta em uma linha precedida por `[*] `
 * - `trata_geo` e obrigatorio
 * - `trata_pm` pode ser NULL, caso em que as consultas sobre moradores
 *   consideram um conjunto vazio de habitantes
 *
 * Retorna NULL em caso de falha.
 */
TrataQry processa_qry(DadosDoArquivo dados_qry, TrataGeo trata_geo, TrataPm trata_pm,
                      const char *caminho_output);

/*
 * Libera os recursos em memoria associados ao processamento do `.qry`.
 *
 * Os arquivos gerados em disco nao sao removidos por esta funcao.
 */
void trata_qry_destruir(TrataQry trata_qry);

/*
 * Retorna o nome-base do arquivo `.qry`, sem extensao.
 */
const char *trata_qry_obter_nome_qry(TrataQry trata_qry);

#endif
