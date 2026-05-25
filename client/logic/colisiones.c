#include "colisiones.h"

// ── BALA vs ENEMIGOS ────────────────────────────────
void verificarColisionesBalaEnemigos(Bala* bala, BloqueEnemigos* bloque, int* puntaje) {
    if (bala->activa == 0) return;

    for (int fila = 0; fila < FILAS_ENEMIGOS; fila++) {
        for (int col = 0; col < COLUMNAS_ENEMIGOS; col++) {
            Enemigo* enemigoActual = &bloque->enemigos[fila][col];

            if (enemigoActual->activo && SDL_HasIntersection(&bala->rect, &enemigoActual->rect)) {
                enemigoActual->activo = 0;
                bala->activa = 0;
                *puntaje += enemigoActual->puntos;
                return;
            }
        }
    }
}

// ── BALA vs OVNI ────────────────────────────────────
void verificarColisionBalaOvni(Bala* bala, Ovni* ovni, int* puntaje) {
    if (bala->activa == 0 || ovni->activo == 0) return;

    if (SDL_HasIntersection(&bala->rect, &ovni->rect)) {
        ovni->activo = 0;
        bala->activa = 0;
        *puntaje += ovni->puntos;
    }
}