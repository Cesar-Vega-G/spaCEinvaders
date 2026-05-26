#ifndef ENEMIGO_H
#define ENEMIGO_H

#include <SDL2/SDL.h>
#include "../constantes.h"

// Struct de un enemigo individual.
// El servidor maneja su posicion y estado; el cliente solo lo dibuja.
typedef struct {
    SDL_Rect rect;
    int activo;
    int tipo;    // TIPO_CALAMAR=0, TIPO_CANGREJO=1, TIPO_PULPO=2
    int puntos;
} Enemigo;

// Bloque completo de enemigos (matriz de filas x columnas).
// Mas un arreglo de extras creados por el admin con el comando CREAR.
#define MAX_EXTRAS 16

typedef struct {
    Enemigo enemigos[FILAS_ENEMIGOS][COLUMNAS_ENEMIGOS];
    Enemigo extras[MAX_EXTRAS];   // enemigos creados con el comando CREAR
    int     numExtras;
} BloqueEnemigos;

BloqueEnemigos crearBloque();

#endif
