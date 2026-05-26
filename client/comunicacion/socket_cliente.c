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
int conectarServidor(Conexion* conexion, const char* ip, int puerto) {
    struct sockaddr_in direccion;
    direccion.sin_family      = AF_INET;
    direccion.sin_port        = htons(puerto);
    direccion.sin_addr.s_addr = inet_addr(ip);

    connect(conexion->socket, (struct sockaddr*)&direccion, sizeof(direccion));
    SDL_Delay(100);  // Dar tiempo a que el OS complete el handshake TCP

    // Enviar rol como primer mensaje para que el servidor asigne partida correcta
    const char* rol = conexion->esEspectador ? "ESPECTADOR\n" : "JUGADOR\n";
    send(conexion->socket, rol, strlen(rol), 0);

    conexion->conectado = 1;
    printf("Conectado al servidor %s:%d como %s\n",
           ip, puerto, conexion->esEspectador ? "ESPECTADOR" : "JUGADOR");
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