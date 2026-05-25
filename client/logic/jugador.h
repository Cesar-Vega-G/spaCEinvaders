#ifndef JUGADOR_H
#define JUGADOR_H

#include <SDL2/SDL.h>
#include "../constantes.h"
#include "bala.h"
#include "../comunicacion/socket_cliente.h"

typedef struct {
    SDL_Rect rect;
    int velocidad;
} Jugador;

Jugador crearJugador();
void moverJugador(Jugador* jugador, Bala* bala, SDL_Event* evento, int* jugando, Conexion* conexion);

#endif