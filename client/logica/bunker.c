#include "bunker.h"

// Inicializa un bunker vacio. El servidor llena posicion y bloques.
Bunker crearBunker() {
    Bunker b;
    b.x        = 0;
    b.y        = 0;
    b.filas    = BUNKER_FILAS;
    b.columnas = BUNKER_COLUMNAS;
    b.lado     = BUNKER_LADO_BLOQUE;
    for (int f = 0; f < BUNKER_FILAS; f++)
        for (int c = 0; c < BUNKER_COLUMNAS; c++)
            b.bloques[f][c] = 0;
    return b;
}
