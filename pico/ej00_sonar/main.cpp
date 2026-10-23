// tocar tres frecuencias con un buzzer pasivo, una despues de otra

// conexiones:
// buzzer pasivo o piezo: una patita a gpio 15, la otra a tierra (GND)

#include "Bocina.h"

#include "pico/stdlib.h"

int main()
{
    // crear una bocina en gpio 15
    Bocina bocina(15);

    while (true)
    {
        // 440, 550 y 660 Hz, de medio segundo cada una.
        // sonar() espera a que termine
        bocina.sonar(440, 500);
        bocina.sonar(550, 500);
        bocina.sonar(660, 500);

        // un segundo de silencio antes de repetir
        sleep_ms(1000);
    }
}
