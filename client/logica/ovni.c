/**
 * LÓGICA CLIENTE — OVNI.
 *
 * El OVNI se inicializa inactivo.  El servidor decide cuándo aparece y por
 * dónde sale; parser.c actualiza la posición y el flag activo cada frame.
 */
#include "ovni.h"

Ovni crearOvni() {
    Ovni o;
    o.rect.x = 0;
    o.rect.y = 30; /* franja superior, igual que Ovni.Y_FIJO en el servidor */
    o.rect.w = 60;
    o.rect.h = 30;
    o.activo = 0;
    o.puntos = 0;
    return o;
}
