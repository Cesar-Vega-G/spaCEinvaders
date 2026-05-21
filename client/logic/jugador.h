#ifndef JUGADOR_H
#define JUGADOR_H
#include "bala.h"   


#include <SDL2/SDL.h>
#include "../constantes.h"

// ── STRUCT DEL JUGADOR ──────────────────────────────
typedef struct {
    SDL_Rect rect;
    int velocidad;
} Jugador;

// ── FUNCIONES ───────────────────────────────────────
Jugador crearJugador();

void moverJugador(Jugador* jugador, Bala* bala, SDL_Event* evento, int* jugando);

#endif