#include "ovni.h"

Ovni crearOvni(){
    Ovni OvniVolador;

    OvniVolador.rect.x = ANCHO_PANTALLA;
    OvniVolador.rect.y = 30;
    OvniVolador.rect.w = ANCHO_ENEMIGO;
    OvniVolador.rect.h = ALTO_ENEMIGO;
    OvniVolador.activo = 0;
    OvniVolador.puntos = 300;
    OvniVolador.velocidad = VELOCIDAD_OVNI;
    OvniVolador.contador = 0;
    OvniVolador.intervalo = DELAY_OVNI;

    return OvniVolador;
}

void actualizarOvni(Ovni* OvniVolador) {
    if (OvniVolador->activo == 0) return;

    OvniVolador->contador++;
    if (OvniVolador->contador < OvniVolador->intervalo) return;
    OvniVolador->contador = 0;

    OvniVolador->rect.x -= OvniVolador->velocidad;

    if (OvniVolador->rect.x + ANCHO_ENEMIGO < 0) {
        OvniVolador->activo = 0;
    }
}

void aparecerOvni(Ovni* OvniVolador) {
    OvniVolador->rect.x = ANCHO_PANTALLA;
    OvniVolador->activo = 1;
    OvniVolador->contador = 0;
}