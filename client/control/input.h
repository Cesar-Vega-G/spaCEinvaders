#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>
#include "../logica/jugador.h"
#include "../comunicacion/socket_cliente.h"

// Capa de Control: captura eventos del teclado y de la Pico.
// El cliente NO mueve nada localmente; solo traduce entradas a mensajes.
void inicializarControlPico();
void cerrarControlPico();
void procesarInput(SDL_Event *evento, int *jugando, Conexion *conexion);

#endif
