#ifndef LISTA_H
#define LISTA_H

/*
 * Módulo de lista duplamente encadeada genérica.
 *
 * A estrutura armazena ponteiros genéricos (`void *`) e oferece operações de
 * criação, inserção, remoção, navegação entre células e consultas utilitárias.
 * O conteúdo apontado por cada elemento é de responsabilidade do código cliente,
 * exceto nas operações que explicitamente liberam esse conteúdo.
 */

#include <stdbool.h>

/**
 * @brief Tipo opaco para a lista duplamente encadeada.
 */
typedef void* Lista;

/**
 * @brief Tipo opaco para uma célula da lista.
 */
typedef void* Celula;

/* ================= Criação e liberação ================= */

/**
 * @brief Cria uma nova lista duplamente encadeada.
 * 
 * @return Um ponteiro para a lista criada.
 */
Lista criaLista();

/**
 * @brief Libera toda a memória da lista e de seus conteúdos.
 * 
 * @param l Ponteiro para a lista a ser liberada.
 */
void liberaLista(Lista l);

/* ================= Inserção ================= */

/**
 * @brief Insere um elemento no início da lista.
 * 
 * @param l Ponteiro para a lista.
 * @param chave Ponteiro para o conteúdo a ser armazenado.
 */
void insereLista(Lista l, void *chave);

/**
 * @brief Insere um elemento no final da lista.
 * 
 * @param l Ponteiro para a lista.
 * @param chave Ponteiro para o conteúdo a ser armazenado.
 */
void insereFinalLista(Lista l, void *chave);

/* ================= Remoção ================= */

/**
 * @brief Remove o elemento do início da lista.
 * 
 * @param l Ponteiro para a lista.
 * @return Ponteiro para o conteúdo removido ou NULL se a lista estiver vazia.
 */
void* removeInicioLista(Lista l);

/**
 * @brief Remove o elemento do final da lista.
 * 
 * @param l Ponteiro para a lista.
 * @return Ponteiro para o conteúdo removido ou NULL se a lista estiver vazia.
 */
void* removeFinalLista(Lista l);

/**
 * @brief Remove o primeiro elemento da lista que corresponda à chave, sem liberar o conteúdo.
 * 
 * @param l Ponteiro para a lista.
 * @param chave Ponteiro para o conteúdo a ser removido.
 * @return true se o elemento foi removido, false caso contrário.
 */
bool removeElementoLista(Lista l, void *chave);

/**
 * @brief Remove o primeiro elemento da lista que corresponda à chave, liberando seu conteúdo.
 * 
 * @param l Ponteiro para a lista.
 * @param chave Ponteiro para o conteúdo a ser removido.
 * @return true se o elemento foi removido, false caso contrário.
 */
bool searchAndRemoveLista(Lista l, void *chave);

/**
 * @brief Remove uma célula específica da lista.
 * 
 * @param l Ponteiro para a lista.
 * @param alvo Ponteiro para a célula a ser removida.
 * @param liberarConteudo Se true, libera também o conteúdo armazenado.
 */
void removeCelula(Lista l, Celula alvo, bool liberarConteudo);

/* ================= Navegação ================= */

/**
 * @brief Retorna a primeira célula da lista.
 * 
 * @param l Ponteiro para a lista.
 * @return Ponteiro para a primeira célula ou NULL se a lista estiver vazia.
 */
Celula getInicioLista(Lista l);

/**
 * @brief Retorna a última célula da lista.
 * 
 * @param l Ponteiro para a lista.
 * @return Ponteiro para a última célula ou NULL se a lista estiver vazia.
 */
Celula getFimLista(Lista l);

/**
 * @brief Retorna a célula seguinte à célula fornecida.
 * 
 * @param c Ponteiro para a célula atual.
 * @return Ponteiro para a célula seguinte ou NULL se não houver.
 */
Celula getProxCelula(Celula c);

/**
 * @brief Retorna a célula anterior à célula fornecida.
 * 
 * @param c Ponteiro para a célula atual.
 * @return Ponteiro para a célula anterior ou NULL se não houver.
 */
Celula getAntCelula(Celula c);

/**
 * @brief Retorna o conteúdo armazenado em uma célula.
 * 
 * @param c Ponteiro para a célula.
 * @return Ponteiro para o conteúdo armazenado ou NULL se a célula for NULL.
 */
void* getConteudoCelula(Celula c);

/* ================= Utilitários ================= */

/**
 * @brief Retorna o tamanho da lista (número de elementos).
 * 
 * @param l Ponteiro para a lista.
 * @return Número de elementos na lista.
 */
int getTamanhoLista(Lista l);

/**
 * @brief Verifica se a lista está vazia.
 * 
 * @param l Ponteiro para a lista.
 * @return true se a lista estiver vazia ou NULL, false caso contrário.
 */
bool listaVazia(Lista l);

/**
 * @brief Copia todos os elementos de uma lista para outra, mantendo a ordem.
 * 
 * @param copia Ponteiro para a lista de destino.
 * @param fonte Ponteiro para a lista de origem.
 */
void copiaListas(Lista copia, Lista fonte);

#endif
