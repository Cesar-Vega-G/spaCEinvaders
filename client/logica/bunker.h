#ifndef BUNKER_H
#define BUNKER_H

#include <SDL2/SDL.h>
#include "../constantes.h"

// Struct de un bunker. Tiene una grilla de bloques pequeños.
// Cada bloque es 1 = vivo, 0 = destruido.
// El servidor manda la grilla cada frame.
typedef struct {
    int x;
    int y;
    int filas;          // BUNKER_FILAS
    int columnas;       // BUNKER_COLUMNAS
    int lado;           // BUNKER_LADO_BLOQUE
    int bloques[BUNKER_FILAS][BUNKER_COLUMNAS];
} Bunker;

Bunker crearBunker();

#endif
