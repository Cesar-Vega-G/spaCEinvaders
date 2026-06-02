#include "render.h"
#include <stdio.h>

// Fuente de pixeles 3x5 para digitos 0-9
static const int FUENTE[10][5][3] = {
    {{1,1,1},{1,0,1},{1,0,1},{1,0,1},{1,1,1}}, // 0
    {{0,1,0},{0,1,0},{0,1,0},{0,1,0},{0,1,0}}, // 1
    {{1,1,1},{0,0,1},{1,1,1},{1,0,0},{1,1,1}}, // 2
    {{1,1,1},{0,0,1},{0,1,1},{0,0,1},{1,1,1}}, // 3
    {{1,0,1},{1,0,1},{1,1,1},{0,0,1},{0,0,1}}, // 4
    {{1,1,1},{1,0,0},{1,1,1},{0,0,1},{1,1,1}}, // 5
    {{1,1,1},{1,0,0},{1,1,1},{1,0,1},{1,1,1}}, // 6
    {{1,1,1},{0,0,1},{0,0,1},{0,0,1},{0,0,1}}, // 7
    {{1,1,1},{1,0,1},{1,1,1},{1,0,1},{1,1,1}}, // 8
    {{1,1,1},{1,0,1},{1,1,1},{0,0,1},{1,1,1}}, // 9
};

static void dibujarNumero(SDL_Renderer* r, int numero, int x, int y, int escala) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", numero);
    for (int i = 0; buf[i] != '\0'; i++) {
        int d  = buf[i] - '0';
        int ox = x + i * (escala * 4);
        for (int fila = 0; fila < 5; fila++)
            for (int col = 0; col < 3; col++)
                if (FUENTE[d][fila][col]) {
                    SDL_Rect px = {ox + col*escala, y + fila*escala, escala, escala};
                    SDL_RenderFillRect(r, &px);
                }
    }
}

void renderizarTodo(SDL_Renderer* renderizador,
                    Jugador jugadores[],
                    Bala balas[],
                    BalaEnemiga balasEnemigas[],
                    BloqueEnemigos* bloque,
                    Ovni* ovni,
                    Bunker bunkers[]) {

    // Fondo negro
    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255);
    SDL_RenderClear(renderizador);

    // Canon jugador 0 (blanco)
    if (jugadores[0].activo) {
        SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 255);
        SDL_RenderFillRect(renderizador, &jugadores[0].rect);
    }

    // Canon jugador 1 (cyan — el segundo jugador en la misma partida)
    if (jugadores[1].activo) {
        SDL_SetRenderDrawColor(renderizador, 0, 255, 255, 255);
        SDL_RenderFillRect(renderizador, &jugadores[1].rect);
    }

    // Bala jugador 0 (amarilla)
    if (balas[0].activa) {
        SDL_SetRenderDrawColor(renderizador, 255, 255, 0, 255);
        SDL_RenderFillRect(renderizador, &balas[0].rect);
    }

    // Bala jugador 1 (naranja)
    if (balas[1].activa) {
        SDL_SetRenderDrawColor(renderizador, 255, 165, 0, 255);
        SDL_RenderFillRect(renderizador, &balas[1].rect);
    }

    // Balas enemigas (rojas)
    SDL_SetRenderDrawColor(renderizador, 255, 60, 60, 255);
    for (int i = 0; i < MAX_BALAS_ENEMIGAS; i++) {
        if (balasEnemigas[i].activa)
            SDL_RenderFillRect(renderizador, &balasEnemigas[i].rect);
    }

    // Enemigos de la grilla principal
    for (int f = 0; f < FILAS_ENEMIGOS; f++) {
        for (int c = 0; c < COLUMNAS_ENEMIGOS; c++) {
            if (!bloque->enemigos[f][c].activo) continue;
            switch (bloque->enemigos[f][c].tipo) {
                case TIPO_CALAMAR:  SDL_SetRenderDrawColor(renderizador, 255, 0,   0,   255); break;
                case TIPO_CANGREJO: SDL_SetRenderDrawColor(renderizador, 0,   255, 0,   255); break;
                case TIPO_PULPO:    SDL_SetRenderDrawColor(renderizador, 0,   0,   255, 255); break;
                default:            SDL_SetRenderDrawColor(renderizador, 200, 200, 200, 255); break;
            }
            SDL_RenderFillRect(renderizador, &bloque->enemigos[f][c].rect);
        }
    }

    // Enemigos extra (creados por el admin con CREAR)
    for (int i = 0; i < bloque->numExtras; i++) {
        if (!bloque->extras[i].activo) continue;
        switch (bloque->extras[i].tipo) {
            case TIPO_CALAMAR:  SDL_SetRenderDrawColor(renderizador, 255, 0,   0,   255); break;
            case TIPO_CANGREJO: SDL_SetRenderDrawColor(renderizador, 0,   255, 0,   255); break;
            case TIPO_PULPO:    SDL_SetRenderDrawColor(renderizador, 0,   0,   255, 255); break;
            default:            SDL_SetRenderDrawColor(renderizador, 200, 200, 200, 255); break;
        }
        SDL_RenderFillRect(renderizador, &bloque->extras[i].rect);
    }

    // Bunkers (verdes, grilla de bloques destructibles)
    SDL_SetRenderDrawColor(renderizador, 0, 220, 80, 255);
    for (int i = 0; i < NUM_BUNKERS; i++) {
        for (int f = 0; f < bunkers[i].filas && f < BUNKER_FILAS; f++) {
            for (int c = 0; c < bunkers[i].columnas && c < BUNKER_COLUMNAS; c++) {
                if (!bunkers[i].bloques[f][c]) continue;
                SDL_Rect r = {
                    bunkers[i].x + c * bunkers[i].lado,
                    bunkers[i].y + f * bunkers[i].lado,
                    bunkers[i].lado,
                    bunkers[i].lado
                };
                SDL_RenderFillRect(renderizador, &r);
            }
        }
    }

    // OVNI (rojo brillante)
    if (ovni->activo) {
        SDL_SetRenderDrawColor(renderizador, 255, 50, 50, 255);
        SDL_RenderFillRect(renderizador, &ovni->rect);
    }

    // HUD jugador 0: puntaje en blanco arriba izquierda
    SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 255);
    dibujarNumero(renderizador, jugadores[0].puntaje, 10, 10, 3);

    // HUD jugador 0: vidas como cuadraditos blancos
    for (int v = 0; v < jugadores[0].vidas; v++) {
        SDL_Rect vida = {10 + v * 30, 55, 20, 20};
        SDL_RenderFillRect(renderizador, &vida);
    }

    // HUD jugador 1: puntaje en cyan arriba derecha (si existe)
    if (jugadores[1].activo) {
        SDL_SetRenderDrawColor(renderizador, 0, 255, 255, 255);
        dibujarNumero(renderizador, jugadores[1].puntaje, 1050, 10, 3);
        for (int v = 0; v < jugadores[1].vidas; v++) {
            SDL_Rect vida = {1050 + v * 30, 55, 20, 20};
            SDL_RenderFillRect(renderizador, &vida);
        }
    }

    SDL_RenderPresent(renderizador);
}
