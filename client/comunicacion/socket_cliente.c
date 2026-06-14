#include "socket_cliente.h"
#include <stdio.h>
#include <string.h>

// ── CREAR CONEXION ──────────────────────────────────
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

    u_long modo = 1;
    ioctlsocket(conexion.socket, FIONBIO, &modo);

    return conexion;
}

// ── CONECTAR AL SERVIDOR ────────────────────────────
int conectarServidor(Conexion* conexion, const char* ip, int puerto, int usarPico) {
    struct sockaddr_in direccion;
    direccion.sin_family      = AF_INET;
    direccion.sin_port        = htons(puerto);
    direccion.sin_addr.s_addr = inet_addr(ip);

    connect(conexion->socket, (struct sockaddr*)&direccion, sizeof(direccion));
    SDL_Delay(100);  // Dar tiempo a que el OS complete el handshake TCP

    const char* rol;
    if (conexion->esEspectador)      rol = "ESPECTADOR\n";
    else if (usarPico)               rol = "JUGADOR_PICO\n";
    else                             rol = "JUGADOR_TECLADO\n";

    send(conexion->socket, rol, strlen(rol), 0);

    conexion->conectado = 1;
    return 1;
}

// ── VERIFICAR SLOT ──────────────────────────────────
// Lee la primera respuesta del servidor (BIENVENIDO X o SLOT_OCUPADO).
// Devuelve 1 si OK (y setea idJugador), 0 si el slot ya está ocupado.
int verificarSlot(Conexion* conexion) {
    // Cambiar a bloqueante con timeout de 3 s
    u_long bloqueante = 0;
    ioctlsocket(conexion->socket, FIONBIO, &bloqueante);
    DWORD tmo = 3000;
    setsockopt(conexion->socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tmo, sizeof(tmo));

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

    // Volver a no-bloqueante sin timeout
    u_long nb = 1;
    ioctlsocket(conexion->socket, FIONBIO, &nb);
    DWORD noTmo = 0;
    setsockopt(conexion->socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&noTmo, sizeof(noTmo));

    if (strncmp(linea, "SLOT_OCUPADO", 12) == 0) return 0;

    // "BIENVENIDO X" — extraer id
    int id;
    if (sscanf(linea, "BIENVENIDO %d", &id) == 1)
        conexion->idJugador = id;

    return 1;
}

// ── ELEGIR PARTIDA (solo espectadores) ─────────────
// Lee una línea del socket bloqueante, carácter a carácter.
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

int elegirPartida(Conexion* conexion) {
    // Cambiar a modo bloqueante para leer la lista del servidor
    u_long bloqueante = 0;
    ioctlsocket(conexion->socket, FIONBIO, &bloqueante);

    // Leer "PARTIDAS n"
    char linea[256];
    leerLinea(conexion->socket, linea, sizeof(linea));

    int n = 0;
    sscanf(linea, "PARTIDAS %d", &n);

    // Leer cada "PARTIDA id"
    int ids[64];
    for (int i = 0; i < n && i < 64; i++) {
        char pl[256];
        leerLinea(conexion->socket, pl, sizeof(pl));
        ids[i] = -1;
        sscanf(pl, "PARTIDA %d", &ids[i]);
    }

    // Volver a modo no bloqueante
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

// ── ENVIAR MENSAJE ──────────────────────────────────
void enviarMensaje(Conexion* conexion, const char* mensaje) {
    if (!conexion->conectado) return;
    send(conexion->socket, mensaje, strlen(mensaje), 0);
    send(conexion->socket, "\n", 1, 0);
}

// ── RECIBIR ESTADO ──────────────────────────────────
int recibirEstado(Conexion* conexion, char* buffer, int tamano) {
    if (!conexion->conectado) return 0;
    int bytesRecibidos = recv(conexion->socket, buffer, tamano - 1, 0);
    if (bytesRecibidos > 0) {
        buffer[bytesRecibidos] = '\0';
        return 1;
    }
    return 0;
}


// ── CERRAR CONEXION ─────────────────────────────────
void cerrarConexion(Conexion* conexion) {
    if (conexion->conectado) {
        closesocket(conexion->socket);
        WSACleanup();
        conexion->conectado = 0;
    }
}