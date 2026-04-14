#include "../unity/unity.h"
#include "../include/lista.h"

#include <stdlib.h>

static Lista lista;

void setUp(void) {
    lista = criaLista();
}

void tearDown(void) {
    liberaLista(lista);
    lista = NULL;
}

void test_criaLista_deve_iniciar_vazia(void) {
    TEST_ASSERT_NOT_NULL(lista);
    TEST_ASSERT_TRUE(listaVazia(lista));
    TEST_ASSERT_EQUAL_INT(0, getTamanhoLista(lista));
    TEST_ASSERT_NULL(getInicioLista(lista));
    TEST_ASSERT_NULL(getFimLista(lista));
}

void test_insereLista_deve_inserir_no_inicio(void) {
    static int a = 10;
    static int b = 20;

    insereLista(lista, &a);
    insereLista(lista, &b);

    TEST_ASSERT_FALSE(listaVazia(lista));
    TEST_ASSERT_EQUAL_INT(2, getTamanhoLista(lista));
    TEST_ASSERT_EQUAL_PTR(&b, getConteudoCelula(getInicioLista(lista)));
    TEST_ASSERT_EQUAL_PTR(&a, getConteudoCelula(getFimLista(lista)));
}

void test_insereFinalLista_deve_inserir_no_final(void) {
    static int a = 10;
    static int b = 20;

    insereFinalLista(lista, &a);
    insereFinalLista(lista, &b);

    TEST_ASSERT_EQUAL_INT(2, getTamanhoLista(lista));
    TEST_ASSERT_EQUAL_PTR(&a, getConteudoCelula(getInicioLista(lista)));
    TEST_ASSERT_EQUAL_PTR(&b, getConteudoCelula(getFimLista(lista)));
}

void test_removeInicioLista_e_removeFinalLista_devem_respeitar_ordem(void) {
    static int a = 10;
    static int b = 20;
    static int c = 30;

    insereFinalLista(lista, &a);
    insereFinalLista(lista, &b);
    insereFinalLista(lista, &c);

    TEST_ASSERT_EQUAL_PTR(&a, removeInicioLista(lista));
    TEST_ASSERT_EQUAL_PTR(&c, removeFinalLista(lista));
    TEST_ASSERT_EQUAL_INT(1, getTamanhoLista(lista));
    TEST_ASSERT_EQUAL_PTR(&b, getConteudoCelula(getInicioLista(lista)));
    TEST_ASSERT_EQUAL_PTR(&b, getConteudoCelula(getFimLista(lista)));
}

void test_getProxCelula_e_getAntCelula_devem_navegar_pela_lista(void) {
    static int a = 10;
    static int b = 20;
    static int c = 30;
    Celula inicio;
    Celula meio;
    Celula fim;

    insereFinalLista(lista, &a);
    insereFinalLista(lista, &b);
    insereFinalLista(lista, &c);

    inicio = getInicioLista(lista);
    meio = getProxCelula(inicio);
    fim = getFimLista(lista);

    TEST_ASSERT_EQUAL_PTR(&a, getConteudoCelula(inicio));
    TEST_ASSERT_EQUAL_PTR(&b, getConteudoCelula(meio));
    TEST_ASSERT_EQUAL_PTR(&c, getConteudoCelula(fim));
    TEST_ASSERT_EQUAL_PTR(meio, getAntCelula(fim));
    TEST_ASSERT_NULL(getAntCelula(inicio));
    TEST_ASSERT_NULL(getProxCelula(fim));
}

void test_removeElementoLista_deve_remover_sem_liberar_conteudo(void) {
    static int a = 10;
    static int b = 20;
    static int c = 30;

    insereFinalLista(lista, &a);
    insereFinalLista(lista, &b);
    insereFinalLista(lista, &c);

    TEST_ASSERT_TRUE(removeElementoLista(lista, &b));
    TEST_ASSERT_EQUAL_INT(2, getTamanhoLista(lista));
    TEST_ASSERT_EQUAL_PTR(&a, getConteudoCelula(getInicioLista(lista)));
    TEST_ASSERT_EQUAL_PTR(&c, getConteudoCelula(getFimLista(lista)));
    TEST_ASSERT_FALSE(removeElementoLista(lista, &b));
}

void test_searchAndRemoveLista_deve_remover_e_liberar_conteudo(void) {
    int *valor_a = malloc(sizeof(int));
    int *valor_b = malloc(sizeof(int));

    TEST_ASSERT_NOT_NULL(valor_a);
    TEST_ASSERT_NOT_NULL(valor_b);

    *valor_a = 10;
    *valor_b = 20;

    insereFinalLista(lista, valor_a);
    insereFinalLista(lista, valor_b);

    TEST_ASSERT_TRUE(searchAndRemoveLista(lista, valor_a));
    TEST_ASSERT_EQUAL_INT(1, getTamanhoLista(lista));
    TEST_ASSERT_EQUAL_PTR(valor_b, getConteudoCelula(getInicioLista(lista)));

    TEST_ASSERT_TRUE(searchAndRemoveLista(lista, valor_b));
    TEST_ASSERT_TRUE(listaVazia(lista));
}

void test_copiaListas_deve_manter_ordem_dos_elementos(void) {
    static int a = 10;
    static int b = 20;
    static int c = 30;
    Lista copia = criaLista();

    TEST_ASSERT_NOT_NULL(copia);

    insereFinalLista(lista, &a);
    insereFinalLista(lista, &b);
    insereFinalLista(lista, &c);

    copiaListas(copia, lista);

    TEST_ASSERT_EQUAL_INT(3, getTamanhoLista(copia));
    TEST_ASSERT_EQUAL_PTR(&a, getConteudoCelula(getInicioLista(copia)));
    TEST_ASSERT_EQUAL_PTR(&b, getConteudoCelula(getProxCelula(getInicioLista(copia))));
    TEST_ASSERT_EQUAL_PTR(&c, getConteudoCelula(getFimLista(copia)));

    liberaLista(copia);
}

void test_removeCelula_deve_remover_celula_especifica(void) {
    static int a = 10;
    static int b = 20;
    static int c = 30;
    Celula meio;

    insereFinalLista(lista, &a);
    insereFinalLista(lista, &b);
    insereFinalLista(lista, &c);

    meio = getProxCelula(getInicioLista(lista));
    removeCelula(lista, meio, false);

    TEST_ASSERT_EQUAL_INT(2, getTamanhoLista(lista));
    TEST_ASSERT_EQUAL_PTR(&a, getConteudoCelula(getInicioLista(lista)));
    TEST_ASSERT_EQUAL_PTR(&c, getConteudoCelula(getFimLista(lista)));
    TEST_ASSERT_EQUAL_PTR(&c, getConteudoCelula(getProxCelula(getInicioLista(lista))));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_criaLista_deve_iniciar_vazia);
    RUN_TEST(test_insereLista_deve_inserir_no_inicio);
    RUN_TEST(test_insereFinalLista_deve_inserir_no_final);
    RUN_TEST(test_removeInicioLista_e_removeFinalLista_devem_respeitar_ordem);
    RUN_TEST(test_getProxCelula_e_getAntCelula_devem_navegar_pela_lista);
    RUN_TEST(test_removeElementoLista_deve_remover_sem_liberar_conteudo);
    RUN_TEST(test_searchAndRemoveLista_deve_remover_e_liberar_conteudo);
    RUN_TEST(test_copiaListas_deve_manter_ordem_dos_elementos);
    RUN_TEST(test_removeCelula_deve_remover_celula_especifica);

    return UNITY_END();
}
