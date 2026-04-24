#include "../include/quadra.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char cep[HE_TAMANHO_CHAVE_MAX + 1];
    double x;
    double y;
    double w;
    double h;
    char sw[QUADRA_TAMANHO_ESPESSURA_MAX + 1];
    char cfill[QUADRA_TAMANHO_COR_MAX + 1];
    char cstrk[QUADRA_TAMANHO_COR_MAX + 1];
} QuadraImpl;

typedef struct HE_PACKED {
    char cep[HE_TAMANHO_CHAVE_MAX + 1];
    double x;
    double y;
    double w;
    double h;
    char sw[QUADRA_TAMANHO_ESPESSURA_MAX + 1];
    char cfill[QUADRA_TAMANHO_COR_MAX + 1];
    char cstrk[QUADRA_TAMANHO_COR_MAX + 1];
} QuadraRegistroInterno;

static QuadraImpl *quadra_impl(Quadra quadra) {
    return (QuadraImpl *)quadra;
}

static bool texto_valido(const char *texto, size_t tamanho_maximo) {
    return texto != NULL && texto[0] != '\0' && strlen(texto) <= tamanho_maximo;
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
              (c >= 'a' && c <= 'z') || c == '-')) {
            return false;
        }
    }

    return true;
}

static bool medida_valida(double valor) {
    return isfinite(valor) != 0;
}

static bool dimensao_valida(double valor) {
    return medida_valida(valor) && valor > 0.0;
}

static bool copiar_texto(char *destino, size_t tamanho_destino, const char *origem) {
    if (destino == NULL || origem == NULL || strlen(origem) >= tamanho_destino) {
        return false;
    }

    memcpy(destino, origem, strlen(origem) + 1u);
    return true;
}

Quadra quadra_criar(const char *cep, double x, double y, double w, double h,
                    const char *sw, const char *cfill, const char *cstrk) {
    QuadraImpl *quadra;

    if (!cep_valido(cep) || !medida_valida(x) || !medida_valida(y) ||
        !dimensao_valida(w) || !dimensao_valida(h) ||
        !texto_valido(sw, QUADRA_TAMANHO_ESPESSURA_MAX) ||
        !texto_valido(cfill, QUADRA_TAMANHO_COR_MAX) ||
        !texto_valido(cstrk, QUADRA_TAMANHO_COR_MAX)) {
        return NULL;
    }

    quadra = (QuadraImpl *)calloc(1, sizeof(QuadraImpl));
    if (quadra == NULL) {
        return NULL;
    }

    copiar_texto(quadra->cep, sizeof(quadra->cep), cep);
    quadra->x = x;
    quadra->y = y;
    quadra->w = w;
    quadra->h = h;
    copiar_texto(quadra->sw, sizeof(quadra->sw), sw);
    copiar_texto(quadra->cfill, sizeof(quadra->cfill), cfill);
    copiar_texto(quadra->cstrk, sizeof(quadra->cstrk), cstrk);
    return quadra;
}

Quadra quadra_criar_de_bytes(const void *dados_registro, size_t tamanho_registro) {
    const QuadraRegistroInterno *registro;

    if (dados_registro == NULL || tamanho_registro != sizeof(QuadraRegistroInterno)) {
        return NULL;
    }

    registro = (const QuadraRegistroInterno *)dados_registro;
    return quadra_criar(registro->cep, registro->x, registro->y, registro->w, registro->h,
                        registro->sw, registro->cfill, registro->cstrk);
}

void quadra_destruir(Quadra quadra) {
    free(quadra_impl(quadra));
}

bool quadra_definir_estilo(Quadra quadra, const char *sw, const char *cfill,
                           const char *cstrk) {
    QuadraImpl *q = quadra_impl(quadra);

    if (q == NULL || !texto_valido(sw, QUADRA_TAMANHO_ESPESSURA_MAX) ||
        !texto_valido(cfill, QUADRA_TAMANHO_COR_MAX) ||
        !texto_valido(cstrk, QUADRA_TAMANHO_COR_MAX)) {
        return false;
    }

    copiar_texto(q->sw, sizeof(q->sw), sw);
    copiar_texto(q->cfill, sizeof(q->cfill), cfill);
    copiar_texto(q->cstrk, sizeof(q->cstrk), cstrk);
    return true;
}

bool quadra_escrever_registro(Quadra quadra, void *registro_out, size_t tamanho_registro) {
    QuadraImpl *q = quadra_impl(quadra);
    QuadraRegistroInterno *registro;

    if (q == NULL || registro_out == NULL ||
        tamanho_registro != sizeof(QuadraRegistroInterno)) {
        return false;
    }

    registro = (QuadraRegistroInterno *)registro_out;
    memset(registro, 0, sizeof(*registro));
    memcpy(registro->cep, q->cep, sizeof(registro->cep));
    registro->x = q->x;
    registro->y = q->y;
    registro->w = q->w;
    registro->h = q->h;
    memcpy(registro->sw, q->sw, sizeof(registro->sw));
    memcpy(registro->cfill, q->cfill, sizeof(registro->cfill));
    memcpy(registro->cstrk, q->cstrk, sizeof(registro->cstrk));
    return true;
}

size_t quadra_tamanho_registro(void) {
    return sizeof(QuadraRegistroInterno);
}

const char *quadra_obter_cep(Quadra quadra) {
    QuadraImpl *q = quadra_impl(quadra);
    return q == NULL ? NULL : q->cep;
}

double quadra_obter_x(Quadra quadra) {
    QuadraImpl *q = quadra_impl(quadra);
    return q == NULL ? 0.0 : q->x;
}

double quadra_obter_y(Quadra quadra) {
    QuadraImpl *q = quadra_impl(quadra);
    return q == NULL ? 0.0 : q->y;
}

double quadra_obter_w(Quadra quadra) {
    QuadraImpl *q = quadra_impl(quadra);
    return q == NULL ? 0.0 : q->w;
}

double quadra_obter_h(Quadra quadra) {
    QuadraImpl *q = quadra_impl(quadra);
    return q == NULL ? 0.0 : q->h;
}

const char *quadra_obter_sw(Quadra quadra) {
    QuadraImpl *q = quadra_impl(quadra);
    return q == NULL ? NULL : q->sw;
}

const char *quadra_obter_cfill(Quadra quadra) {
    QuadraImpl *q = quadra_impl(quadra);
    return q == NULL ? NULL : q->cfill;
}

const char *quadra_obter_cstrk(Quadra quadra) {
    QuadraImpl *q = quadra_impl(quadra);
    return q == NULL ? NULL : q->cstrk;
}
