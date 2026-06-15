/*
 * input.c — Manejo de entrada del jugador.
 *
 * Soporta dos modos de control mutuamente excluyentes:
 *   · Teclado (A/D/ESPACIO): activo cuando el Pico NO está conectado.
 *   · Raspberry Pi Pico vía puerto serial (COM15): envía caracteres
 *     'I' (izquierda), 'D' (derecha), 'F' (fuego).
 *
 * Cuando el Pico está conectado el teclado queda deshabilitado para
 * controles de juego; ESC sigue funcionando para salir.
 */

#include "input.h"
#include "../constantes.h"
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* Handle del puerto serial del Pico; INVALID_HANDLE_VALUE = no conectado. */
#ifdef _WIN32
static HANDLE puertoPico = INVALID_HANDLE_VALUE;
#endif

/* ── INICIALIZAR CONTROL PICO ────────────────────────────────────────────── */
void inicializarControlPico()
{
#if USAR_CONTROL_PICO
#ifdef _WIN32
    /* Abrir el puerto serial en modo lectura exclusiva. */
    puertoPico = CreateFileA(
        PUERTO_PICO,
        GENERIC_READ,
        0,           /* sin compartir: otro proceso no puede abrir el mismo puerto */
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (puertoPico == INVALID_HANDLE_VALUE)
    {
        /* Mostrar popup con el error exacto (código Win32) para facilitar el debug. */
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

    /* Configurar baudios, bits de datos, paridad y stop bits. */
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
    config.Parity   = NOPARITY;

    if (!SetCommState(puertoPico, &config))
    {
        SDL_Log("No se pudo configurar el puerto Pico");
        CloseHandle(puertoPico);
        puertoPico = INVALID_HANDLE_VALUE;
        return;
    }

    /* Timeouts mínimos: leer sin bloquear el game loop. */
    COMMTIMEOUTS tiempos;
    SecureZeroMemory(&tiempos, sizeof(tiempos));
    tiempos.ReadIntervalTimeout         = 1;
    tiempos.ReadTotalTimeoutConstant    = 1;
    tiempos.ReadTotalTimeoutMultiplier  = 1;
    SetCommTimeouts(puertoPico, &tiempos);

    /* Limpiar buffer y activar señales DTR/RTS para que el Pico detecte la conexión. */
    PurgeComm(puertoPico, PURGE_RXCLEAR | PURGE_TXCLEAR);
    EscapeCommFunction(puertoPico, SETDTR);
    EscapeCommFunction(puertoPico, SETRTS);

    printf("[PICO] Control Pico conectado en %s\n", PUERTO_PICO);
    fflush(stdout);
#endif
#endif
}

/* ── CERRAR CONTROL PICO ─────────────────────────────────────────────────── */
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

/* ── PROCESAR COMANDOS DEL PICO ──────────────────────────────────────────── */
/*
 * Lee todos los bytes disponibles en el buffer serial del Pico y traduce
 * cada carácter a un comando de juego enviado al servidor.
 * Caracteres esperados: 'I' = izquierda, 'D' = derecha, 'F' = fuego.
 */
static void procesarComandoPico(Conexion *conexion)
{
#if USAR_CONTROL_PICO
#ifdef _WIN32
    if (puertoPico == INVALID_HANDLE_VALUE) return;
    if (conexion->idJugador < 0) return; /* espectadores no envían comandos */

    DWORD errores;
    COMSTAT estado;
    ClearCommError(puertoPico, &errores, &estado);

    char dato;
    DWORD leidos = 0;

    /* Vaciar el buffer serial: procesar todos los bytes acumulados. */
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

/* ── PROCESAR INPUT (teclado + Pico) ─────────────────────────────────────── */
void procesarInput(SDL_Event *evento, int *jugando, Conexion *conexion)
{
    while (SDL_PollEvent(evento))
    {
        if (evento->type == SDL_QUIT)
            *jugando = 0;

        if (evento->type == SDL_KEYDOWN)
        {
            SDL_Keycode sym = evento->key.keysym.sym;

            /* ESC siempre disponible para salir, incluso con Pico activo. */
            if (sym == SDLK_ESCAPE)
                *jugando = 0;

            /*
             * Controles de teclado habilitados solo cuando el Pico NO está
             * conectado, para evitar que ambos controles interfieran.
             */
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

    /* Procesar los datos del Pico después de los eventos SDL. */
    procesarComandoPico(conexion);
}
