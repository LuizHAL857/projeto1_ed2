#include "../include/habitante.h"

#include <stdlib.h>
#include <string.h>
/*
 * Representa o habitante "vivo" em memoria.
 *
 * Esta struct e usada pelo tipo opaco `Habitante` durante a execucao normal
 * do programa, com acesso pelos construtores, modificadores e acessores do
 * modulo.
 */
typedef struct {
    char cpf[HE_TAMANHO_CHAVE_MAX + 1];
    char nome[HABITANTE_TAMANHO_NOME_MAX + 1];
    char sobrenome[HABITANTE_TAMANHO_SOBRENOME_MAX + 1];
    char sexo;
    char nasc[HABITANTE_TAMANHO_NASC_MAX + 1];
    char tem_moradia;
    char cep[HE_TAMANHO_CHAVE_MAX + 1];
    char face;
    int num;
    char compl[HABITANTE_TAMANHO_COMPLEMENTO_MAX + 1];
} HabitanteImpl;

/*
 * Representa o formato persistido do habitante.
 *
 * Esta struct nao e o objeto manipulado pela aplicacao no dia a dia; ela
 * existe para definir exatamente o layout do registro gravado/lido em arquivos
 * da hash extensivel. Por isso o tipo e `HE_PACKED` e so deve ser usado nas
 * funcoes de serializacao e desserializacao.
 */
typedef struct HE_PACKED {
    char cpf[HE_TAMANHO_CHAVE_MAX + 1];
    char nome[HABITANTE_TAMANHO_NOME_MAX + 1];
    char sobrenome[HABITANTE_TAMANHO_SOBRENOME_MAX + 1];
    char sexo;
    char nasc[HABITANTE_TAMANHO_NASC_MAX + 1];
    char tem_moradia;
    char cep[HE_TAMANHO_CHAVE_MAX + 1];
    char face;
    int num;
    char compl[HABITANTE_TAMANHO_COMPLEMENTO_MAX + 1];
} HabitanteRegistroInterno;

static HabitanteImpl *habitante_impl(Habitante habitante) {
    return (HabitanteImpl *)habitante;
}

static bool texto_valido(const char *texto, size_t tamanho_maximo) {
    return texto != NULL && texto[0] != '\0' && strlen(texto) <= tamanho_maximo;
}

static bool cpf_valido(const char *cpf) {
    size_t i;
    size_t tamanho;

    if (!texto_valido(cpf, HE_TAMANHO_CHAVE_MAX)) {
        return false;
    }

    tamanho = strlen(cpf);
    for (i = 0; i < tamanho; i++) {
        unsigned char c = (unsigned char)cpf[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') || c == '-' || c == '.')) {
            return false;
        }
    }

    return true;
}

static bool cep_valido(const char *cep) {
    size_t i;
    size_t tamanho;

    if (!texto_valido(cep, HE_TAMANHO_CHAVE_MAX)) {
        return false;
    }

    tamanho = strlen(cep);
    for (i = 0; i < tamanho; i++) {
        unsigned char c = (unsigned char)cep[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') || c == '-' || c == '.')) {
            return false;
        }
    }

    return true;
}

static bool sexo_valido(char sexo) {
    return sexo == 'M' || sexo == 'F';
}

static bool face_valida(char face) {
    return face == 'N' || face == 'S' || face == 'L' || face == 'O';
}

static bool copiar_texto(char *destino, size_t tamanho_destino, const char *origem) {
    size_t tamanho;

    if (destino == NULL || origem == NULL) {
        return false;
    }

    tamanho = strlen(origem);
    if (tamanho >= tamanho_destino) {
        return false;
    }

    memcpy(destino, origem, tamanho + 1u);
    return true;
}

Habitante habitante_criar(const char *cpf, const char *nome, const char *sobrenome,
                          char sexo, const char *nasc) {
    HabitanteImpl *habitante;

    if (!cpf_valido(cpf) || !texto_valido(nome, HABITANTE_TAMANHO_NOME_MAX) ||
        !texto_valido(sobrenome, HABITANTE_TAMANHO_SOBRENOME_MAX) ||
        !sexo_valido(sexo) || !texto_valido(nasc, HABITANTE_TAMANHO_NASC_MAX)) {
        return NULL;
    }

    habitante = (HabitanteImpl *)calloc(1, sizeof(HabitanteImpl));
    if (habitante == NULL) {
        return NULL;
    }

    copiar_texto(habitante->cpf, sizeof(habitante->cpf), cpf);
    copiar_texto(habitante->nome, sizeof(habitante->nome), nome);
    copiar_texto(habitante->sobrenome, sizeof(habitante->sobrenome), sobrenome);
    habitante->sexo = sexo;
    copiar_texto(habitante->nasc, sizeof(habitante->nasc), nasc);
    habitante->num = -1;
    return habitante;
}

Habitante habitante_criar_de_bytes(const void *dados_registro, size_t tamanho_registro) {
    const HabitanteRegistroInterno *registro;
    Habitante habitante;

    if (dados_registro == NULL || tamanho_registro != sizeof(HabitanteRegistroInterno)) {
        return NULL;
    }

    /* Converte do formato persistido para o objeto em memoria. */
    registro = (const HabitanteRegistroInterno *)dados_registro;
    habitante = habitante_criar(registro->cpf, registro->nome, registro->sobrenome,
                                registro->sexo, registro->nasc);
    if (habitante == NULL) {
        return NULL;
    }

    if (registro->tem_moradia != 0 &&
        !habitante_definir_moradia(habitante, registro->cep, registro->face,
                                   registro->num, registro->compl)) {
        habitante_destruir(habitante);
        return NULL;
    }

    return habitante;
}

void habitante_destruir(Habitante habitante) {
    free(habitante_impl(habitante));
}

bool habitante_escrever_registro(Habitante habitante, void *registro_out,
                                 size_t tamanho_registro) {
    HabitanteImpl *h = habitante_impl(habitante);
    HabitanteRegistroInterno *registro;

    if (h == NULL || registro_out == NULL ||
        tamanho_registro != sizeof(HabitanteRegistroInterno)) {
        return false;
    }

    /* Converte do objeto em memoria para o formato persistido. */
    registro = (HabitanteRegistroInterno *)registro_out;
    memset(registro, 0, sizeof(*registro));
    memcpy(registro->cpf, h->cpf, sizeof(registro->cpf));
    memcpy(registro->nome, h->nome, sizeof(registro->nome));
    memcpy(registro->sobrenome, h->sobrenome, sizeof(registro->sobrenome));
    registro->sexo = h->sexo;
    memcpy(registro->nasc, h->nasc, sizeof(registro->nasc));
    registro->tem_moradia = h->tem_moradia;
    memcpy(registro->cep, h->cep, sizeof(registro->cep));
    registro->face = h->face;
    registro->num = h->num;
    memcpy(registro->compl, h->compl, sizeof(registro->compl));
    return true;
}

size_t habitante_tamanho_registro(void) {
    return sizeof(HabitanteRegistroInterno);
}

bool habitante_definir_moradia(Habitante habitante, const char *cep, char face,
                               int num, const char *compl) {
    HabitanteImpl *h = habitante_impl(habitante);

    if (h == NULL || !cep_valido(cep) || !face_valida(face) || num < 0 ||
        !texto_valido(compl, HABITANTE_TAMANHO_COMPLEMENTO_MAX)) {
        return false;
    }

    copiar_texto(h->cep, sizeof(h->cep), cep);
    h->face = face;
    h->num = num;
    copiar_texto(h->compl, sizeof(h->compl), compl);
    h->tem_moradia = 1;
    return true;
}

void habitante_remover_moradia(Habitante habitante) {
    HabitanteImpl *h = habitante_impl(habitante);

    if (h == NULL) {
        return;
    }

    h->tem_moradia = 0;
    h->cep[0] = '\0';
    h->face = '\0';
    h->num = -1;
    h->compl[0] = '\0';
}

bool habitante_eh_morador(Habitante habitante) {
    HabitanteImpl *h = habitante_impl(habitante);
    return h != NULL && h->tem_moradia != 0;
}

const char *habitante_obter_cpf(Habitante habitante) {
    HabitanteImpl *h = habitante_impl(habitante);
    return h == NULL ? NULL : h->cpf;
}

const char *habitante_obter_nome(Habitante habitante) {
    HabitanteImpl *h = habitante_impl(habitante);
    return h == NULL ? NULL : h->nome;
}

const char *habitante_obter_sobrenome(Habitante habitante) {
    HabitanteImpl *h = habitante_impl(habitante);
    return h == NULL ? NULL : h->sobrenome;
}

char habitante_obter_sexo(Habitante habitante) {
    HabitanteImpl *h = habitante_impl(habitante);
    return h == NULL ? '\0' : h->sexo;
}

const char *habitante_obter_nasc(Habitante habitante) {
    HabitanteImpl *h = habitante_impl(habitante);
    return h == NULL ? NULL : h->nasc;
}

const char *habitante_obter_cep(Habitante habitante) {
    HabitanteImpl *h = habitante_impl(habitante);
    return h == NULL || h->tem_moradia == 0 ? NULL : h->cep;
}

char habitante_obter_face(Habitante habitante) {
    HabitanteImpl *h = habitante_impl(habitante);
    return h == NULL || h->tem_moradia == 0 ? '\0' : h->face;
}

int habitante_obter_num(Habitante habitante) {
    HabitanteImpl *h = habitante_impl(habitante);
    return h == NULL || h->tem_moradia == 0 ? -1 : h->num;
}

const char *habitante_obter_compl(Habitante habitante) {
    HabitanteImpl *h = habitante_impl(habitante);
    return h == NULL || h->tem_moradia == 0 ? NULL : h->compl;
}
