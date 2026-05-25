#include <SDL2/SDL.h>
#include "constantes.h"
#include "logic/jugador.h"
#include "interface/render.h"
#include "logic/bala.h"
#include "logic/enemigo.h"
#include "logic/ovni.h"
#include "logic/colisiones.h"
#include "comunicacion/socket_cliente.h"

int main(int argc, char* argv[]) {

    // ── 1. INICIALIZAR SDL ──────────────────────────────
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Error al iniciar SDL: %s", SDL_GetError());
        return 1;
    }

    // ── 2. CREAR VENTANA ────────────────────────────────
    SDL_Window* ventana = SDL_CreateWindow(
        TITULO_JUEGO,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        ANCHO_PANTALLA,
        ALTO_PANTALLA,
        0
    );

    if (ventana == NULL) {
        SDL_Log("Error al crear ventana: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // ── 3. CREAR RENDERER ───────────────────────────────
    SDL_Renderer* renderizador = SDL_CreateRenderer(ventana, -1, SDL_RENDERER_ACCELERATED);

    if (renderizador == NULL) {
        SDL_Log("Error al crear renderizador: %s", SDL_GetError());
        SDL_DestroyWindow(ventana);
        SDL_Quit();
        return 1;
    }

    // ── 4. CREAR ENTIDADES ──────────────────────────────
    Jugador jugador = crearJugador();
    Bala bala       = crearBala();
    BloqueEnemigos bloque = crearBloque();
    Ovni ovni       = crearOvni();

    // ── 4b. CONEXION AL SERVIDOR ────────────────────────
    Conexion conexion = crearConexion();
    conectarServidor(&conexion, "127.0.0.1", 5000);

    // ── 5. GAME LOOP ────────────────────────────────────
    int jugando      = 1;
    int contadorOvni = 0;
    int puntaje      = 0;
    SDL_Event evento;

    while (jugando) {
        moverJugador(&jugador, &bala, &evento, &jugando, &conexion);
        actualizarBala(&bala);
        actualizarBloque(&bloque);
        actualizarOvni(&ovni);
        verificarColisionesBalaEnemigos(&bala, &bloque, &puntaje);
        verificarColisionBalaOvni(&bala, &ovni, &puntaje);
        renderizarTodo(renderizador, &jugador, &bala, &bloque, &ovni);
        SDL_Delay(1000 / FPS_OBJETIVO);

        contadorOvni++;
        if (contadorOvni >= 600 && ovni.activo == 0) {
            aparecerOvni(&ovni);
            contadorOvni = 0;
        }
    }

    // ── 6. LIMPIAR ──────────────────────────────────────
    cerrarConexion(&conexion);
    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(ventana);
    SDL_Quit();

    return 0;
}