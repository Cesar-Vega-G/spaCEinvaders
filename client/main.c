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
    (void)argc; (void)argv;

    // ── 1. MENU CONSOLA ──────────────────────────────────
    printf("=== spaCEinvaders ===\n");
    printf("1. Unirse como Jugador (Teclado)\n");
    printf("2. Unirse como Jugador (Control Pico)\n");
    printf("3. Unirse como Espectador\n");
    printf("Seleccione: ");
    fflush(stdout);

    int opcion = 1;
    scanf("%d", &opcion);

    int usarPico = (opcion == 2) ? 1 : 0;

    // ── 2. SDL INIT (necesario antes de SDL_Delay en conectarServidor) ──
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "Error SDL: %s\n", SDL_GetError());
        return 1;
    }

    // ── 3. CONECTAR AL SERVIDOR ──────────────────────────
    Conexion conexion = crearConexion();
    conexion.esEspectador = (opcion == 3) ? 1 : 0;
    conectarServidor(&conexion, "127.0.0.1", 5000);

    // Espectadores eligen qué partida observar
    if (conexion.esEspectador) {
        if (!elegirPartida(&conexion)) {
            printf("No se pudo unir a ninguna partida. Cerrando.\n");
            cerrarConexion(&conexion);
            SDL_Quit();
            return 0;
        }
    }

    SDL_Window *ventana = SDL_CreateWindow(
        TITULO_JUEGO,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        ANCHO_PANTALLA, ALTO_PANTALLA, 0);
    if (!ventana) { cerrarConexion(&conexion); SDL_Quit(); return 1; }

    SDL_Renderer *renderizador = SDL_CreateRenderer(ventana, -1, SDL_RENDERER_ACCELERATED);
    if (!renderizador) { SDL_DestroyWindow(ventana); cerrarConexion(&conexion); SDL_Quit(); return 1; }

    cargarTexturas(renderizador);

    // ── 4. ENTIDADES ─────────────────────────────────────
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

    // ── 5. CONTROL PICO (solo si el jugador eligió Pico) ──
    if (usarPico)
    {
        printf("[MAIN] Inicializando control Pico\n");
        fflush(stdout);
        inicializarControlPico();
    }

    // ── 6. GAME LOOP ─────────────────────────────────────
    int jugando = 1;
    SDL_Event evento;
    char buffer[16384];

    while (jugando)
    {
        if (!conexion.esEspectador)
            procesarInput(&evento, &jugando, &conexion);
        else
        {
            while (SDL_PollEvent(&evento))
                if (evento.type == SDL_QUIT)
                    jugando = 0;
        }

        if (recibirEstado(&conexion, buffer, sizeof(buffer)))
        {
            parsearEstado(buffer, &ovni, &bloque, jugadores, balas,
                          balasEnemigas, bunkers, &jugando, &conexion);
        }

        renderizarTodo(renderizador, jugadores, balas, balasEnemigas,
                       &bloque, &ovni, bunkers);

        SDL_Delay(1000 / FPS_OBJETIVO);
    }

    // ── 7. LIMPIAR ───────────────────────────────────────
    if (usarPico)
        cerrarControlPico();
    cerrarConexion(&conexion);
    liberarTexturas();
    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(ventana);
    SDL_Quit();
    return 0;
}
