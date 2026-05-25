#include "jugador.h"
#include "bala.h"
#include "../comunicacion/socket_cliente.h"

// ── CREAR JUGADOR ───────────────────────────────────
Jugador crearJugador() {
    Jugador j;
    j.rect.x    = INICIO_X_JUGADOR;
    j.rect.y    = INICIO_Y_JUGADOR;
    j.rect.w    = ANCHO_JUGADOR;
    j.rect.h    = ALTO_JUGADOR;
    j.velocidad = VELOCIDAD_JUGADOR;
    return j;
}

// ── MOVER JUGADOR ───────────────────────────────────
void moverJugador(Jugador* jugador, Bala* bala, SDL_Event* evento, int* jugando, Conexion* conexion) {
    while (SDL_PollEvent(evento)) {
        if (evento->type == SDL_QUIT) {
            *jugando = 0;
        }
        if (evento->type == SDL_KEYDOWN) {
            if (evento->key.keysym.sym == SDLK_a && jugador->rect.x > 0) {
                jugador->rect.x -= jugador->velocidad;
                enviarMensaje(conexion, "MOVER_IZQ");
            }
            if (evento->key.keysym.sym == SDLK_d && jugador->rect.x + jugador->rect.w < ANCHO_PANTALLA) {
                jugador->rect.x += jugador->velocidad;
                enviarMensaje(conexion, "MOVER_DER");
            }
            if (evento->key.keysym.sym == SDLK_SPACE) {
                dispararBala(bala, jugador->rect.x + jugador->rect.w / 2, jugador->rect.y);
                enviarMensaje(conexion, "DISPARAR");
            }
        }
    }
}