#ifndef ENEMIGO_H
#define ENEMIGO_H

#include <SDL2/SDL.h>
#include "../constantes.h"



// ── STRUCT ENEMIGO INDIVIDUAL ───────────────────────
typedef struct {
    SDL_Rect rect;
    int activo;
    int tipo;
    int puntos;
    
} Enemigo;

// ── STRUCT BLOQUE COMPLETO ──────────────────────────
//([typedf Enemigo][][][][][][][][][][]
//[][][][][][][][][][][]
//[][][][][][][][][][][]
//[][][][][][][][][][][]
// direccion y velovidad ) Bloque enemigos
typedef struct {
    Enemigo enemigos[FILAS_ENEMIGOS][COLUMNAS_ENEMIGOS];                                                          
    int direccion;    //  1 = derecha, -1 = izquierda
    int velocidad;
    int contador;       
    int delay;
} BloqueEnemigos;

// ── FUNCIONES ───────────────────────────────────────
BloqueEnemigos crearBloque();
void actualizarBloque(BloqueEnemigos* bloque);

#endif