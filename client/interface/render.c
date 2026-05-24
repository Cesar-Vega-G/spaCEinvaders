#include "render.h"
#include "../logic/bala.h"
#include "../logic/bala.h"
// ── RENDERIZAR TODO ─────────────────────────────────
void renderizarTodo(SDL_Renderer* renderizador, Jugador* jugador, Bala* bala, BloqueEnemigos* bloque) {

    // Fondo negro
    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255);
    SDL_RenderClear(renderizador);

    // Jugador en blanco
    SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 255);
    SDL_RenderFillRect(renderizador, &jugador->rect);

    // Bala
    if (bala->activa == 1) {
        SDL_SetRenderDrawColor(renderizador, 255, 255, 0, 255);
        SDL_RenderFillRect(renderizador, &bala->rect);
    }

    // Enemigos — cada tipo con su color
    for (int fila = 0; fila < FILAS_ENEMIGOS; fila++) {
        for (int col = 0; col < COLUMNAS_ENEMIGOS; col++) {
            if (bloque->enemigos[fila][col].activo) {
                switch (bloque->enemigos[fila][col].tipo) {
                    case TIPO_CALAMAR:        SDL_SetRenderDrawColor(renderizador, 255, 0,   0,   255); break;
                    case TIPO_CANGREJO:       SDL_SetRenderDrawColor(renderizador, 0,   255, 0,   255); break;
                    case TIPO_PULPO:          SDL_SetRenderDrawColor(renderizador, 0,   0,   255, 255); break;
                    case TIPO_EXTRATERRESTRE: SDL_SetRenderDrawColor(renderizador, 255, 0,   255, 255); break;
                }
                SDL_RenderFillRect(renderizador, &bloque->enemigos[fila][col].rect);
            }
        }
    }

    // Mostrar en pantalla
    SDL_RenderPresent(renderizador);
}