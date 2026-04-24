#include "../include/habitante.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    char cpf[HE_TAMANHO_CHAVE_MAX + 1];
    char nome[HABITANTE_TAMANHO_NOME_MAX + 1];
    char sobrenome[HABITANTE_TAMANHO_SOBRENOME_MAX + 1];
    char sexo;
    char nasc[HABITANTE_TAMANHO_NASC_MAX + 1];
} HabitanteImpl;

typedef struct HE_PACKED {
    char cpf[HE_TAMANHO_CHAVE_MAX + 1];
    char nome[HABITANTE_TAMANHO_NOME_MAX + 1];
    char sobrenome[HABITANTE_TAMANHO_SOBRENOME_MAX + 1];
    char sexo;
    char nasc[HABITANTE_TAMANHO_NASC_MAX + 1];
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
              (c >= 'a' && c <= 'z') || c == '-')) {
            return false;
        }
    }

    return true;
}

static bool sexo_valido(char sexo) {
    return sexo == 'M' || sexo == 'F';
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
    return habitante;
}

Habitante habitante_criar_de_bytes(const void *dados_registro, size_t tamanho_registro) {
    const HabitanteRegistroInterno *registro;

    if (dados_registro == NULL || tamanho_registro != sizeof(HabitanteRegistroInterno)) {
        return NULL;
    }

    registro = (const HabitanteRegistroInterno *)dados_registro;
    return habitante_criar(registro->cpf, registro->nome, registro->sobrenome,
                           registro->sexo, registro->nasc);
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

    registro = (HabitanteRegistroInterno *)registro_out;
    memset(registro, 0, sizeof(*registro));
    memcpy(registro->cpf, h->cpf, sizeof(registro->cpf));
    memcpy(registro->nome, h->nome, sizeof(registro->nome));
    memcpy(registro->sobrenome, h->sobrenome, sizeof(registro->sobrenome));
    registro->sexo = h->sexo;
    memcpy(registro->nasc, h->nasc, sizeof(registro->nasc));
    return true;
}

size_t habitante_tamanho_registro(void) {
    return sizeof(HabitanteRegistroInterno);
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
