#ifndef ENEMIGO_H
#define ENEMIGO_H

#include <SDL2/SDL.h>
#include "../constantes.h"

// ── TIPOS DE ENEMIGO ────────────────────────────────
#define TIPO_CALAMAR        0
#define TIPO_CANGREJO       1
#define TIPO_PULPO          2
#define TIPO_EXTRATERRESTRE 3

#define FILAS_ENEMIGOS      4
#define COLUMNAS_ENEMIGOS   11
#define TOTAL_ENEMIGOS      44

#define ANCHO_ENEMIGO       50
#define ALTO_ENEMIGO        50
#define ESPACIO_ENEMIGO_X   20
#define ESPACIO_ENEMIGO_Y   40
#define BAJADA_ENEMIGOS     40

// ── STRUCT ENEMIGO INDIVIDUAL ───────────────────────
typedef struct {
    SDL_Rect rect;
    int activo;
    int tipo;
    int puntos;
} Enemigo;

// ── STRUCT BLOQUE COMPLETO ──────────────────────────
typedef struct {
    Enemigo enemigos[FILAS_ENEMIGOS][COLUMNAS_ENEMIGOS]; 
                                                            //([typedf Enemigo][][][][][][][][][][]
                                                            //[][][][][][][][][][][]
                                                            //[][][][][][][][][][][]
                                                            //[][][][][][][][][][][]
                                                            // direccion y velovidad ) Bloque enemigos
    int direccion;    //  1 = derecha, -1 = izquierda
    int velocidad;
} BloqueEnemigos;

// ── FUNCIONES ───────────────────────────────────────
BloqueEnemigos crearBloque();
void actualizarBloque(BloqueEnemigos* bloque);

#endif