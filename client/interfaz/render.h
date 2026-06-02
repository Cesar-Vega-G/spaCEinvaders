#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "../logica/jugador.h"
#include "../logica/bala.h"
#include "../logica/bala_enemiga.h"
#include "../logica/enemigo.h"
#include "../logica/ovni.h"
#include "../logica/bunker.h"

// jugadores[0] = blanco, jugadores[1] = cyan.
// Cada ventana ve ambos canones; solo controla el suyo (conexion->idJugador).
void renderizarTodo(SDL_Renderer* renderizador,
                    Jugador jugadores[],
                    Bala balas[],
                    BalaEnemiga balasEnemigas[],
                    BloqueEnemigos* bloque,
                    Ovni* ovni,
                    Bunker bunkers[]);

#endif
