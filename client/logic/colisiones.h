#ifndef COLISIONES_H
#define COLISIONES_H

#include <SDL2/SDL.h>
#include "bala.h"
#include "enemigo.h"
#include "ovni.h"

void verificarColisionesBalaEnemigos(Bala* bala, BloqueEnemigos* bloque, int* puntaje);
void verificarColisionBalaOvni(Bala* bala, Ovni* ovni, int* puntaje);

#endif