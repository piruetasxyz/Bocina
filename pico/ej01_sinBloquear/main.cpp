// alternar dos frecuencias sin bloquear el programa,
// mientras el led de la placa parpadea

// conexiones:
// buzzer pasivo o piezo: una patita a gpio 15, la otra a tierra (GND)

#include "Bocina.h"

#include "pico/stdlib.h"

int main()
{
    // led de la placa
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // crear una bocina en gpio 15
    Bocina bocina(15);

    // las dos frecuencias, como una sirena
    uint16_t frecuencias[] = {440, 660};
    uint8_t actual = 0;

    // para el parpadeo del led
    uint32_t ultimoCambioLed = 0;
    bool ledEncendido = false;

    while (true)
    {
        // terminar el sonido cuando se cumple su duracion
        bocina.actualizar();

        // cuando termina, tocar la otra frecuencia durante 400 milisegundos
        if (!bocina.sonando())
        {
            bocina.tocar(frecuencias[actual], 400);
            actual = 1 - actual;
        }

        // el led sigue parpadeando, porque tocar() no bloquea
        uint32_t ahora = to_ms_since_boot(get_absolute_time());
        if (ahora - ultimoCambioLed >= 300)
        {
            ultimoCambioLed = ahora;
            ledEncendido = !ledEncendido;
            gpio_put(PICO_DEFAULT_LED_PIN, ledEncendido);
        }
    }
}
