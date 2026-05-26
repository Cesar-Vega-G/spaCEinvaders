#include "bala.h"

// Inicializa el struct de la bala. El estado real viene del servidor.
Bala crearBala() {
    Bala b;
    b.rect.x  = 0;
    b.rect.y  = 0;
    b.rect.w  = 5;
    b.rect.h  = 15;
    b.velocidad = 12;
    b.activa  = 0;
    return b;
}
