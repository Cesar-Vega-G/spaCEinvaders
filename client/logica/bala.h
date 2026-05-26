#ifndef BALA_H
#define BALA_H

#include <SDL2/SDL.h>
#include "../constantes.h"

// Struct de la bala del jugador.
// El servidor decide cuando esta activa y su posicion.
typedef struct {
    SDL_Rect rect;
    int velocidad;
    int activa;
} Bala;

Bala crearBala();

#endif
