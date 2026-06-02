#ifndef BALA_ENEMIGA_H
#define BALA_ENEMIGA_H

#include <SDL2/SDL.h>
#include "../constantes.h"

// Struct de una bala disparada por un enemigo. Se mueve hacia abajo.
// Estado real (posicion, activa) lo manda el servidor.
typedef struct {
    SDL_Rect rect;
    int activa;
} BalaEnemiga;

BalaEnemiga crearBalaEnemiga();

#endif
