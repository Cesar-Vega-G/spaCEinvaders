#include "socket_cliente.h"
#include <stdio.h>

// ── CREAR CONEXION ──────────────────────────────────
Conexion crearConexion() {
    Conexion conexion;
    conexion.conectado = 0;
    conexion.idJugador = -1;

    // Inicializar Winsock (requerido en Windows)
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

    return conexion;
}

// ── CONECTAR AL SERVIDOR ────────────────────────────
int conectarServidor(Conexion* conexion, const char* ip, int puerto) {
    struct sockaddr_in direccion;
    direccion.sin_family = AF_INET;
    direccion.sin_port   = htons(puerto);
    direccion.sin_addr.s_addr = inet_addr(ip);

    if (connect(conexion->socket, (struct sockaddr*)&direccion, sizeof(direccion)) == SOCKET_ERROR) {
        printf("Error conectando al servidor\n");
        return 0;
    }

    conexion->conectado = 1;
    printf("Conectado al servidor %s:%d\n", ip, puerto);
    return 1;
}

// ── ENVIAR MENSAJE ──────────────────────────────────
void enviarMensaje(Conexion* conexion, const char* mensaje) {
    if (!conexion->conectado) return;
    send(conexion->socket, mensaje, strlen(mensaje), 0);
    send(conexion->socket, "\n", 1, 0);
}

// ── CERRAR CONEXION ─────────────────────────────────
void cerrarConexion(Conexion* conexion) {
    if (conexion->conectado) {
        closesocket(conexion->socket);
        WSACleanup();
        conexion->conectado = 0;
    }
}