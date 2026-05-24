#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "../logic/jugador.h"
#include "../logic/bala.h"
#include "../logic/enemigo.h"

void renderizarTodo(SDL_Renderer* renderizador, Jugador* jugador, Bala* bala, BloqueEnemigos* bloque);

#endif