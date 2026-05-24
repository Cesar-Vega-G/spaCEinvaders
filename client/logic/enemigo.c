#include "enemigo.h"

// ── CREAR BLOQUE ────────────────────────────────────
BloqueEnemigos crearBloque() {
    //inicializamos el typedef bloqueenemigos

    BloqueEnemigos bloque;
    bloque.direccion = 1;
    bloque.velocidad = VELOCIDAD_ENEMIGOS;
    
    int puntos_por_fila[] = {40, 30, 20, 10};
    //un arreglo que define cuántos puntos da cada fila. Fila 0 (calamar) da 40, fila 3 (extraterrestre) da 10.



    for (int fila = 0; fila < FILAS_ENEMIGOS; fila++) {
        for (int col = 0; col < COLUMNAS_ENEMIGOS; col++) {

            bloque.enemigos[fila][col].activo = 1;   
            bloque.enemigos[fila][col].tipo   = fila;
            bloque.enemigos[fila][col].puntos = puntos_por_fila[fila];

            ///Primer ejemplo: fila 0, calamar, pts
            ///

            bloque.enemigos[fila][col].rect.x = INICIO_ENEMIGOS_X + col * ANCHO_ENEMIGO + ESPACIO_ENTRE_ENEMIGOS;
            bloque.enemigos[fila][col].rect.y = INICIO_ENEMIGOS_Y + fila * ALTO_ENEMIGO + ESPACIO_ENTRE_ENEMIGOS;
            bloque.enemigos[fila][col].rect.w = ANCHO_ENEMIGO;
            bloque.enemigos[fila][col].rect.h = ALTO_ENEMIGO;
        }
    }

    return bloque;
}

// ── ACTUALIZAR BLOQUE ───────────────────────────────
// tipo bloque enemigo el bloque de name bloque que creamos
void actualizarBloque(BloqueEnemigos* bloque) {
    int tocar_borde = 0;

    // Mover todos horizontalmente
    for (int fila = 0; fila < FILAS_ENEMIGOS; fila++) {
        for (int col = 0; col < COLUMNAS_ENEMIGOS; col++) {
            if (bloque->enemigos[fila][col].activo) {
                bloque->enemigos[fila][col].rect.x += bloque->velocidad * bloque->direccion;

                // ¿Alguno tocó el borde?
                if (bloque->enemigos[fila][col].rect.x <= 0 ||bloque->enemigos[fila][col].rect.x + ANCHO_ENEMIGO >= ANCHO_PANTALLA) {
                    tocar_borde = 1;
                }
            }
        }
    }

    // Si alguno tocó el borde: bajar y cambiar dirección
    if (tocar_borde) {
        bloque->direccion *= -1;
        for (int fila = 0; fila < FILAS_ENEMIGOS; fila++) {
            for (int col = 0; col < COLUMNAS_ENEMIGOS; col++) {
                if (bloque->enemigos[fila][col].activo) {
                    bloque->enemigos[fila][col].rect.y += BAJADA_ENEMIGOS;
                }
            }
        }
    }
}