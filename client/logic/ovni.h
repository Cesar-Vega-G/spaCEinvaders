#ifndef OVNI_H
#define OVNI_H

#include <SDL2/SDL.h>
#include "../constantes.h"

typedef struct {
    SDL_Rect rect;
    int activo;
    int puntos;
    int velocidad;
    int contador;
    int intervalo;    
} Ovni;

Ovni crearOvni();
void aparecerOvni(Ovni* ovni);
void actualizarOvni(Ovni* ovni);

#endif