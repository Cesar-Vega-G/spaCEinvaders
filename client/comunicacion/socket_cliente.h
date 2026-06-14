#ifndef SOCKET_CLIENTE_H
#define SOCKET_CLIENTE_H

#include <winsock2.h>
#include <SDL2/SDL.h>
#include "../constantes.h"

typedef struct {
    SOCKET socket;
    int conectado;
    int idJugador;   // 0 = jugador, -1 = espectador
    int esEspectador;
} Conexion;

Conexion crearConexion();
int  conectarServidor(Conexion* conexion, const char* ip, int puerto, int usarPico);
int  verificarSlot(Conexion* conexion);
int  elegirPartida(Conexion* conexion);
void enviarMensaje(Conexion* conexion, const char* mensaje);
int  recibirEstado(Conexion* conexion, char* buffer, int tamano);
void cerrarConexion(Conexion* conexion);

#endif