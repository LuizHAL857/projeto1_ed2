#include "../unity/unity.h"
#include "../include/quadra.h"
#include "../include/hash_extensivel.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static const char *ARQ_HASH = "quadra_teste.hf";
static const char *ARQ_CONTROLE = "quadra_teste.hfc";
static Quadra quadra;

static void assert_quadra(Quadra q, const char *cep, double x, double y, double w, double h,
                          const char *sw, const char *cfill, const char *cstrk) {
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_STRING(cep, quadra_obter_cep(q));
    TEST_ASSERT_TRUE(fabs(quadra_obter_x(q) - x) <= 0.0001);
    TEST_ASSERT_TRUE(fabs(quadra_obter_y(q) - y) <= 0.0001);
    TEST_ASSERT_TRUE(fabs(quadra_obter_w(q) - w) <= 0.0001);
    TEST_ASSERT_TRUE(fabs(quadra_obter_h(q) - h) <= 0.0001);
    TEST_ASSERT_EQUAL_STRING(sw, quadra_obter_sw(q));
    TEST_ASSERT_EQUAL_STRING(cfill, quadra_obter_cfill(q));
    TEST_ASSERT_EQUAL_STRING(cstrk, quadra_obter_cstrk(q));
}

void setUp(void) {
    remove(ARQ_HASH);
    remove(ARQ_CONTROLE);
    quadra = quadra_criar("Q-01", 10.0, 20.0, 30.0, 40.0, "2px", "gold", "black");
}

void tearDown(void) {
    quadra_destruir(quadra);
    quadra = NULL;
    remove(ARQ_HASH);
    remove(ARQ_CONTROLE);
}

void test_quadra_criar_deve_inicializar_campos(void) {
    assert_quadra(quadra, "Q-01", 10.0, 20.0, 30.0, 40.0, "2px", "gold", "black");
}

void test_quadra_criar_deve_aceitar_cep_com_ponto(void) {
    Quadra com_ponto =
        quadra_criar("b01.1", 1.0, 2.0, 3.0, 4.0, "1px", "white", "black");

    assert_quadra(com_ponto, "b01.1", 1.0, 2.0, 3.0, 4.0, "1px", "white", "black");
    quadra_destruir(com_ponto);
}

void test_quadra_criar_deve_rejeitar_parametros_invalidos(void) {
    TEST_ASSERT_NULL(quadra_criar(NULL, 1.0, 2.0, 3.0, 4.0, "1", "a", "b"));
    TEST_ASSERT_NULL(quadra_criar("", 1.0, 2.0, 3.0, 4.0, "1", "a", "b"));
    TEST_ASSERT_NULL(quadra_criar("cep@erro", 1.0, 2.0, 3.0, 4.0, "1", "a", "b"));
    TEST_ASSERT_NULL(quadra_criar("Q-02", 1.0, 2.0, 0.0, 4.0, "1", "a", "b"));
    TEST_ASSERT_NULL(quadra_criar("Q-02", 1.0, 2.0, 3.0, -1.0, "1", "a", "b"));
    TEST_ASSERT_NULL(quadra_criar("Q-02", 1.0, 2.0, 3.0, 4.0, "", "a", "b"));
}

void test_quadra_definir_estilo_deve_atualizar_cores_e_espessura(void) {
    TEST_ASSERT_TRUE(quadra_definir_estilo(quadra, "4px", "white", "navy"));
    assert_quadra(quadra, "Q-01", 10.0, 20.0, 30.0, 40.0, "4px", "white", "navy");
}

void test_quadra_registro_deve_preservar_dados_em_roundtrip(void) {
    size_t tamanho_registro = quadra_tamanho_registro();
    unsigned char registro[tamanho_registro];
    Quadra reconstruida;

    memset(registro, 0, sizeof(registro));
    TEST_ASSERT_TRUE(quadra_escrever_registro(quadra, registro, tamanho_registro));
    TEST_ASSERT_EQUAL_STRING("Q-01", (const char *)registro);
    TEST_ASSERT_EQUAL_UINT(sizeof(registro), quadra_tamanho_registro());

    reconstruida = quadra_criar_de_bytes(registro, tamanho_registro);
    assert_quadra(reconstruida, "Q-01", 10.0, 20.0, 30.0, 40.0, "2px", "gold", "black");
    quadra_destruir(reconstruida);
}

void test_quadra_deve_ser_compativel_com_hash_extensivel(void) {
    HashExtensivel he;
    size_t tamanho_registro = quadra_tamanho_registro();
    unsigned char registro[tamanho_registro];
    unsigned char obtido[tamanho_registro];
    Quadra restaurada;

    memset(registro, 0, sizeof(registro));
    memset(obtido, 0, sizeof(obtido));
    TEST_ASSERT_TRUE(quadra_escrever_registro(quadra, registro, tamanho_registro));

    he = he_criar(ARQ_HASH, 2, tamanho_registro);
    TEST_ASSERT_NOT_NULL(he);
    TEST_ASSERT_TRUE(he_inserir(he, registro));
    he_fechar(he);

    he = he_abrir(ARQ_HASH, 2, tamanho_registro);
    TEST_ASSERT_NOT_NULL(he);
    TEST_ASSERT_TRUE(he_buscar(he, "Q-01", obtido));
    he_fechar(he);

    restaurada = quadra_criar_de_bytes(obtido, tamanho_registro);
    assert_quadra(restaurada, "Q-01", 10.0, 20.0, 30.0, 40.0, "2px", "gold", "black");
    quadra_destruir(restaurada);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_quadra_criar_deve_inicializar_campos);
    RUN_TEST(test_quadra_criar_deve_aceitar_cep_com_ponto);
    RUN_TEST(test_quadra_criar_deve_rejeitar_parametros_invalidos);
    RUN_TEST(test_quadra_definir_estilo_deve_atualizar_cores_e_espessura);
    RUN_TEST(test_quadra_registro_deve_preservar_dados_em_roundtrip);
    RUN_TEST(test_quadra_deve_ser_compativel_com_hash_extensivel);

    return UNITY_END();
}
