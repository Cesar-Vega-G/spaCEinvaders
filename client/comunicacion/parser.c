#include "parser.h"
#include <stdio.h>
#include <string.h>

void parsearEstado(const char* estado,
                   Ovni* ovni,
                   BloqueEnemigos* bloque,
                   Jugador jugadores[],
                   Bala balas[],
                   int* jugando,
                   Conexion* conexion) {
    char linea[256];
    const char* ptr = estado;

    while (*ptr) {
        int i = 0;
        while (*ptr && *ptr != '\n' && i < 255) linea[i++] = *ptr++;
        linea[i] = '\0';
        if (*ptr == '\n') ptr++;
        if (i == 0) continue;

        int x, y, activo, puntos, id, vidas;

        // ── BIENVENIDO id ────────────────────────────────
        if (sscanf(linea, "BIENVENIDO %d", &id) == 1) {
            conexion->idJugador = id;
            continue;
        }

        // ── ESPECTADOR ───────────────────────────────────
        if (strncmp(linea, "ESPECTADOR", 10) == 0) {
            conexion->idJugador = -1;
            continue;
        }

        // ── JUGADOR id x y vidas puntaje ─────────────────
        // Hay hasta 2 jugadores (id=0 y id=1) en la misma partida.
        if (sscanf(linea, "JUGADOR %d %d %d %d %d", &id, &x, &y, &vidas, &puntos) == 5) {
            if (id == 0 || id == 1) {
                jugadores[id].rect.x = x;
                jugadores[id].rect.y = y;
                jugadores[id].vidas   = vidas;
                jugadores[id].puntaje = puntos;
                jugadores[id].activo  = 1;
            }
            continue;
        }

        // ── BALA id x y activa ───────────────────────────
        if (sscanf(linea, "BALA %d %d %d %d", &id, &x, &y, &activo) == 4) {
            if (id == 0 || id == 1) {
                balas[id].rect.x = x;
                balas[id].rect.y = y;
                balas[id].activa = activo;
            }
            continue;
        }

        // ── ENEMIGO fila col x y activo tipo ─────────────
        int fila, col;
        char tipo[32];
        if (sscanf(linea, "ENEMIGO %d %d %d %d %d %s", &fila, &col, &x, &y, &activo, tipo) == 6) {
            if (fila >= 0 && fila < FILAS_ENEMIGOS && col >= 0 && col < COLUMNAS_ENEMIGOS) {
                bloque->enemigos[fila][col].rect.x = x;
                bloque->enemigos[fila][col].rect.y = y;
                bloque->enemigos[fila][col].activo  = activo;
            }
            continue;
        }

        // ── EXTRA id x y activo tipo ──────────────────────
        // Enemigos creados por el admin con el comando CREAR
        int idExtra;
        if (sscanf(linea, "EXTRA %d %d %d %d %s", &idExtra, &x, &y, &activo, tipo) == 5) {
            if (idExtra >= 0 && idExtra < MAX_EXTRAS) {
                if (idExtra >= bloque->numExtras) bloque->numExtras = idExtra + 1;
                bloque->extras[idExtra].rect.x = x;
                bloque->extras[idExtra].rect.y = y;
                bloque->extras[idExtra].rect.w = 50;
                bloque->extras[idExtra].rect.h = 50;
                bloque->extras[idExtra].activo = activo;
                if      (strcmp(tipo, "CALAMAR")  == 0) bloque->extras[idExtra].tipo = TIPO_CALAMAR;
                else if (strcmp(tipo, "CANGREJO") == 0) bloque->extras[idExtra].tipo = TIPO_CANGREJO;
                else                                    bloque->extras[idExtra].tipo = TIPO_PULPO;
            }
            continue;
        }

        // ── OVNI x y activo puntos ────────────────────────
        if (sscanf(linea, "OVNI %d %d %d %d", &x, &y, &activo, &puntos) == 4) {
            ovni->rect.x = x;
            ovni->rect.y = y;
            ovni->activo = activo;
            ovni->puntos = puntos;
            continue;
        }

        // ── JUEGO_ACTIVO 0/1 ──────────────────────────────
        if (sscanf(linea, "JUEGO_ACTIVO %d", &activo) == 1) {
            if (activo == 0) *jugando = 0;
        }
    }
}
