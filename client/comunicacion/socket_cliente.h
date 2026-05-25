#ifndef SOCKET_CLIENTE_H
#define SOCKET_CLIENTE_H

#include <winsock2.h>
#include "../constantes.h"

// ── STRUCT CONEXION ─────────────────────────────────
typedef struct {
    SOCKET socket;
    int conectado;
    int idJugador;
} Conexion;

// ── FUNCIONES ───────────────────────────────────────
Conexion crearConexion();
int conectarServidor(Conexion* conexion, const char* ip, int puerto);
void enviarMensaje(Conexion* conexion, const char* mensaje);
void cerrarConexion(Conexion* conexion);

#endif