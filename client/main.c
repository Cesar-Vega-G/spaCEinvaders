#include <SDL2/SDL.h>
#include "constantes.h"
#include <stdio.h>

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
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Error SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window   *ventana      = NULL;
    SDL_Renderer *renderizador = NULL;
    int           continuar    = 1;

    while (continuar)
    {
        // ── MENU CONSOLA ─────────────────────────────────────
        printf("=== spaCEinvaders ===\n");
        printf("1. Unirse como Jugador (Teclado)\n");
        printf("2. Unirse como Jugador (Control Pico)\n");
        printf("3. Unirse como Espectador\n");
        printf("Seleccione: ");
        fflush(stdout);

        int opcion = 1;
        scanf("%d", &opcion);
        while (getchar() != '\n'); // vaciar buffer de entrada

        int usarPico = (opcion == 2) ? 1 : 0;

        // ── CONECTAR AL SERVIDOR ─────────────────────────────
        Conexion conexion = crearConexion();
        conexion.esEspectador = (opcion == 3) ? 1 : 0;
        conectarServidor(&conexion, "127.0.0.1", 5000, usarPico);

        if (!conexion.esEspectador) {
            if (!verificarSlot(&conexion)) {
                printf("\nEse control ya tiene un jugador activo.\n");
                printf("Presiona ENTER para volver al menu...\n");
                fflush(stdout);
                getchar();
                cerrarConexion(&conexion);
                continue;
            }
        } else {
            if (!elegirPartida(&conexion)) {
                printf("No hay partidas disponibles. Cerrando.\n");
                cerrarConexion(&conexion);
                continuar = 0;
                break;
            }
        }

        system("cls");

        // ── SDL WINDOW (crear solo la primera vez) ────────────
        if (!ventana) {
            ventana = SDL_CreateWindow(
                TITULO_JUEGO,
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                ANCHO_PANTALLA, ALTO_PANTALLA, 0);
            if (!ventana) {
                cerrarConexion(&conexion);
                continuar = 0;
                break;
            }
            renderizador = SDL_CreateRenderer(ventana, -1, SDL_RENDERER_ACCELERATED);
            if (!renderizador) {
                SDL_DestroyWindow(ventana); ventana = NULL;
                cerrarConexion(&conexion);
                continuar = 0;
                break;
            }
            cargarTexturas(renderizador);
        }
        SDL_ShowWindow(ventana);
        SDL_RaiseWindow(ventana);
<<<<<<< HEAD
        ShowWindow(GetConsoleWindow(), SW_HIDE);
=======
>>>>>>> origin/cambiosVictor

        // ── ENTIDADES ─────────────────────────────────────────
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

        // ── CONTROL PICO ──────────────────────────────────────
        if (usarPico)
            inicializarControlPico();

        // ── GAME LOOP ─────────────────────────────────────────
        int jugando = 1;
        SDL_Event evento;
        char buffer[16384];

        while (jugando)
        {
            if (!conexion.esEspectador)
                procesarInput(&evento, &jugando, &conexion);
            else {
                while (SDL_PollEvent(&evento))
                    if (evento.type == SDL_QUIT) jugando = 0;
            }

            if (recibirEstado(&conexion, buffer, sizeof(buffer)))
                parsearEstado(buffer, &ovni, &bloque, jugadores, balas,
                              balasEnemigas, bunkers, &jugando, &conexion);

            renderizarTodo(renderizador, jugadores, balas, balasEnemigas,
                           &bloque, &ovni, bunkers);

            SDL_Delay(1000 / FPS_OBJETIVO);
        }

        // ── GAME OVER ─────────────────────────────────────────
        continuar = mostrarGameOver(renderizador, jugadores[0].puntaje);

        // ── LIMPIAR ESTA RONDA ────────────────────────────────
        if (usarPico)
            cerrarControlPico();
        cerrarConexion(&conexion);

<<<<<<< HEAD
        // Ocultar ventana SDL y mostrar consola para el menu
        if (continuar) {
            SDL_HideWindow(ventana);
            ShowWindow(GetConsoleWindow(), SW_SHOW);
            SetForegroundWindow(GetConsoleWindow());
        }
=======
        // Ocultar ventana mientras el jugador ve el menu de consola
        if (continuar)
            SDL_HideWindow(ventana);
>>>>>>> origin/cambiosVictor
    }

    // ── LIMPIAR FINAL ─────────────────────────────────────────
    liberarTexturas();
    if (renderizador) SDL_DestroyRenderer(renderizador);
    if (ventana)      SDL_DestroyWindow(ventana);
    SDL_Quit();
    return 0;
}
