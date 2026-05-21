#include "bala.h"

// ── CREAR BALA ──────────────────────────────────────
Bala crearBala() {
    Bala b;
    b.rect.x = 0;
    b.rect.y = 0;
    b.rect.w = 5;
    b.rect.h = 15;
    b.velocidad = 8;
    b.activa = 0;
    return b;
}

// ── DISPARAR BALA ───────────────────────────────────
void dispararBala(Bala* bala, int x, int y) {
    if (bala->activa == 0) {
        bala->rect.x = x;
        bala->rect.y = y;
        bala->activa = 1;
    }
}

// ── ACTUALIZAR BALA ─────────────────────────────────
void actualizarBala(Bala* bala) {
    if (bala->activa == 1) {
        bala->rect.y -= bala->velocidad;

        if (bala->rect.y < 0) {
            bala->activa = 0;
        }
    }
}