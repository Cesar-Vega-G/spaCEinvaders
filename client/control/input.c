#include "input.h"
#include "../constantes.h"
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
static HANDLE puertoPico = INVALID_HANDLE_VALUE;
#endif

void inicializarControlPico()
{
#if USAR_CONTROL_PICO
#ifdef _WIN32
    puertoPico = CreateFileA(
        PUERTO_PICO,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (puertoPico == INVALID_HANDLE_VALUE)
    {
        SDL_Log("No se pudo abrir el puerto de la Pico: %s", PUERTO_PICO);
        return;
    }

    DCB config;
    SecureZeroMemory(&config, sizeof(config));
    config.DCBlength = sizeof(config);

    if (!GetCommState(puertoPico, &config))
    {
        SDL_Log("No se pudo leer la configuracion del puerto Pico");
        CloseHandle(puertoPico);
        puertoPico = INVALID_HANDLE_VALUE;
        return;
    }

    config.BaudRate = BAUDIOS_PICO;
    config.ByteSize = 8;
    config.StopBits = ONESTOPBIT;
    config.Parity = NOPARITY;

    if (!SetCommState(puertoPico, &config))
    {
        SDL_Log("No se pudo configurar el puerto Pico");
        CloseHandle(puertoPico);
        puertoPico = INVALID_HANDLE_VALUE;
        return;
    }

    COMMTIMEOUTS tiempos;
    SecureZeroMemory(&tiempos, sizeof(tiempos));

    tiempos.ReadIntervalTimeout = 1;
    tiempos.ReadTotalTimeoutConstant = 1;
    tiempos.ReadTotalTimeoutMultiplier = 1;

    SetCommTimeouts(puertoPico, &tiempos);

    // Limpia datos viejos y activa señales de control del puerto serial
    PurgeComm(puertoPico, PURGE_RXCLEAR | PURGE_TXCLEAR);
    EscapeCommFunction(puertoPico, SETDTR);
    EscapeCommFunction(puertoPico, SETRTS);

    printf("[PICO] Control Pico conectado en %s\n", PUERTO_PICO);
    fflush(stdout);
    SDL_Log("Control Pico conectado en %s", PUERTO_PICO);
#endif
#endif
}

void cerrarControlPico()
{
#if USAR_CONTROL_PICO
#ifdef _WIN32
    if (puertoPico != INVALID_HANDLE_VALUE)
    {
        CloseHandle(puertoPico);
        puertoPico = INVALID_HANDLE_VALUE;
    }
#endif
#endif
}

static void procesarComandoPico(Conexion *conexion)
{
#if USAR_CONTROL_PICO
#ifdef _WIN32
    if (puertoPico == INVALID_HANDLE_VALUE)
    {
        return;
    }

    if (conexion->idJugador < 0)
    {
        return;
    }

    DWORD errores;
    COMSTAT estado;

    ClearCommError(puertoPico, &errores, &estado);

    if (estado.cbInQue > 0)
    {
        printf("[PICO] Bytes disponibles: %lu\n", estado.cbInQue);
        fflush(stdout);
    }

    char dato;
    DWORD leidos = 0;

    while (estado.cbInQue > 0)
    {
        if (!ReadFile(puertoPico, &dato, 1, &leidos, NULL))
        {
            printf("[PICO] ERROR leyendo puerto\n");
            fflush(stdout);
            break;
        }

        if (leidos == 0)
        {
            break;
        }

        printf("[PICO] Dato recibido: %c\n", dato);
        fflush(stdout);

        switch (dato)
        {
        case 'I':
            printf("[PICO] Enviando al servidor: MOVER_IZQ\n");
            fflush(stdout);
            enviarMensaje(conexion, "MOVER_IZQ");
            break;

        case 'D':
            printf("[PICO] Enviando al servidor: MOVER_DER\n");
            fflush(stdout);
            enviarMensaje(conexion, "MOVER_DER");
            break;

        case 'F':
            printf("[PICO] Enviando al servidor: DISPARAR\n");
            fflush(stdout);
            enviarMensaje(conexion, "DISPARAR");
            break;

        default:
            printf("[PICO] Dato desconocido: %c\n", dato);
            fflush(stdout);
            break;
        }

        ClearCommError(puertoPico, &errores, &estado);
    }
#endif
#endif
}

void procesarInput(SDL_Event *evento, int *jugando, Conexion *conexion)
{
    while (SDL_PollEvent(evento))
    {
        if (evento->type == SDL_QUIT)
        {
            *jugando = 0;
        }

        if (evento->type == SDL_KEYDOWN)
        {
            int id = conexion->idJugador;

            switch (evento->key.keysym.sym)
            {

            // Jugador 0: A / D / ESPACIO
            case SDLK_a:
                if (id == 0)
                    enviarMensaje(conexion, "MOVER_IZQ");
                break;

            case SDLK_d:
                if (id == 0)
                    enviarMensaje(conexion, "MOVER_DER");
                break;

            case SDLK_SPACE:
                if (id == 0)
                    enviarMensaje(conexion, "DISPARAR");
                break;

            // Jugador 1: flechas
            case SDLK_LEFT:
                if (id == 1)
                    enviarMensaje(conexion, "MOVER_IZQ");
                break;

            case SDLK_RIGHT:
                if (id == 1)
                    enviarMensaje(conexion, "MOVER_DER");
                break;

            case SDLK_UP:
                if (id == 1)
                    enviarMensaje(conexion, "DISPARAR");
                break;

            case SDLK_ESCAPE:
                *jugando = 0;
                break;

            default:
                break;
            }
        }
    }

    procesarComandoPico(conexion);
}