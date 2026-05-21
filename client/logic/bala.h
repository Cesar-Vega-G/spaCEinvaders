#ifndef BALA_H
#define BALA_H

#include <SDL2/SDL.h>
#include "../constantes.h"

// ── STRUCT ──────────────────────────────────────────
typedef struct {
    SDL_Rect rect;
    int velocidad;
    int activa;
} Bala;

// ── FUNCIONES ───────────────────────────────────────
Bala crearBala();
void dispararBala(Bala* bala, int x, int y);
void actualizarBala(Bala* bala);

#endif