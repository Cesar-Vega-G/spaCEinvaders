/**
 * LÓGICA CLIENTE — Bunker (escudo).
 *
 * Se inicializa con todos los bloques destruidos (0).  El servidor envía el
 * bitmap completo de cada bunker en cada frame; parser.c lo sobreescribe aquí.
 * El cliente nunca calcula colisiones: solo renderiza los bloques vivos.
 */
#include "bunker.h"

Bunker crearBunker() {
    Bunker b;
    b.x        = 0;
    b.y        = 0;
    b.filas    = BUNKER_FILAS;
    b.columnas = BUNKER_COLUMNAS;
    b.lado     = BUNKER_LADO_BLOQUE;
    /* Iniciar todo destruido evita dibujar bloques fantasma antes de recibir
     * el primer estado del servidor. */
    for (int f = 0; f < BUNKER_FILAS; f++)
        for (int c = 0; c < BUNKER_COLUMNAS; c++)
            b.bloques[f][c] = 0;
    return b;
}
