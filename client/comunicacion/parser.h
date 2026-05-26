#ifndef PARSER_H
#define PARSER_H

#include "../logica/ovni.h"
#include "../logica/enemigo.h"
#include "../logica/jugador.h"
#include "../logica/bala.h"
#include "socket_cliente.h"

// Parsea el estado que manda el servidor cada frame y actualiza las entidades.
// jugadores[2] y balas[2] corresponden a los 2 posibles jugadores de la partida.
// conexion->idJugador indica cual de los 2 es el jugador local.
void parsearEstado(const char* estado,
                   Ovni* ovni,
                   BloqueEnemigos* bloque,
                   Jugador jugadores[],
                   Bala balas[],
                   int* jugando,
                   Conexion* conexion);

#endif
