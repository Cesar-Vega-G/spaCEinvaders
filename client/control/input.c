#include "input.h"

// Controles separados por jugador:
//   Jugador 0 (idJugador == 0): A = izq, D = der, ESPACIO = disparar
//   Jugador 1 (idJugador == 1): ← = izq, → = der, ↑ = disparar
void procesarInput(SDL_Event* evento, int* jugando, Conexion* conexion) {
    while (SDL_PollEvent(evento)) {
        if (evento->type == SDL_QUIT) {
            *jugando = 0;
        }
        if (evento->type == SDL_KEYDOWN) {
            int id = conexion->idJugador;

            switch (evento->key.keysym.sym) {

                // ── Jugador 0: A / D / ESPACIO ──────────────
                case SDLK_a:
                    if (id == 0) enviarMensaje(conexion, "MOVER_IZQ");
                    break;
                case SDLK_d:
                    if (id == 0) enviarMensaje(conexion, "MOVER_DER");
                    break;
                case SDLK_SPACE:
                    if (id == 0) enviarMensaje(conexion, "DISPARAR");
                    break;

                // ── Jugador 1: ← / → / ↑ ────────────────────
                case SDLK_LEFT:
                    if (id == 1) enviarMensaje(conexion, "MOVER_IZQ");
                    break;
                case SDLK_RIGHT:
                    if (id == 1) enviarMensaje(conexion, "MOVER_DER");
                    break;
                case SDLK_UP:
                    if (id == 1) enviarMensaje(conexion, "DISPARAR");
                    break;

                // ── Salir (cualquier ventana) ────────────────
                case SDLK_ESCAPE:
                    *jugando = 0;
                    break;

                default:
                    break;
            }
        }
    }
}
