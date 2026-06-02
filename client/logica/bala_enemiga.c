#include "bala_enemiga.h"

// Inicializa el struct. El estado real viene del servidor.
BalaEnemiga crearBalaEnemiga() {
    BalaEnemiga b;
    b.rect.x = 0;
    b.rect.y = 0;
    b.rect.w = ANCHO_BALA_ENEMIGA;
    b.rect.h = ALTO_BALA_ENEMIGA;
    b.activa = 0;
    return b;
}
