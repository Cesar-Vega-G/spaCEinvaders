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
        char msg[256];
        snprintf(msg, sizeof(msg),
            "No se pudo abrir %s (error %lu)\n\n"
            "Posibles causas:\n"
            "- Otro programa tiene el puerto abierto (Thonny, Arduino IDE)\n"
            "- El Pico no esta conectado\n"
            "- Puerto incorrecto en constantes.h",
            PUERTO_PICO, GetLastError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error Pico", msg, NULL);
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

    char dato;
    DWORD leidos = 0;

    while (estado.cbInQue > 0)
    {
        if (!ReadFile(puertoPico, &dato, 1, &leidos, NULL) || leidos == 0)
            break;

        switch (dato)
        {
        case 'I': enviarMensaje(conexion, "MOVER_IZQ"); break;
        case 'D': enviarMensaje(conexion, "MOVER_DER"); break;
        case 'F': enviarMensaje(conexion, "DISPARAR");  break;
        default:  break;
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
            SDL_Keycode sym = evento->key.keysym.sym;

            if (sym == SDLK_ESCAPE) {
                *jugando = 0;
            }

            // Controles de teclado solo si el Pico no está conectado
            if (puertoPico == INVALID_HANDLE_VALUE)
            {
                switch (sym)
                {
                case SDLK_a:     enviarMensaje(conexion, "MOVER_IZQ"); break;
                case SDLK_d:     enviarMensaje(conexion, "MOVER_DER"); break;
                case SDLK_SPACE: enviarMensaje(conexion, "DISPARAR");  break;
                default: break;
                }
            }
        }
    }

    procesarComandoPico(conexion);
}