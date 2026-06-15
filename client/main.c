/*
 * main.c — Punto de entrada del cliente spaCEinvaders.
 *
 * Arquitectura cliente-servidor:
 *   - El SERVIDOR Java es dueño de toda la lógica del juego (posiciones,
 *     colisiones, puntaje). El cliente solo renderiza el estado recibido
 *     y envía comandos de entrada (MOVER_IZQ, MOVER_DER, DISPARAR).
 *
 * Flujo de cada ronda:
 *   1. Menú en consola → el jugador elige rol (teclado / Pico / espectador)
 *   2. Conexión TCP al servidor (127.0.0.1:5000)
 *   3. Verificación de slot (solo 1 jugador por tipo de control)
 *   4. Game loop: recibir estado → renderizar → enviar input
 *   5. Pantalla de Game Over → el jugador puede volver al paso 1
 *
 * La ventana SDL y el renderer se crean una sola vez y se reutilizan
 * entre rondas; la consola se oculta durante el juego y se muestra
 * de nuevo al volver al menú.
 */

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

    /* SDL_Init se llama una sola vez; la ventana se reutiliza entre rondas. */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Error SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window   *ventana      = NULL;
    SDL_Renderer *renderizador = NULL;
    int           continuar    = 1; /* 1 = jugar otra ronda, 0 = salir */

    while (continuar)
    {
        /* ── 1. MENÚ DE CONSOLA ──────────────────────────────────────────── */
        printf("=== spaCEinvaders ===\n");
        printf("1. Unirse como Jugador (Teclado)\n");
        printf("2. Unirse como Jugador (Control Pico)\n");
        printf("3. Unirse como Espectador\n");
        printf("Seleccione: ");
        fflush(stdout);

        int opcion = 1;
        scanf("%d", &opcion);
        while (getchar() != '\n'); /* vaciar '\n' residual del buffer */

        int usarPico = (opcion == 2) ? 1 : 0;

        /* ── 2. CONEXIÓN TCP ─────────────────────────────────────────────── */
        Conexion conexion = crearConexion();
        conexion.esEspectador = (opcion == 3) ? 1 : 0;
        conectarServidor(&conexion, "127.0.0.1", 5000, usarPico);

        if (!conexion.esEspectador) {
            /*
             * El servidor responde con "BIENVENIDO id" o "SLOT_OCUPADO".
             * Solo puede haber un jugador de teclado y uno de Pico a la vez.
             */
            if (!verificarSlot(&conexion)) {
                printf("\nEse control ya tiene un jugador activo.\n");
                printf("Presiona ENTER para volver al menu...\n");
                fflush(stdout);
                getchar();
                cerrarConexion(&conexion);
                continue;
            }
        } else {
            /* Los espectadores eligen qué partida observar de la lista. */
            if (!elegirPartida(&conexion)) {
                printf("No hay partidas disponibles. Cerrando.\n");
                cerrarConexion(&conexion);
                continuar = 0;
                break;
            }
        }

        /* Limpiar la consola para que no se vea el menú durante el juego. */
        system("cls");

        /* ── 3. VENTANA SDL (se crea solo en la primera ronda) ──────────── */
        if (!ventana) {
            ventana = SDL_CreateWindow(
                TITULO_JUEGO,
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                960, 720, 0);
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

            /*
             * SDL_RenderSetLogicalSize permite que el juego use siempre
             * coordenadas 1200×900 internamente, independiente del tamaño
             * real de la ventana; SDL escala automáticamente.
             */
            SDL_RenderSetLogicalSize(renderizador, ANCHO_PANTALLA, ALTO_PANTALLA);
            cargarTexturas(renderizador);
        }
        SDL_ShowWindow(ventana);
        SDL_RaiseWindow(ventana);

        /* ── 4. INICIALIZAR ENTIDADES LOCALES ───────────────────────────── */
        /*
         * Estos structs son "espejos" locales del estado del servidor.
         * Sus valores reales se sobreescriben en cada frame por parsearEstado().
         */
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

        /* ── 5. INICIALIZAR CONTROL PICO (puerto serial) ────────────────── */
        if (usarPico)
            inicializarControlPico();

        /* Ocultar consola después de inicializar el Pico para que sus mensajes
         * de error (si los hay) sean visibles antes de esconderla. */
        ShowWindow(GetConsoleWindow(), SW_HIDE);

        /* ── 6. GAME LOOP ───────────────────────────────────────────────── */
        int jugando = 1;
        SDL_Event evento;
        char buffer[16384]; /* búfer para el estado serializado del servidor */

        while (jugando)
        {
            /* Procesar entrada: teclado+Pico para jugadores, solo quit para espectadores. */
            if (!conexion.esEspectador)
                procesarInput(&evento, &jugando, &conexion);
            else {
                while (SDL_PollEvent(&evento))
                    if (evento.type == SDL_QUIT) jugando = 0;
            }

            /* Recibir estado del servidor (no bloqueante) y actualizar entidades locales. */
            if (recibirEstado(&conexion, buffer, sizeof(buffer)))
                parsearEstado(buffer, &ovni, &bloque, jugadores, balas,
                              balasEnemigas, bunkers, &jugando, &conexion);

            /* Renderizar el frame actual. */
            renderizarTodo(renderizador, jugadores, balas, balasEnemigas,
                           &bloque, &ovni, bunkers);

            SDL_Delay(1000 / FPS_OBJETIVO);
        }

        /* ── 7. PANTALLA DE GAME OVER ───────────────────────────────────── */
        /* Retorna 1 si el jugador quiere volver a jugar, 0 si sale. */
        continuar = mostrarGameOver(renderizador, jugadores[0].puntaje);

        /* ── 8. LIMPIEZA DE RONDA ───────────────────────────────────────── */
        if (usarPico)
            cerrarControlPico();
        cerrarConexion(&conexion); /* libera el socket; el servidor libera el slot */

        /* Ocultar ventana SDL y mostrar la consola para el siguiente menú. */
        if (continuar) {
            SDL_HideWindow(ventana);
            ShowWindow(GetConsoleWindow(), SW_SHOW);
            SetForegroundWindow(GetConsoleWindow());
        }
    }

    /* ── 9. LIMPIEZA FINAL ──────────────────────────────────────────────── */
    liberarTexturas();
    if (renderizador) SDL_DestroyRenderer(renderizador);
    if (ventana)      SDL_DestroyWindow(ventana);
    SDL_Quit();
    return 0;
}
