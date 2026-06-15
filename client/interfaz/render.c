/**
 * INTERFAZ — Módulo de renderizado de spaCEinvaders.
 *
 * Responsabilidades:
 *   · Cargar y liberar texturas PNG usando stb_image (sin SDL_image).
 *   · Dibujar todos los elementos del juego cada frame (renderizarTodo).
 *   · Mostrar la pantalla de Game Over y esperar la decisión del usuario.
 *
 * Fuente de píxeles:
 *   Los dígitos y letras se renderizan con bitmaps 3×5 (FUENTE y LETRAS).
 *   Cada "píxel" del bitmap se escala a `sc×sc` píxeles SDL antes de
 *   dibujarlo con SDL_RenderFillRect, lo que permite un texto nítido a
 *   cualquier tamaño sin necesitar un archivo de fuente externo.
 *
 * stb_image:
 *   Se usa para decodificar PNG a RGBA en memoria.  SDL_CreateRGBSurfaceFrom
 *   envuelve ese búfer en un SDL_Surface sin copiarlo, y SDL_CreateTextureFromSurface
 *   lo sube a la GPU.  stbi_image_free libera el búfer original después de
 *   que SDL ya copió los datos.
 *
 * Coordenadas lógicas:
 *   Todas las posiciones usan el espacio lógico 1200×900 definido en
 *   constantes.h. SDL_RenderSetLogicalSize escala automáticamente a la
 *   ventana física (960×720), por lo que aquí nunca se trabaja con píxeles
 *   reales de pantalla.
 */
#include "render.h"
#include <stdio.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb_image.h"

/* Texturas estáticas compartidas por todo el módulo. Se cargan una sola vez
 * en cargarTexturas() y se liberan al cerrar el programa en liberarTexturas(). */
static SDL_Texture* texJugador  = NULL;
static SDL_Texture* texCalamar  = NULL;
static SDL_Texture* texCangrejo = NULL;
static SDL_Texture* texPulpo    = NULL;
static SDL_Texture* texOvni     = NULL;

/* Carga un PNG desde disco y devuelve una textura SDL lista para dibujar.
 * Devuelve NULL si el archivo no existe; dibujarSprite caerá a un rectángulo
 * de color para que el juego siga funcionando sin los assets. */
static SDL_Texture* cargarPNG(SDL_Renderer* r, const char* ruta) {
    int w, h, canales;
    unsigned char* datos = stbi_load(ruta, &w, &h, &canales, 4);
    if (!datos) {
        fprintf(stderr, "[RENDER] No se pudo cargar: %s\n", ruta);
        return NULL;
    }
    SDL_Surface* sup = SDL_CreateRGBSurfaceFrom(
        datos, w, h, 32, w * 4,
        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    SDL_Texture* tex = NULL;
    if (sup) {
        tex = SDL_CreateTextureFromSurface(r, sup);
        SDL_FreeSurface(sup);
    }
    stbi_image_free(datos);
    return tex;
}

int cargarTexturas(SDL_Renderer* renderizador) {
    texJugador  = cargarPNG(renderizador, "assets/jugador.png");
    texCalamar  = cargarPNG(renderizador, "assets/calamar.png");
    texCangrejo = cargarPNG(renderizador, "assets/cangrejo.png");
    texPulpo    = cargarPNG(renderizador, "assets/pulpo.png");
    texOvni     = cargarPNG(renderizador, "assets/ovni.png");
    return 1;
}

void liberarTexturas(void) {
    if (texJugador)  SDL_DestroyTexture(texJugador);
    if (texCalamar)  SDL_DestroyTexture(texCalamar);
    if (texCangrejo) SDL_DestroyTexture(texCangrejo);
    if (texPulpo)    SDL_DestroyTexture(texPulpo);
    if (texOvni)     SDL_DestroyTexture(texOvni);
    texJugador = texCalamar = texCangrejo = texPulpo = texOvni = NULL;
}

/* Dibuja la textura en el rect dado.  Si la textura es NULL (asset no
 * encontrado), dibuja un rectángulo del color (cr,cg,cb) como fallback. */
static void dibujarSprite(SDL_Renderer* r, SDL_Texture* tex,
                          const SDL_Rect* rect,
                          Uint8 cr, Uint8 cg, Uint8 cb) {
    if (tex) {
        SDL_RenderCopy(r, tex, NULL, rect);
    } else {
        SDL_SetRenderDrawColor(r, cr, cg, cb, 255);
        SDL_RenderFillRect(r, rect);
    }
}

/* ── FUENTE DE PÍXELES ──────────────────────────────────────────────────── */
/* Bitmaps 3×5 para los dígitos 0–9.  Cada fila es un array de 3 bits:
 * 1 = píxel encendido, 0 = apagado.  Se renderizan ampliados por `sc`. */
static const int FUENTE[10][5][3] = {
    {{1,1,1},{1,0,1},{1,0,1},{1,0,1},{1,1,1}}, // 0
    {{0,1,0},{0,1,0},{0,1,0},{0,1,0},{0,1,0}}, // 1
    {{1,1,1},{0,0,1},{1,1,1},{1,0,0},{1,1,1}}, // 2
    {{1,1,1},{0,0,1},{0,1,1},{0,0,1},{1,1,1}}, // 3
    {{1,0,1},{1,0,1},{1,1,1},{0,0,1},{0,0,1}}, // 4
    {{1,1,1},{1,0,0},{1,1,1},{0,0,1},{1,1,1}}, // 5
    {{1,1,1},{1,0,0},{1,1,1},{1,0,1},{1,1,1}}, // 6
    {{1,1,1},{0,0,1},{0,0,1},{0,0,1},{0,0,1}}, // 7
    {{1,1,1},{1,0,1},{1,1,1},{1,0,1},{1,1,1}}, // 8
    {{1,1,1},{1,0,1},{1,1,1},{0,0,1},{1,1,1}}, // 9
};

/* Bitmaps 3×5 para las letras A–Z (solo mayúsculas).  Mismo esquema de
 * indexado: LETRAS['C'-'A'] → bitmap de la letra C. */
static const int LETRAS[26][5][3] = {
    {{0,1,0},{1,0,1},{1,1,1},{1,0,1},{1,0,1}}, // A
    {{1,1,0},{1,0,1},{1,1,0},{1,0,1},{1,1,0}}, // B
    {{0,1,1},{1,0,0},{1,0,0},{1,0,0},{0,1,1}}, // C
    {{1,1,0},{1,0,1},{1,0,1},{1,0,1},{1,1,0}}, // D
    {{1,1,1},{1,0,0},{1,1,0},{1,0,0},{1,1,1}}, // E
    {{1,1,1},{1,0,0},{1,1,0},{1,0,0},{1,0,0}}, // F
    {{0,1,1},{1,0,0},{1,0,1},{1,0,1},{0,1,1}}, // G
    {{1,0,1},{1,0,1},{1,1,1},{1,0,1},{1,0,1}}, // H
    {{1,1,1},{0,1,0},{0,1,0},{0,1,0},{1,1,1}}, // I
    {{0,1,1},{0,0,1},{0,0,1},{1,0,1},{0,1,0}}, // J
    {{1,0,1},{1,1,0},{1,1,0},{1,0,1},{1,0,1}}, // K
    {{1,0,0},{1,0,0},{1,0,0},{1,0,0},{1,1,1}}, // L
    {{1,0,1},{1,1,1},{1,1,1},{1,0,1},{1,0,1}}, // M
    {{1,0,1},{1,1,1},{1,0,1},{1,0,1},{1,0,1}}, // N
    {{0,1,0},{1,0,1},{1,0,1},{1,0,1},{0,1,0}}, // O
    {{1,1,0},{1,0,1},{1,1,0},{1,0,0},{1,0,0}}, // P
    {{0,1,0},{1,0,1},{1,0,1},{0,1,1},{0,0,1}}, // Q
    {{1,1,0},{1,0,1},{1,1,0},{1,0,1},{1,0,1}}, // R
    {{0,1,1},{1,0,0},{0,1,0},{0,0,1},{1,1,0}}, // S
    {{1,1,1},{0,1,0},{0,1,0},{0,1,0},{0,1,0}}, // T
    {{1,0,1},{1,0,1},{1,0,1},{1,0,1},{0,1,1}}, // U
    {{1,0,1},{1,0,1},{1,0,1},{0,1,0},{0,1,0}}, // V
    {{1,0,1},{1,0,1},{1,1,1},{1,1,1},{1,0,1}}, // W
    {{1,0,1},{1,0,1},{0,1,0},{1,0,1},{1,0,1}}, // X
    {{1,0,1},{1,0,1},{0,1,0},{0,1,0},{0,1,0}}, // Y
    {{1,1,1},{0,0,1},{0,1,0},{1,0,0},{1,1,1}}, // Z
};

/* ── FUNCIONES DE TEXTO ─────────────────────────────────────────────────── */

/* Dibuja un solo carácter (dígito o letra) en (x,y) escalado a sc×sc px. */
static void dibujarChar(SDL_Renderer* r, char c, int x, int y, int sc,
                        Uint8 cr, Uint8 cg, Uint8 cb) {
    const int (*bmp)[3] = NULL;
    if      (c >= '0' && c <= '9') bmp = FUENTE[c - '0'];
    else if (c >= 'A' && c <= 'Z') bmp = LETRAS[c - 'A'];
    else return;
    SDL_SetRenderDrawColor(r, cr, cg, cb, 255);
    for (int row = 0; row < 5; row++)
        for (int col = 0; col < 3; col++)
            if (bmp[row][col]) {
                SDL_Rect px = {x + col*sc, y + row*sc, sc, sc};
                SDL_RenderFillRect(r, &px);
            }
}

/* Dibuja una cadena carácter a carácter.  El avance entre caracteres es
 * sc*4: 3 columnas de píxel + 1 columna de espacio. */
static void dibujarTexto(SDL_Renderer* r, const char* txt, int x, int y, int sc,
                          Uint8 cr, Uint8 cg, Uint8 cb) {
    for (; *txt; txt++, x += sc * 4)
        dibujarChar(r, *txt, x, y, sc, cr, cg, cb);
}

/* Calcula el ancho total del texto y lo centra en ANCHO_PANTALLA (1200). */
static void dibujarTextoCentrado(SDL_Renderer* r, const char* txt, int y, int sc,
                                  Uint8 cr, Uint8 cg, Uint8 cb) {
    int ancho = (int)strlen(txt) * sc * 4;
    dibujarTexto(r, txt, (ANCHO_PANTALLA - ancho) / 2, y, sc, cr, cg, cb);
}

/* Convierte el número a texto y lo dibuja usando FUENTE[].
 * Predecesor de dibujarTexto; se mantiene por compatibilidad con el HUD. */
static void dibujarNumero(SDL_Renderer* r, int numero, int x, int y, int escala) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", numero);
    for (int i = 0; buf[i] != '\0'; i++) {
        int d  = buf[i] - '0';
        int ox = x + i * (escala * 4);
        for (int fila = 0; fila < 5; fila++)
            for (int col = 0; col < 3; col++)
                if (FUENTE[d][fila][col]) {
                    SDL_Rect px = {ox + col*escala, y + fila*escala, escala, escala};
                    SDL_RenderFillRect(r, &px);
                }
    }
}

/* ── RENDERIZADO PRINCIPAL ──────────────────────────────────────────────── */
/**
 * Dibuja el frame completo del juego.  El orden importa: los elementos más
 * lejanos (fondo) se dibujan primero; los del HUD, al final, para que queden
 * encima de todo.
 *
 * El estado de cada entidad viene del parser.c, que actualiza los structs
 * en memoria con el último estado recibido del servidor.  Este módulo solo
 * lee; nunca modifica el estado del juego.
 */
void renderizarTodo(SDL_Renderer* renderizador,
                    Jugador jugadores[],
                    Bala balas[],
                    BalaEnemiga balasEnemigas[],
                    BloqueEnemigos* bloque,
                    Ovni* ovni,
                    Bunker bunkers[]) {

    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255);
    SDL_RenderClear(renderizador);

    /* 2. Jugadores. El jugador 0 usa la textura sin modificar (blanco);
     *    el jugador 1 aplica tinte cyan con SDL_SetTextureColorMod y lo
     *    restaura después para no afectar los iconos de vida del HUD. */
    if (jugadores[0].activo)
        dibujarSprite(renderizador, texJugador, &jugadores[0].rect, 255, 255, 255);

    if (jugadores[1].activo) {
        if (texJugador) {
            SDL_SetTextureColorMod(texJugador, 0, 255, 255);
            SDL_RenderCopy(renderizador, texJugador, NULL, &jugadores[1].rect);
            SDL_SetTextureColorMod(texJugador, 255, 255, 255);
        } else {
            SDL_SetRenderDrawColor(renderizador, 0, 255, 255, 255);
            SDL_RenderFillRect(renderizador, &jugadores[1].rect);
        }
    }

    /* 3. Balas de jugadores: amarilla (J0) y naranja (J1). */
    if (balas[0].activa) {
        SDL_SetRenderDrawColor(renderizador, 255, 255, 0, 255);
        SDL_RenderFillRect(renderizador, &balas[0].rect);
    }
    if (balas[1].activa) {
        SDL_SetRenderDrawColor(renderizador, 255, 165, 0, 255);
        SDL_RenderFillRect(renderizador, &balas[1].rect);
    }

    /* 4. Balas enemigas (rojo suave, distintas de las del jugador). */
    SDL_SetRenderDrawColor(renderizador, 255, 60, 60, 255);
    for (int i = 0; i < MAX_BALAS_ENEMIGAS; i++) {
        if (balasEnemigas[i].activa)
            SDL_RenderFillRect(renderizador, &balasEnemigas[i].rect);
    }

    /* 5. Enemigos de la grilla principal (55 en formación 5×11). */
    for (int f = 0; f < FILAS_ENEMIGOS; f++) {
        for (int c = 0; c < COLUMNAS_ENEMIGOS; c++) {
            if (!bloque->enemigos[f][c].activo) continue;
            Enemigo* e = &bloque->enemigos[f][c];
            switch (e->tipo) {
                case TIPO_CALAMAR:
                    dibujarSprite(renderizador, texCalamar,  &e->rect, 255, 0,   0);   break;
                case TIPO_CANGREJO:
                    dibujarSprite(renderizador, texCangrejo, &e->rect, 0,   255, 0);   break;
                case TIPO_PULPO:
                    dibujarSprite(renderizador, texPulpo,    &e->rect, 0,   0,   255); break;
                default:
                    SDL_SetRenderDrawColor(renderizador, 200, 200, 200, 255);
                    SDL_RenderFillRect(renderizador, &e->rect);
            }
        }
    }

    /* 6. Enemigos extra creados por el admin con el comando CREAR. */
    for (int i = 0; i < bloque->numExtras; i++) {
        if (!bloque->extras[i].activo) continue;
        Enemigo* e = &bloque->extras[i];
        switch (e->tipo) {
            case TIPO_CALAMAR:
                dibujarSprite(renderizador, texCalamar,  &e->rect, 255, 0,   0);   break;
            case TIPO_CANGREJO:
                dibujarSprite(renderizador, texCangrejo, &e->rect, 0,   255, 0);   break;
            case TIPO_PULPO:
                dibujarSprite(renderizador, texPulpo,    &e->rect, 0,   0,   255); break;
            default:
                SDL_SetRenderDrawColor(renderizador, 200, 200, 200, 255);
                SDL_RenderFillRect(renderizador, &e->rect);
        }
    }

    /* 7. Bunkers: grilla de bloques destructibles (verdes).
     *    Cada bloque se dibuja individualmente según su estado (vivo/destruido). */
    SDL_SetRenderDrawColor(renderizador, 0, 220, 80, 255);
    for (int i = 0; i < NUM_BUNKERS; i++) {
        for (int f = 0; f < bunkers[i].filas && f < BUNKER_FILAS; f++) {
            for (int c = 0; c < bunkers[i].columnas && c < BUNKER_COLUMNAS; c++) {
                if (!bunkers[i].bloques[f][c]) continue;
                SDL_Rect rr = {
                    bunkers[i].x + c * bunkers[i].lado,
                    bunkers[i].y + f * bunkers[i].lado,
                    bunkers[i].lado,
                    bunkers[i].lado
                };
                SDL_RenderFillRect(renderizador, &rr);
            }
        }
    }

    /* 8. OVNI (aparece en la parte superior, se mueve de lado a lado). */
    if (ovni->activo)
        dibujarSprite(renderizador, texOvni, &ovni->rect, 255, 50, 50);

    /* 9. HUD — siempre encima de los sprites del juego.
     *    Jugador 0: puntaje (blanco, arriba izquierda) + vidas como sprites.
     *    Jugador 1: puntaje (cyan, arriba derecha) + vidas con tinte cyan. */
    SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 255);
    dibujarNumero(renderizador, jugadores[0].puntaje, 10, 10, 3);
    for (int v = 0; v < jugadores[0].vidas; v++) {
        SDL_Rect vida = {10 + v * 40, 50, 32, 15};
        dibujarSprite(renderizador, texJugador, &vida, 255, 255, 255);
    }

    if (jugadores[1].activo) {
        SDL_SetRenderDrawColor(renderizador, 0, 255, 255, 255);
        dibujarNumero(renderizador, jugadores[1].puntaje, 1050, 10, 3);
        for (int v = 0; v < jugadores[1].vidas; v++) {
            SDL_Rect vida = {1050 + v * 40, 50, 32, 15};
            if (texJugador) {
                SDL_SetTextureColorMod(texJugador, 0, 255, 255);
                SDL_RenderCopy(renderizador, texJugador, NULL, &vida);
                SDL_SetTextureColorMod(texJugador, 255, 255, 255);
            } else {
                SDL_SetRenderDrawColor(renderizador, 0, 255, 255, 255);
                SDL_RenderFillRect(renderizador, &vida);
            }
        }
    }

    /* 10. Presentar el frame terminado al monitor. */
    SDL_RenderPresent(renderizador);
}

/* ── PANTALLA DE GAME OVER ──────────────────────────────────────────────── */
/**
 * Bloquea hasta que el usuario elige una opción.
 * Retorna 1 si presiona ENTER o hace clic en "JUGAR DE NUEVO".
 * Retorna 0 si presiona ESC, cierra la ventana o hace clic en "SALIR".
 *
 * Se mantiene el bucle de eventos propio para no depender del game loop
 * principal (que ya terminó antes de llegar aquí).
 */
int mostrarGameOver(SDL_Renderer* r, int puntaje) {
    char bufPts[32];
    snprintf(bufPts, sizeof(bufPts), "PUNTAJE  %d", puntaje);

    SDL_Rect btnJugar = {ANCHO_PANTALLA/2 - 230, 460, 460, 65}; /* verde */
    SDL_Rect btnSalir = {ANCHO_PANTALLA/2 - 230, 570, 460, 65}; /* rojo  */

    int resultado = 0, loop = 1;
    SDL_Event e;

    while (loop) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                loop = 0; resultado = 0;
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN ||
                    e.key.keysym.sym == SDLK_KP_ENTER) {
                    loop = 0; resultado = 1;
                }
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    loop = 0; resultado = 0;
                }
            }
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x, my = e.button.y;
                if (mx >= btnJugar.x && mx < btnJugar.x + btnJugar.w &&
                    my >= btnJugar.y && my < btnJugar.y + btnJugar.h) {
                    loop = 0; resultado = 1;
                }
                if (mx >= btnSalir.x && mx < btnSalir.x + btnSalir.w &&
                    my >= btnSalir.y && my < btnSalir.y + btnSalir.h) {
                    loop = 0; resultado = 0;
                }
            }
        }

        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_RenderClear(r);

        /* "GAME OVER" grande en rojo (escala 12 → cada dot ocupa 12×12 px). */
        dibujarTextoCentrado(r, "GAME OVER", 180, 12, 220, 40, 40);

        /* Puntaje final centrado (escala 6). */
        dibujarTextoCentrado(r, bufPts, 330, 6, 255, 255, 255);

        SDL_SetRenderDrawColor(r, 20, 140, 20, 255);
        SDL_RenderFillRect(r, &btnJugar);
        dibujarTextoCentrado(r, "ENTER  JUGAR DE NUEVO", 484, 4, 255, 255, 255);

        SDL_SetRenderDrawColor(r, 140, 20, 20, 255);
        SDL_RenderFillRect(r, &btnSalir);
        dibujarTextoCentrado(r, "ESC  SALIR", 594, 4, 255, 255, 255);

        SDL_RenderPresent(r);
        SDL_Delay(33);
    }

    return resultado;
}
