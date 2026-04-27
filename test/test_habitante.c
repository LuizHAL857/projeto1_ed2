#include "../unity/unity.h"
#include "../include/habitante.h"
#include "../include/hash_extensivel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *ARQ_HASH = "habitante_teste.hf";
static const char *ARQ_CONTROLE = "habitante_teste.hfc";
static Habitante habitante;

static void assert_habitante(Habitante h, const char *cpf, const char *nome,
                             const char *sobrenome, char sexo, const char *nasc) {
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_EQUAL_STRING(cpf, habitante_obter_cpf(h));
    TEST_ASSERT_EQUAL_STRING(nome, habitante_obter_nome(h));
    TEST_ASSERT_EQUAL_STRING(sobrenome, habitante_obter_sobrenome(h));
    TEST_ASSERT_EQUAL_CHAR(sexo, habitante_obter_sexo(h));
    TEST_ASSERT_EQUAL_STRING(nasc, habitante_obter_nasc(h));
}

static void assert_moradia(Habitante h, const char *cep, char face, int num,
                           const char *compl) {
    TEST_ASSERT_TRUE(habitante_eh_morador(h));
    TEST_ASSERT_EQUAL_STRING(cep, habitante_obter_cep(h));
    TEST_ASSERT_EQUAL_CHAR(face, habitante_obter_face(h));
    TEST_ASSERT_EQUAL_INT(num, habitante_obter_num(h));
    TEST_ASSERT_EQUAL_STRING(compl, habitante_obter_compl(h));
}

void setUp(void) {
    remove(ARQ_HASH);
    remove(ARQ_CONTROLE);
    habitante = habitante_criar("cpf-001", "Ana", "Silva", 'F', "10/01/1999");
}

void tearDown(void) {
    habitante_destruir(habitante);
    habitante = NULL;
    remove(ARQ_HASH);
    remove(ARQ_CONTROLE);
}

void test_habitante_criar_deve_inicializar_campos(void) {
    assert_habitante(habitante, "cpf-001", "Ana", "Silva", 'F', "10/01/1999");
    TEST_ASSERT_FALSE(habitante_eh_morador(habitante));
}

void test_habitante_criar_deve_rejeitar_parametros_invalidos(void) {
    TEST_ASSERT_NULL(habitante_criar(NULL, "Ana", "Silva", 'F', "10/01/1999"));
    TEST_ASSERT_NULL(habitante_criar("", "Ana", "Silva", 'F', "10/01/1999"));
    TEST_ASSERT_NULL(habitante_criar("cpf@erro", "Ana", "Silva", 'F', "10/01/1999"));
    TEST_ASSERT_NULL(habitante_criar("cpf-002", NULL, "Silva", 'F', "10/01/1999"));
    TEST_ASSERT_NULL(habitante_criar("cpf-002", "Ana", "", 'F', "10/01/1999"));
    TEST_ASSERT_NULL(habitante_criar("cpf-002", "Ana", "Silva", 'X', "10/01/1999"));
    TEST_ASSERT_NULL(habitante_criar("cpf-002", "Ana", "Silva", 'F', ""));
}

void test_habitante_criar_deve_aceitar_cpf_com_pontuacao(void) {
    Habitante com_pontos =
        habitante_criar("000.000.001-91", "Ana", "Silva", 'F', "10/01/1999");

    TEST_ASSERT_NOT_NULL(com_pontos);
    TEST_ASSERT_EQUAL_STRING("000.000.001-91", habitante_obter_cpf(com_pontos));

    habitante_destruir(com_pontos);
}

void test_habitante_registro_deve_preservar_dados_em_roundtrip(void) {
    size_t tamanho_registro = habitante_tamanho_registro();
    unsigned char *registro;
    Habitante reconstruido;

    registro = (unsigned char *)malloc(tamanho_registro);
    TEST_ASSERT_NOT_NULL(registro);
    memset(registro, 0, tamanho_registro);
    TEST_ASSERT_TRUE(habitante_escrever_registro(habitante, registro, tamanho_registro));
    TEST_ASSERT_EQUAL_STRING("cpf-001", (const char *)registro);

    reconstruido = habitante_criar_de_bytes(registro, tamanho_registro);
    assert_habitante(reconstruido, "cpf-001", "Ana", "Silva", 'F', "10/01/1999");
    habitante_destruir(reconstruido);
    free(registro);
}

void test_habitante_deve_permitir_definir_e_remover_moradia(void) {
    TEST_ASSERT_TRUE(habitante_definir_moradia(habitante, "b01.1", 'N', 42, "apto1"));
    assert_moradia(habitante, "b01.1", 'N', 42, "apto1");

    habitante_remover_moradia(habitante);
    TEST_ASSERT_FALSE(habitante_eh_morador(habitante));
    TEST_ASSERT_NULL(habitante_obter_cep(habitante));
}

void test_habitante_deve_rejeitar_moradia_invalida(void) {
    TEST_ASSERT_FALSE(habitante_definir_moradia(habitante, NULL, 'N', 42, "apto1"));
    TEST_ASSERT_FALSE(habitante_definir_moradia(habitante, "b01.1", 'X', 42, "apto1"));
    TEST_ASSERT_FALSE(habitante_definir_moradia(habitante, "b01.1", 'N', -1, "apto1"));
    TEST_ASSERT_FALSE(habitante_definir_moradia(habitante, "b01.1", 'N', 42, ""));
}

void test_habitante_deve_ser_compativel_com_hash_extensivel(void) {
    HashExtensivel he;
    size_t tamanho_registro = habitante_tamanho_registro();
    unsigned char *registro;
    unsigned char *obtido;
    Habitante restaurado;

    registro = (unsigned char *)malloc(tamanho_registro);
    obtido = (unsigned char *)malloc(tamanho_registro);
    TEST_ASSERT_NOT_NULL(registro);
    TEST_ASSERT_NOT_NULL(obtido);
    memset(registro, 0, tamanho_registro);
    memset(obtido, 0, tamanho_registro);
    TEST_ASSERT_TRUE(habitante_definir_moradia(habitante, "b01.1", 'N', 42, "apto1"));
    TEST_ASSERT_TRUE(habitante_escrever_registro(habitante, registro, tamanho_registro));

    he = he_criar(ARQ_HASH, 2, tamanho_registro);
    TEST_ASSERT_NOT_NULL(he);
    TEST_ASSERT_TRUE(he_inserir(he, registro));
    he_fechar(he);

    he = he_abrir(ARQ_HASH, 2, tamanho_registro);
    TEST_ASSERT_NOT_NULL(he);
    TEST_ASSERT_TRUE(he_buscar(he, "cpf-001", obtido));
    he_fechar(he);

    restaurado = habitante_criar_de_bytes(obtido, tamanho_registro);
    assert_habitante(restaurado, "cpf-001", "Ana", "Silva", 'F', "10/01/1999");
    assert_moradia(restaurado, "b01.1", 'N', 42, "apto1");
    habitante_destruir(restaurado);
    free(registro);
    free(obtido);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_habitante_criar_deve_inicializar_campos);
    RUN_TEST(test_habitante_criar_deve_rejeitar_parametros_invalidos);
    RUN_TEST(test_habitante_criar_deve_aceitar_cpf_com_pontuacao);
    RUN_TEST(test_habitante_registro_deve_preservar_dados_em_roundtrip);
    RUN_TEST(test_habitante_deve_permitir_definir_e_remover_moradia);
    RUN_TEST(test_habitante_deve_rejeitar_moradia_invalida);
    RUN_TEST(test_habitante_deve_ser_compativel_com_hash_extensivel);

    return UNITY_END();
}
