/*
 * socket_cliente.c — Comunicación TCP con el servidor Java.
 *
 * Protocolo de conexión:
 *   Cliente → Servidor : "JUGADOR_TECLADO\n" | "JUGADOR_PICO\n" | "ESPECTADOR\n"
 *   Servidor → Cliente : "BIENVENIDO id\n"   (jugadores)
 *                      | "SLOT_OCUPADO\n"     (si ya hay un jugador de ese tipo)
 *                      | "PARTIDAS n\n"       (espectadores)
 *
 * El socket opera en modo NO BLOQUEANTE durante el game loop para que
 * recv() retorne inmediatamente cuando no hay datos. Solo se cambia
 * a BLOQUEANTE durante el handshake inicial (verificarSlot / elegirPartida).
 */

#include "socket_cliente.h"
#include <stdio.h>
#include <string.h>

/* ── CREAR CONEXIÓN ─────────────────────────────────────────────────────── */
Conexion crearConexion() {
    Conexion conexion;
    conexion.conectado    = 0;
    conexion.idJugador    = -1;
    conexion.esEspectador = 0;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Error iniciando Winsock\n");
        return conexion;
    }

    conexion.socket = socket(AF_INET, SOCK_STREAM, 0);
    if (conexion.socket == INVALID_SOCKET) {
        printf("Error creando socket\n");
        WSACleanup();
        return conexion;
    }

    /* Modo no bloqueante: recv() durante el game loop no congela el hilo. */
    u_long modo = 1;
    ioctlsocket(conexion.socket, FIONBIO, &modo);

    return conexion;
}

/* ── CONECTAR AL SERVIDOR ────────────────────────────────────────────────── */
int conectarServidor(Conexion* conexion, const char* ip, int puerto, int usarPico) {
    struct sockaddr_in direccion;
    direccion.sin_family      = AF_INET;
    direccion.sin_port        = htons(puerto);
    direccion.sin_addr.s_addr = inet_addr(ip);

    /* connect() en socket no bloqueante retorna inmediatamente (WSAEWOULDBLOCK).
     * El SDL_Delay da tiempo al OS para completar el handshake TCP de 3 vías. */
    connect(conexion->socket, (struct sockaddr*)&direccion, sizeof(direccion));
    SDL_Delay(100);

    /* Informar al servidor qué rol tomará este cliente. */
    const char* rol;
    if (conexion->esEspectador)  rol = "ESPECTADOR\n";
    else if (usarPico)           rol = "JUGADOR_PICO\n";
    else                         rol = "JUGADOR_TECLADO\n";

    send(conexion->socket, rol, strlen(rol), 0);

    conexion->conectado = 1;
    return 1;
}

/* ── VERIFICAR SLOT ──────────────────────────────────────────────────────── */
/*
 * Lee la primera línea que envía el servidor después del handshake.
 * Puede ser "BIENVENIDO id" (slot libre) o "SLOT_OCUPADO" (rechazado).
 * Retorna 1 si el jugador fue aceptado, 0 si fue rechazado.
 *
 * Se cambia temporalmente a modo BLOQUEANTE con timeout de 3 s para
 * leer carácter a carácter sin espera activa.
 */
int verificarSlot(Conexion* conexion) {
    u_long bloqueante = 0;
    ioctlsocket(conexion->socket, FIONBIO, &bloqueante);
    DWORD tmo = 3000;
    setsockopt(conexion->socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tmo, sizeof(tmo));

    /* Leer una línea completa carácter a carácter. */
    char linea[256];
    int i = 0;
    char c;
    while (i < 255) {
        int n = recv(conexion->socket, &c, 1, 0);
        if (n <= 0) break;
        if (c == '\n') break;
        if (c != '\r') linea[i++] = c;
    }
    linea[i] = '\0';

    /* Restaurar modo no bloqueante para el game loop. */
    u_long nb = 1;
    ioctlsocket(conexion->socket, FIONBIO, &nb);
    DWORD noTmo = 0;
    setsockopt(conexion->socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&noTmo, sizeof(noTmo));

    if (strncmp(linea, "SLOT_OCUPADO", 12) == 0) return 0;

    /* Extraer el id asignado ("BIENVENIDO 0" o "BIENVENIDO 1"). */
    int id;
    if (sscanf(linea, "BIENVENIDO %d", &id) == 1)
        conexion->idJugador = id;

    return 1;
}

/* ── ELEGIR PARTIDA (solo espectadores) ──────────────────────────────────── */

/* Lee una línea completa del socket bloqueante carácter a carácter. */
static void leerLinea(SOCKET s, char* buf, int maxLen) {
    int i = 0;
    char c;
    while (i < maxLen - 1) {
        int n = recv(s, &c, 1, 0);
        if (n <= 0) break;
        if (c == '\n') break;
        if (c != '\r') buf[i++] = c;
    }
    buf[i] = '\0';
}

/*
 * El servidor envía la lista de partidas activas:
 *   "PARTIDAS n\n"
 *   "PARTIDA id\n"  (n veces)
 * El espectador elige una y envía "VER id\n".
 */
int elegirPartida(Conexion* conexion) {
    u_long bloqueante = 0;
    ioctlsocket(conexion->socket, FIONBIO, &bloqueante);

    char linea[256];
    leerLinea(conexion->socket, linea, sizeof(linea));

    int n = 0;
    sscanf(linea, "PARTIDAS %d", &n);

    int ids[64];
    for (int i = 0; i < n && i < 64; i++) {
        char pl[256];
        leerLinea(conexion->socket, pl, sizeof(pl));
        ids[i] = -1;
        sscanf(pl, "PARTIDA %d", &ids[i]);
    }

    u_long noBloqueante = 1;
    ioctlsocket(conexion->socket, FIONBIO, &noBloqueante);

    if (n == 0) {
        printf("No hay partidas activas en este momento.\n");
        return 0;
    }

    printf("\nPartidas disponibles:\n");
    for (int i = 0; i < n; i++)
        printf("  %d. Partida del Jugador %d\n", i + 1, ids[i]);

    printf("Seleccione (1-%d): ", n);
    fflush(stdout);

    int sel = 0;
    scanf("%d", &sel);

    if (sel < 1 || sel > n) {
        printf("Seleccion invalida.\n");
        return 0;
    }

    char msg[32];
    snprintf(msg, sizeof(msg), "VER %d\n", ids[sel - 1]);
    send(conexion->socket, msg, (int)strlen(msg), 0);
    return 1;
}

/* ── ENVIAR MENSAJE ──────────────────────────────────────────────────────── */
/* Envía un comando de juego al servidor (ej. "MOVER_IZQ"). */
void enviarMensaje(Conexion* conexion, const char* mensaje) {
    if (!conexion->conectado) return;
    send(conexion->socket, mensaje, strlen(mensaje), 0);
    send(conexion->socket, "\n", 1, 0);
}

/* ── RECIBIR ESTADO ──────────────────────────────────────────────────────── */
/*
 * Intenta leer el estado serializado del servidor (no bloqueante).
 * El servidor envía un bloque "INICIO_ESTADO\n...\nFIN_ESTADO\n" por frame.
 * Si no hay datos disponibles, retorna 0 sin bloquear el game loop.
 */
int recibirEstado(Conexion* conexion, char* buffer, int tamano) {
    if (!conexion->conectado) return 0;
    int bytesRecibidos = recv(conexion->socket, buffer, tamano - 1, 0);
    if (bytesRecibidos > 0) {
        buffer[bytesRecibidos] = '\0';
        return 1;
    }
    return 0;
}

/* ── CERRAR CONEXIÓN ─────────────────────────────────────────────────────── */
/* Cierra el socket; el servidor detecta EOF y libera el slot del jugador. */
void cerrarConexion(Conexion* conexion) {
    if (conexion->conectado) {
        closesocket(conexion->socket);
        WSACleanup();
        conexion->conectado = 0;
    }
}
