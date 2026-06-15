/**
 * LÓGICA CLIENTE — Bloque de enemigos.
 *
 * El cliente inicializa la cuadrícula con valores por defecto.  El servidor
 * es quien mueve el bloque y destruye enemigos; parser.c sobreescribe las
 * posiciones y el flag activo de cada celda con los datos recibidos en cada
 * frame.  Los "extras" son los enemigos creados con el comando admin CREAR.
 */
#include "enemigo.h"

BloqueEnemigos crearBloque() {
    BloqueEnemigos bloque;

    /* Distribución clásica de Space Invaders: calamar arriba, pulpo abajo. */
    int tipo_por_fila[]   = {TIPO_CALAMAR, TIPO_CANGREJO, TIPO_CANGREJO, TIPO_PULPO, TIPO_PULPO};
    int puntos_por_fila[] = {10, 20, 20, 40, 40};

    for (int fila = 0; fila < FILAS_ENEMIGOS; fila++) {
        for (int col = 0; col < COLUMNAS_ENEMIGOS; col++) {
            bloque.enemigos[fila][col].activo = 1;
            bloque.enemigos[fila][col].tipo   = tipo_por_fila[fila];
            bloque.enemigos[fila][col].puntos = puntos_por_fila[fila];
            bloque.enemigos[fila][col].rect.x = INICIO_ENEMIGOS_X + col * (ANCHO_ENEMIGO + ESPACIO_ENTRE_ENEMIGOS);
            bloque.enemigos[fila][col].rect.y = INICIO_ENEMIGOS_Y + fila * (ALTO_ENEMIGO + ESPACIO_ENTRE_ENEMIGOS);
            bloque.enemigos[fila][col].rect.w = ANCHO_ENEMIGO;
            bloque.enemigos[fila][col].rect.h = ALTO_ENEMIGO;
        }
    }

    /* Arreglo de extras vacío al inicio; parser.c los rellena al recibir "EXTRA". */
    bloque.numExtras = 0;
    for (int i = 0; i < MAX_EXTRAS; i++) {
        bloque.extras[i].activo = 0;
        bloque.extras[i].rect.x = 0;
        bloque.extras[i].rect.y = 0;
        bloque.extras[i].rect.w = 50;
        bloque.extras[i].rect.h = 50;
        bloque.extras[i].tipo   = TIPO_PULPO;
        bloque.extras[i].puntos = 40;
    }

    return bloque;
}
