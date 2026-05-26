#include "enemigo.h"

// Inicializa el bloque con tipos y posiciones por defecto.
// El servidor sobrescribe posiciones y estado via parser en el game loop.
BloqueEnemigos crearBloque() {
    BloqueEnemigos bloque;

    // tipo_por_fila: calamar(rojo), cangrejo(verde)x2, pulpo(azul)x2
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

    // Inicializar arreglo de extras (vacios al inicio)
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
