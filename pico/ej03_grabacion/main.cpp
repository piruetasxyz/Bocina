// calcular una grabacion al partir y tocarla una y otra vez.
// es una gota: una onda seno que baja de tono y se apaga

// conexiones: las mismas que en ej02_formasDeOnda

#include "Bocina.h"

#include <math.h>

#include "pico/stdlib.h"

// 0.3 segundos de sonido.
// cada muestra ocupa 2 bytes, asi que son unos 13 KB de memoria
const uint32_t largo = Bocina::FRECUENCIA_MUESTREO * 3 / 10;
int16_t grabacion[largo];

int main()
{
    // crear una bocina. gpio 15 solo se usa en modo TONOS
    Bocina bocina(15);

    // pasar al modo MUESTRAS, por i2s
    bocina.setModo(Bocina::MUESTRAS);

    // calcular la gota, muestra por muestra
    float fase = 0;
    for (uint32_t i = 0; i < largo; i++)
    {
        // t va de 0 al principio a 1 al final
        float t = (float)i / largo;

        // la frecuencia baja de 1200 a 200 Hz, y el volumen de 1 a 0
        float frecuencia = 1200 - 1000 * t;
        float amplitud = 1 - t;

        grabacion[i] = sinf(fase) * amplitud * 32767;

        // avanzar la fase lo que corresponde a una muestra
        fase += 2 * (float)M_PI * frecuencia / Bocina::FRECUENCIA_MUESTREO;
    }

    while (true)
    {
        // cuando termina la gota, esperar medio segundo y tocarla de nuevo.
        // tocarMuestras() no bloquea: aqui puedes hacer otras cosas
        if (!bocina.sonando())
        {
            sleep_ms(500);
            bocina.tocarMuestras(grabacion, largo);
        }
    }
}
