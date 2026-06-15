/**
 * LÓGICA CLIENTE — Bala enemiga.
 *
 * Pool de MAX_BALAS_ENEMIGAS instancias inicializadas inactivas.
 * El servidor mantiene el mismo pool y envía el estado (id, x, y, activa)
 * de cada bala en la serialización; parser.c actualiza los structs aquí.
 */
#include "bala_enemiga.h"

BalaEnemiga crearBalaEnemiga() {
    BalaEnemiga b;
    b.rect.x = 0;
    b.rect.y = 0;
    b.rect.w = ANCHO_BALA_ENEMIGA;
    b.rect.h = ALTO_BALA_ENEMIGA;
    b.activa = 0;
    return b;
}
