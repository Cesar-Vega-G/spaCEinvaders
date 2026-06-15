/**
 * LÓGICA CLIENTE — Bala del jugador.
 *
 * El cliente no mueve la bala: el servidor la actualiza y serializa su
 * posición cada frame.  Este struct solo almacena el estado más reciente
 * recibido del servidor para que render.c pueda dibujarlo.
 */
#include "bala.h"

Bala crearBala() {
    Bala b;
    b.rect.x    = 0;
    b.rect.y    = 0;
    b.rect.w    = 5;
    b.rect.h    = 15;
    b.velocidad = 20; /* no se usa en el cliente; el servidor controla el movimiento */
    b.activa    = 0;
    return b;
}
