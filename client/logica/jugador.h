#ifndef JUGADOR_H
#define JUGADOR_H

#include <SDL2/SDL.h>
#include "../constantes.h"

// Struct del jugador — solo datos visuales.
// La posicion, vidas y puntaje los actualiza el parser con lo que manda el servidor.
typedef struct {
    SDL_Rect rect;
    int velocidad;
    int vidas;
    int puntaje;
    int activo;  // 1 = el servidor confirmo que este jugador existe, 0 = no conectado aun
} Jugador;

Jugador crearJugador();

#endif
