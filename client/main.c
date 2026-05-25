#include <SDL2/SDL.h>
#include "constantes.h"
#include "logic/jugador.h"
#include "interface/render.h"
#include "logic/bala.h"
#include "logic/enemigo.h"
#include "logic/ovni.h"

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
/////////////////////////////////////////////////////////////////////////////////////////////////

    // ── 3. CREAR RENDERER ───────────────────────────────
    SDL_Renderer* renderizador = SDL_CreateRenderer(ventana, -1, SDL_RENDERER_ACCELERATED);

    if (renderizador == NULL) {
        SDL_Log("Error al crear renderizador: %s", SDL_GetError());
        SDL_DestroyWindow(ventana);
        SDL_Quit();
        return 1;
    }
/////////////////////////////////////////////////////////////////////////////////////////////////
    // ── 4. CREAR JUGADOR ────────────────────────────────
    Jugador jugador = crearJugador();

    //CREAR BALA
    Bala bala = crearBala();
    BloqueEnemigos bloque = crearBloque();

    Ovni ovni = crearOvni();
    
    


    // ── 5. GAME LOOP ────────────────────────────────────
    int jugando = 1;
    SDL_Event evento;

    //Ovni
    int contadorOvni = 0;
  
while (jugando) {
    moverJugador(&jugador, &bala, &evento, &jugando);
    actualizarBala(&bala);
    actualizarBloque(&bloque);
    renderizarTodo(renderizador, &jugador, &bala, &bloque, &ovni);
    actualizarOvni(&ovni);
    SDL_Delay(1000 / FPS_OBJETIVO);

    contadorOvni++;
    if (contadorOvni >= 300) {    // 300 frames = 10 segundos a 30 FPS
        aparecerOvni(&ovni);
        contadorOvni = 0;
    }
}
    
    // ── 6. LIMPIAR ──────────────────────────────────────
    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(ventana);
    SDL_Quit();

    return 0;
}