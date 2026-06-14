#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "../logica/jugador.h"
#include "../logica/bala.h"
#include "../logica/bala_enemiga.h"
#include "../logica/enemigo.h"
#include "../logica/ovni.h"
#include "../logica/bunker.h"

int  cargarTexturas(SDL_Renderer* renderizador);
void liberarTexturas(void);

void renderizarTodo(SDL_Renderer* renderizador,
                    Jugador jugadores[],
                    Bala balas[],
                    BalaEnemiga balasEnemigas[],
                    BloqueEnemigos* bloque,
                    Ovni* ovni,
                    Bunker bunkers[]);

#endif
