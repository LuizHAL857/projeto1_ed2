#include "stdio.h"
#include "stdlib.h"
#include "../include/lista.h"

typedef struct stCelula {
    void* chave;
    struct stCelula *prox;
    struct stCelula *ant;
} stCelula;  

typedef struct stLista {
    int tam;
    stCelula *inicio;
    stCelula *fim;
} stLista;

Lista criaLista() {
    stLista *l = malloc(sizeof(stLista));
    l->inicio = NULL;
    l->fim = NULL;
    l->tam = 0;
    return ((stLista*)l);
}

void insereLista(Lista l, void* chave) {
    stCelula *new = malloc(sizeof(stCelula));
    new->chave = chave;
    new->ant = NULL;

    if(((stLista*)l)->inicio == NULL) {
        // Lista vazia
        new->prox = NULL;
        ((stLista*)l)->inicio = new;
        ((stLista*)l)->fim = new;
    } else {
        // Inserir no início
        new->prox = ((stLista*)l)->inicio;
        ((stLista*)l)->inicio->ant = new;
        ((stLista*)l)->inicio = new;        
    }

    ((stLista*)l)->tam++;
}

void* getConteudoInicioLista(Lista l) {
    return ((stLista*)l)->inicio->chave;
}

Celula getInicioLista(Lista l) {
    return ((stLista*)l)->inicio;
}

void* removeInicioLista(Lista l) {
    if (l == NULL) return NULL;

    stLista *lista = (stLista*)l;

    if (lista->inicio == NULL) return NULL;   // lista vazia

    stCelula *aux = lista->inicio;
    void* c = aux->chave;

    lista->inicio = aux->prox;
    
    if (lista->inicio != NULL) {
        lista->inicio->ant = NULL;
    } else {
        // Lista ficou vazia
        lista->fim = NULL;
    }

    free(aux);
    lista->tam--;

    return c;
}

void* removeFinalLista(Lista l) {
    if (l == NULL) return NULL;
    
    stLista *lista = (stLista*)l;
    
    if (lista->fim == NULL) return NULL;  // lista vazia
    
    stCelula *ultimo = lista->fim;
    void* c = ultimo->chave;
    
    if (lista->fim->ant == NULL) {
        // Apenas um elemento
        lista->inicio = NULL;
        lista->fim = NULL;
    } else {
        // Mais de um elemento
        lista->fim = lista->fim->ant;
        lista->fim->prox = NULL;
    }
    
    free(ultimo);
    lista->tam--;
    
    return c;
}


int getTamanhoLista(Lista l) {
    return ((stLista*)l)->tam;
}

void copiaListas(Lista copia, Lista fonte) {
    Celula aux = ((stLista*)fonte)->inicio;
    while(aux != NULL) {
        insereFinalLista(copia, ((stCelula*)aux)->chave);
        aux = ((stCelula*)aux)->prox;
    }
}

void liberaLista(Lista l) {
    if (l == NULL) return;
    
    Celula atual = ((stLista*)l)->inicio;  
    while (atual != NULL) {
        Celula proximo = ((stCelula*)atual)->prox;
        free(atual);  
        atual = proximo;
    }
    
    free(l);  
}

Celula getProxCelula(Celula p) {
    Celula aux = ((stCelula*)p)->prox;
    if(aux != NULL) return aux;
    else return NULL;
}

void* getConteudoCelula(Celula p) {
    return ((stCelula*)p)->chave;
}

bool searchAndRemoveLista(Lista lista, void* chave) {
    if (lista == NULL || ((stLista*)lista)->inicio == NULL) return false;

    Celula atual = ((stLista*)lista)->inicio;

    while (atual != NULL) {
        if (((stCelula*)atual)->chave == chave) {
            // Atualizar ponteiro anterior
            if (((stCelula*)atual)->ant != NULL) {
                ((stCelula*)atual)->ant->prox = ((stCelula*)atual)->prox;
            } else {
                // Removendo do início
                ((stLista*)lista)->inicio = ((stCelula*)atual)->prox;
            }
            
            // Atualizar ponteiro próximo
            if (((stCelula*)atual)->prox != NULL) {
                ((stCelula*)atual)->prox->ant = ((stCelula*)atual)->ant;
            } else {
                // Removendo do fim
                ((stLista*)lista)->fim = ((stCelula*)atual)->ant;
            }

            // libera a chave e a célula
            free(((stCelula*)atual)->chave);
            free(atual);
            ((stLista*)lista)->tam--;
            return true;  // remove apenas a primeira ocorrência
        }

        atual = ((stCelula*)atual)->prox;
    }
    return false;
}

void removeCelula(Lista lista, Celula alvo, bool liberarConteudo) {
    (void)liberarConteudo; 
    if (alvo == NULL) return;

    // Atualizar ponteiro anterior
    if (((stCelula*)alvo)->ant != NULL) {
        ((stCelula*)alvo)->ant->prox = ((stCelula*)alvo)->prox;
    } else {
        // Removendo do início
        ((stLista*)lista)->inicio = ((stCelula*)alvo)->prox;
    }
    
    // Atualizar ponteiro próximo
    if (((stCelula*)alvo)->prox != NULL) {
        ((stCelula*)alvo)->prox->ant = ((stCelula*)alvo)->ant;
    } else {
        // Removendo do fim
        ((stLista*)lista)->fim = ((stCelula*)alvo)->ant;
    }
    
    ((stLista*)lista)->tam--;
}

void insereFinalLista(Lista l, void* chave) {
    stCelula *novo = malloc(sizeof(stCelula));
    novo->chave = chave;
    novo->prox = NULL;

    // Se a lista está vazia
    if (((stLista*)l)->fim == NULL) {
        novo->ant = NULL;
        ((stLista*)l)->inicio = novo;
        ((stLista*)l)->fim = novo;
    } else {
        // Inserir no final usando ponteiro fim
        novo->ant = ((stLista*)l)->fim;
        ((stLista*)l)->fim->prox = novo;
        ((stLista*)l)->fim = novo;
    }

    ((stLista*)l)->tam++;
}



bool listaVazia(Lista l) {
    if (l == NULL) return true;
    return (((stLista*)l)->inicio == NULL || ((stLista*)l)->tam == 0);
}

bool removeElementoLista(Lista lista, void* chave) {
    if (lista == NULL || ((stLista*)lista)->inicio == NULL) return false;

    Celula atual = ((stLista*)lista)->inicio;

    while (atual != NULL) {
        if (((stCelula*)atual)->chave == chave) {
            // Atualizar ponteiro anterior
            if (((stCelula*)atual)->ant != NULL) {
                ((stCelula*)atual)->ant->prox = ((stCelula*)atual)->prox;
            } else {
                // Removendo do início
                ((stLista*)lista)->inicio = ((stCelula*)atual)->prox;
            }
            
            // Atualizar ponteiro próximo
            if (((stCelula*)atual)->prox != NULL) {
                ((stCelula*)atual)->prox->ant = ((stCelula*)atual)->ant;
            } else {
                // Removendo do fim
                ((stLista*)lista)->fim = ((stCelula*)atual)->ant;
            }

            free(atual); // Libera a célula, mas NÃO o conteúdo
            ((stLista*)lista)->tam--;
            return true;
        }
        atual = ((stCelula*)atual)->prox;
    }
    return false;
}

Celula getAnteriorCelula(Celula p) {
    if (p == NULL) return NULL;
    return ((stCelula*)p)->ant;
}

Celula getFinalLista(Lista l) {
    if (l == NULL) return NULL;
    return ((stLista*)l)->fim;
}


Celula getFimLista(Lista l) {
    return getFinalLista(l);
}

Celula getAntCelula(Celula c) {
    return getAnteriorCelula(c);
}