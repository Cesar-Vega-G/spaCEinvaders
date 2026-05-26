#include "ovni.h"

// Inicializa el struct del OVNI. El servidor decide cuando aparece.
Ovni crearOvni() {
    Ovni o;
    o.rect.x = 0;
    o.rect.y = 30;
    o.rect.w = 60;
    o.rect.h = 30;
    o.activo = 0;
    o.puntos = 0;
    return o;
}
