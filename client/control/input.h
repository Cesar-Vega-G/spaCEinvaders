#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>
#include "../logica/jugador.h"
#include "../comunicacion/socket_cliente.h"

// Capa de Control: captura eventos del teclado y los envia al servidor.
// El cliente NO mueve nada localmente; solo traduce teclas a mensajes.
void procesarInput(SDL_Event* evento, int* jugando, Conexion* conexion);

#endif
