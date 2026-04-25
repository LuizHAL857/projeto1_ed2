#ifndef TRATA_ARGUMENTOS_H
#define TRATA_ARGUMENTOS_H

/*
 * Módulo utilitário para interpretação dos argumentos de linha de comando.
 *
 * O módulo oferece funções para localizar o valor associado a uma opção no
 * formato `-opcao valor` e para identificar um possível sufixo posicional do
 * comando, ignorando os pares de opções já reconhecidos.
 */

/*
 * Busca o valor associado a uma opção no vetor de argumentos.
 *
 * A busca considera opções no formato `-nome_opcao valor`. Se a opção for
 * encontrada, retorna o argumento imediatamente seguinte. Se a opção não
 * existir ou não possuir valor associado, retorna NULL.
 *
 * Parâmetros:
 * - argc: quantidade total de argumentos
 * - argv: vetor de argumentos da linha de comando
 * - nome_opcao: nome da opção sem o hífen inicial
 *
 * Retorno:
 * - ponteiro para o valor associado à opção
 * - NULL se a opção não existir ou estiver sem valor
 */
char *obter_valor_opcao(int argc, char *argv[], char *nome_opcao);

/*
 * Obtém o sufixo posicional do comando.
 *
 * A função desconsidera os pares `-opcao valor` presentes em `argv` e tenta
 * identificar um único argumento remanescente que não faça parte dessas
 * opções. Se houver exatamente um sufixo, ele é retornado. Se não houver
 * sufixo, o retorno é NULL.
 *
 * Observação:
 * - se mais de um sufixo for encontrado, a implementação atual encerra o
 *   programa com mensagem de erro
 *
 * Parâmetros:
 * - argc: quantidade total de argumentos
 * - argv: vetor de argumentos da linha de comando
 *
 * Retorno:
 * - ponteiro para o sufixo encontrado
 * - NULL se não houver sufixo
 */
char *obter_sufixo_comando(int argc, char *argv[]);

/*
 * Monta o caminho completo de um arquivo de entrada a partir do BED.
 *
 * Regras:
 * - se `nome_arquivo` for absoluto, retorna uma copia dele
 * - se `diretorio_base` for NULL ou vazio, usa o diretorio corrente `.`
 * - o chamador deve liberar a string retornada com `free`
 *
 * Retorno:
 * - string alocada dinamicamente com o caminho completo
 * - NULL em caso de erro ou se `nome_arquivo` for NULL
 */
char *montar_caminho_entrada(const char *diretorio_base, const char *nome_arquivo);

#endif
