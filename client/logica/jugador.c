/**
 * LÓGICA CLIENTE — Jugador.
 *
 * El cliente no tiene física propia: el servidor calcula posición, vidas y
 * puntaje y los envía cada frame.  crearJugador() solo inicializa el struct
 * con valores por defecto; el parser.c sobreescribe los campos en el bucle
 * principal con los datos reales del servidor.
 */
#include "jugador.h"

Jugador crearJugador() {
    Jugador j;
    j.rect.x    = INICIO_X_JUGADOR;
    j.rect.y    = INICIO_Y_JUGADOR;
    j.rect.w    = ANCHO_JUGADOR;
    j.rect.h    = ALTO_JUGADOR;
    j.velocidad = VELOCIDAD_JUGADOR;
    j.vidas     = 3;
    j.puntaje   = 0;
    /* activo=0: el jugador no se renderiza hasta que el servidor confirme
     * su existencia con la primera línea "JUGADOR id x y vidas puntaje". */
    j.activo    = 0;
    return j;
}
