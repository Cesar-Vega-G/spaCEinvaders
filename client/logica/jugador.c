#include "jugador.h"

// Inicializa el struct del jugador con valores por defecto.
// La posicion real viene del servidor via parser.
Jugador crearJugador() {
    Jugador j;
    j.rect.x    = INICIO_X_JUGADOR;
    j.rect.y    = INICIO_Y_JUGADOR;
    j.rect.w    = ANCHO_JUGADOR;
    j.rect.h    = ALTO_JUGADOR;
    j.velocidad = VELOCIDAD_JUGADOR;
    j.vidas     = 3;
    j.puntaje   = 0;
    j.activo    = 0;  // el servidor lo activa cuando ese jugador conecta
    return j;
}
