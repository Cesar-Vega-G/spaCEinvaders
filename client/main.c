#include <SDL2/SDL.h>
#include "constantes.h"
#include <stdio.h>

// MUC — Logica / Interfaz / Control / Comunicacion
#include "logica/jugador.h"
#include "logica/bala.h"
#include "logica/bala_enemiga.h"
#include "logica/enemigo.h"
#include "logica/ovni.h"
#include "logica/bunker.h"
#include "interfaz/render.h"
#include "control/input.h"
#include "comunicacion/socket_cliente.h"
#include "comunicacion/parser.h"

int main(int argc, char *argv[])
{

    // ── 1. SDL ───────────────────────────────────────────
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Error SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *ventana = SDL_CreateWindow(
        TITULO_JUEGO,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        ANCHO_PANTALLA, ALTO_PANTALLA, 0);
    if (!ventana)
    {
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderizador = SDL_CreateRenderer(ventana, -1, SDL_RENDERER_ACCELERATED);
    if (!renderizador)
    {
        SDL_DestroyWindow(ventana);
        SDL_Quit();
        return 1;
    }

    cargarTexturas(renderizador);

    // ── 2. ENTIDADES ─────────────────────────────────────
    // Partida compartida: hasta 2 jugadores en el mismo juego.
    // jugadores[0] = blanco, jugadores[1] = cyan.
    Jugador jugadores[2];
    jugadores[0] = crearJugador();
    jugadores[1] = crearJugador();

    Bala balas[2];
    balas[0] = crearBala();
    balas[1] = crearBala();

    BalaEnemiga balasEnemigas[MAX_BALAS_ENEMIGAS];
    for (int i = 0; i < MAX_BALAS_ENEMIGAS; i++)
        balasEnemigas[i] = crearBalaEnemiga();

    Bunker bunkers[NUM_BUNKERS];
    for (int i = 0; i < NUM_BUNKERS; i++)
        bunkers[i] = crearBunker();

    BloqueEnemigos bloque = crearBloque();
    Ovni ovni = crearOvni();

    // ── 3. CONECTAR AL SERVIDOR ──────────────────────────
    // Uso: ./juego.exe      → jugador
    //      ./juego.exe s    → espectador
    Conexion conexion = crearConexion();
    conexion.esEspectador = (argc > 1 && argv[1][0] == 's') ? 1 : 0;
    conectarServidor(&conexion, "127.0.0.1", 5000);
    printf("[MAIN] Despues de conectar al servidor\n");
    fflush(stdout);

    if (!conexion.esEspectador)
    {
        printf("[MAIN] Voy a inicializar Pico\n");
        fflush(stdout);
        inicializarControlPico();
    }
    else
    {
        printf("[MAIN] Soy espectador, no inicializo Pico\n");
        fflush(stdout);
    }

    // ── 4. GAME LOOP ─────────────────────────────────────
    int jugando = 1;
    SDL_Event evento;
    char buffer[16384];

    while (jugando)
    {
        // Control (espectador no envia comandos)
        if (!conexion.esEspectador)
            procesarInput(&evento, &jugando, &conexion);
        else
        {
            while (SDL_PollEvent(&evento))
                if (evento.type == SDL_QUIT)
                    jugando = 0;
        }

        // Comunicacion
        if (recibirEstado(&conexion, buffer, sizeof(buffer)))
        {
            parsearEstado(buffer, &ovni, &bloque, jugadores, balas,
                          balasEnemigas, bunkers, &jugando, &conexion);
        }

        // Interfaz
        renderizarTodo(renderizador, jugadores, balas, balasEnemigas,
                       &bloque, &ovni, bunkers);

        SDL_Delay(1000 / FPS_OBJETIVO);
    }

    // ── 5. LIMPIAR ───────────────────────────────────────
    cerrarControlPico();
    cerrarConexion(&conexion);
    liberarTexturas();
    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(ventana);
    SDL_Quit();
    return 0;
}
