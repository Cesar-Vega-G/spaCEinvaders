#ifndef CONSTANTES_H
#define CONSTANTES_H

// Pantalla
#define ANCHO_PANTALLA 1200
#define ALTO_PANTALLA 900
#define TITULO_JUEGO "spaCEinvaders"
#define FPS_OBJETIVO 30
#
// Jugador
#define ANCHO_JUGADOR 65
#define ALTO_JUGADOR 30
#define VELOCIDAD_JUGADOR 10 //////////
#define INICIO_X_JUGADOR 380
#define INICIO_Y_JUGADOR 810
// Enemigos
#define VELOCIDAD_ENEMIGOS 15 /////////
#define INICIO_ENEMIGOS_X 50
#define INICIO_ENEMIGOS_Y 50
#define ESPACIO_ENTRE_ENEMIGOS 20
#define ANCHO_ENEMIGO 50
#define ALTO_ENEMIGO 50
#define BAJADA_ENEMIGOS 40

// ── TIPOS DE ENEMIGO ────────────────────────────────
#define TIPO_CALAMAR 0
#define TIPO_CANGREJO 1
#define TIPO_PULPO 2
#define TIPO_EXTRATERRESTRE 3

#define FILAS_ENEMIGOS 5
#define COLUMNAS_ENEMIGOS 11
#define TOTAL_ENEMIGOS 44

// OVNI
#define VELOCIDAD_OVNI 10 //////////
#define DELAY_OVNI 3

// ── BALAS ENEMIGAS ──────────────────────────────────
#define MAX_BALAS_ENEMIGAS 4
#define ANCHO_BALA_ENEMIGA 5
#define ALTO_BALA_ENEMIGA 15

// ── BUNKERS ─────────────────────────────────────────
#define NUM_BUNKERS 4
#define BUNKER_FILAS 3
#define BUNKER_COLUMNAS 5
#define BUNKER_LADO_BLOQUE 20
#define BUNKER_TOTAL_BLOQUES (BUNKER_FILAS * BUNKER_COLUMNAS)

// Control
#define USAR_CONTROL_PICO 1
#define PUERTO_PICO "\\\\.\\COM6"
#define BAUDIOS_PICO 115200

#endif