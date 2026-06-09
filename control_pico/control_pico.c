#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include <stdio.h>

#define BTN_MOVIMIENTO 15
#define BTN_DISPARO 2

#define TIEMPO_DOBLE_TOQUE_MS 300
#define DEBOUNCE_MS 50

int botonPresionado(uint pin)
{
    return gpio_get(pin) == 0;
}

int main()
{
    stdio_init_all();

    gpio_init(BTN_MOVIMIENTO);
    gpio_set_dir(BTN_MOVIMIENTO, GPIO_IN);
    gpio_pull_up(BTN_MOVIMIENTO);

    gpio_init(BTN_DISPARO);
    gpio_set_dir(BTN_DISPARO, GPIO_IN);
    gpio_pull_up(BTN_DISPARO);

    while (!stdio_usb_connected())
    {
        sleep_ms(100);
    }

    absolute_time_t tiempoPrimerToque = get_absolute_time();
    int esperandoSegundoToque = 0;

    while (true)
    {

        if (botonPresionado(BTN_DISPARO))
        {
            printf("F");
            fflush(stdout);

            sleep_ms(DEBOUNCE_MS);

            while (botonPresionado(BTN_DISPARO))
            {
                sleep_ms(10);
            }
        }

        if (botonPresionado(BTN_MOVIMIENTO))
        {
            sleep_ms(DEBOUNCE_MS);

            while (botonPresionado(BTN_MOVIMIENTO))
            {
                sleep_ms(10);
            }

            if (!esperandoSegundoToque)
            {
                esperandoSegundoToque = 1;
                tiempoPrimerToque = get_absolute_time();
            }
            else
            {
                printf("D");
                fflush(stdout);
                esperandoSegundoToque = 0;
            }
        }

        if (esperandoSegundoToque)
        {
            int64_t diferenciaMs =
                absolute_time_diff_us(tiempoPrimerToque, get_absolute_time()) / 1000;

            if (diferenciaMs > TIEMPO_DOBLE_TOQUE_MS)
            {
                printf("I");
                fflush(stdout);
                esperandoSegundoToque = 0;
            }
        }

        sleep_ms(5);
    }

    return 0;
}