#ifndef RENDER_H
#define RENDER_H
#include "../logic/bala.h"

#include <SDL2/SDL.h>
#include "../logic/jugador.h"

// ── FUNCIONES ───────────────────────────────────────
void renderizarTodo(SDL_Renderer* renderizador, Jugador* jugador, Bala* bala);

#endif