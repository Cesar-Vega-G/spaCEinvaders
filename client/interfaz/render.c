#include "render.h"
#include <stdio.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb_image.h"

static SDL_Texture* texJugador  = NULL;
static SDL_Texture* texCalamar  = NULL;
static SDL_Texture* texCangrejo = NULL;
static SDL_Texture* texPulpo    = NULL;
static SDL_Texture* texOvni     = NULL;

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

// Dibuja textura; si es NULL cae a rectangulo de color fallback.
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

// Fuente de pixeles 3x5 para digitos 0-9
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

// Fuente de pixeles 3x5 para letras A-Z
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

static void dibujarTexto(SDL_Renderer* r, const char* txt, int x, int y, int sc,
                          Uint8 cr, Uint8 cg, Uint8 cb) {
    for (; *txt; txt++, x += sc * 4)
        dibujarChar(r, *txt, x, y, sc, cr, cg, cb);
}

static void dibujarTextoCentrado(SDL_Renderer* r, const char* txt, int y, int sc,
                                  Uint8 cr, Uint8 cg, Uint8 cb) {
    int ancho = (int)strlen(txt) * sc * 4;
    dibujarTexto(r, txt, (ANCHO_PANTALLA - ancho) / 2, y, sc, cr, cg, cb);
}

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

void renderizarTodo(SDL_Renderer* renderizador,
                    Jugador jugadores[],
                    Bala balas[],
                    BalaEnemiga balasEnemigas[],
                    BloqueEnemigos* bloque,
                    Ovni* ovni,
                    Bunker bunkers[]) {

    // Fondo negro
    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255);
    SDL_RenderClear(renderizador);

    // Jugador 0 (sprite; tinte blanco por defecto)
    if (jugadores[0].activo)
        dibujarSprite(renderizador, texJugador, &jugadores[0].rect, 255, 255, 255);

    // Jugador 1 (mismo sprite; tinte cyan)
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

    // Bala jugador 0 (amarilla)
    if (balas[0].activa) {
        SDL_SetRenderDrawColor(renderizador, 255, 255, 0, 255);
        SDL_RenderFillRect(renderizador, &balas[0].rect);
    }

    // Bala jugador 1 (naranja)
    if (balas[1].activa) {
        SDL_SetRenderDrawColor(renderizador, 255, 165, 0, 255);
        SDL_RenderFillRect(renderizador, &balas[1].rect);
    }

    // Balas enemigas (rojas)
    SDL_SetRenderDrawColor(renderizador, 255, 60, 60, 255);
    for (int i = 0; i < MAX_BALAS_ENEMIGAS; i++) {
        if (balasEnemigas[i].activa)
            SDL_RenderFillRect(renderizador, &balasEnemigas[i].rect);
    }

    // Enemigos de la grilla principal
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

    // Enemigos extra (creados por el admin con CREAR)
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

    // Bunkers (verdes, grilla de bloques destructibles)
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

    // OVNI
    if (ovni->activo)
        dibujarSprite(renderizador, texOvni, &ovni->rect, 255, 50, 50);

    // HUD jugador 0: puntaje en blanco arriba izquierda
    SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 255);
    dibujarNumero(renderizador, jugadores[0].puntaje, 10, 10, 3);

    // HUD jugador 0: vidas como miniaturas del sprite del jugador
    for (int v = 0; v < jugadores[0].vidas; v++) {
        SDL_Rect vida = {10 + v * 40, 50, 32, 15};
        dibujarSprite(renderizador, texJugador, &vida, 255, 255, 255);
    }

    // HUD jugador 1: puntaje en cyan arriba derecha (si existe)
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

    SDL_RenderPresent(renderizador);
}

int mostrarGameOver(SDL_Renderer* r, int puntaje) {
    char bufPts[32];
    snprintf(bufPts, sizeof(bufPts), "PUNTAJE  %d", puntaje);

    // Botón JUGAR DE NUEVO (verde)
    SDL_Rect btnJugar = {ANCHO_PANTALLA/2 - 230, 460, 460, 65};
    // Botón SALIR (rojo)
    SDL_Rect btnSalir = {ANCHO_PANTALLA/2 - 230, 570, 460, 65};

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

        // "GAME OVER" en rojo (escala 12 → cada pixel 12x12)
        dibujarTextoCentrado(r, "GAME OVER", 180, 12, 220, 40, 40);

        // Puntaje final en blanco (escala 6)
        dibujarTextoCentrado(r, bufPts, 330, 6, 255, 255, 255);

        // Botón verde: JUGAR DE NUEVO
        SDL_SetRenderDrawColor(r, 20, 140, 20, 255);
        SDL_RenderFillRect(r, &btnJugar);
        dibujarTextoCentrado(r, "ENTER  JUGAR DE NUEVO", 484, 4, 255, 255, 255);

        // Botón rojo: SALIR
        SDL_SetRenderDrawColor(r, 140, 20, 20, 255);
        SDL_RenderFillRect(r, &btnSalir);
        dibujarTextoCentrado(r, "ESC  SALIR", 594, 4, 255, 255, 255);

        SDL_RenderPresent(r);
        SDL_Delay(33);
    }

    return resultado;
}
