#include "render.h"
#include "../logic/bala.h"
#include "../logic/bala.h"
// ── RENDERIZAR TODO ─────────────────────────────────
void renderizarTodo(SDL_Renderer* renderizador, Jugador* jugador, Bala* bala) {

    // Fondo negro
    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255);
    SDL_RenderClear(renderizador);

    // Jugador en blanco
    SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 255);
    SDL_RenderFillRect(renderizador, &jugador->rect);

    //Bala
    // Dibujar bala si está activa
    if (bala->activa == 1) {
    SDL_SetRenderDrawColor(renderizador, 255, 255, 0, 255);
    SDL_RenderFillRect(renderizador, &bala->rect);
    }

    // Mostrar en pantalla
    SDL_RenderPresent(renderizador);
}