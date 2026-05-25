#include "enemigo.h"

// ── CREAR BLOQUE ────────────────────────────────────
BloqueEnemigos crearBloque() {
    //inicializamos el typedef bloqueenemigos

    BloqueEnemigos bloque;
    bloque.direccion = 1;
    bloque.velocidad = VELOCIDAD_ENEMIGOS;
    bloque.contador  = 0;        // ← agregar
    bloque.delay     = 20;
    
    int tipo_por_fila[]   = {0, 1, 1, 2, 2};  // calamar, cangrejo, cangrejo, pulpo, pulpo
    int puntos_por_fila[] = {40, 30, 30, 20, 20};

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


    return bloque;
}

// ── ACTUALIZAR BLOQUE ───────────────────────────────
// tipo bloque enemigo el bloque de name bloque que creamos
void actualizarBloque(BloqueEnemigos* bloque) {

    // Contar frames
    bloque->contador++;
    if (bloque->contador < bloque->delay) return;
    bloque->contador = 0;

    int tocar_borde = 0;

    for (int fila = 0; fila < FILAS_ENEMIGOS; fila++) {
        for (int col = 0; col < COLUMNAS_ENEMIGOS; col++) {
            if (bloque->enemigos[fila][col].activo) {
                bloque->enemigos[fila][col].rect.x += bloque->velocidad * bloque->direccion;

                if (bloque->enemigos[fila][col].rect.x <= 0 ||
                    bloque->enemigos[fila][col].rect.x + ANCHO_ENEMIGO >= ANCHO_PANTALLA) {
                    tocar_borde = 1;
                }
            }
        }
    }

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