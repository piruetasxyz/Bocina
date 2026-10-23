// tocar la misma nota con cada forma de onda,
// para escuchar como cambia el sonido

// conexiones, con un conversor I2S (por ejemplo MAX98357A):
// VIN a 3V3, GND a tierra (GND), DIN a gpio 9, BCLK a gpio 10,
// LRC a gpio 11, y el parlante a las salidas + y -

#include "Bocina.h"

#include <stdio.h>

#include "pico/stdlib.h"

// las formas, y sus nombres para imprimirlos
const Bocina::Forma formas[] = {
    Bocina::SENO, Bocina::TRIANGULO, Bocina::SIERRA, Bocina::CUADRADA, Bocina::RUIDO};
const char *nombres[] = {"seno", "triangulo", "sierra", "cuadrada", "ruido"};

int main()
{
    // abrir comunicacion serial por usb
    stdio_init_all();

    // crear una bocina. gpio 15 solo se usa en modo TONOS
    Bocina bocina(15);

    // pasar al modo MUESTRAS, por i2s.
    // para usar otras patitas, llamar antes a bocina.setPatitasI2S(datos, reloj)
    bocina.setModo(Bocina::MUESTRAS);

    // volumen de 0 a 255
    bocina.setVolumen(100);

    while (true)
    {
        for (uint8_t i = 0; i < 5; i++)
        {
            printf("%s\n", nombres[i]);

            // cambiar la forma y tocar 220 Hz durante un segundo
            bocina.setForma(formas[i]);
            bocina.sonar(220, 1000);

            sleep_ms(300);
        }
    }
}
