#ifndef OVNI_H
#define OVNI_H

#include <SDL2/SDL.h>
#include "../constantes.h"

// Struct del OVNI. El servidor controla cuando aparece y se mueve.
typedef struct {
    SDL_Rect rect;
    int activo;
    int puntos;
} Ovni;

Ovni crearOvni();

#endif
