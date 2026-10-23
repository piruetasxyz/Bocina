// alternar dos frecuencias sin bloquear el programa,
// mientras el led de la placa parpadea

// conexiones:
// buzzer pasivo o piezo: una patita a la patita 8, la otra a tierra (GND)

// incluir biblioteca
#include "Bocina.h"

// crear una bocina en la patita 8
Bocina bocina(8);

// las dos frecuencias, como una sirena
uint16_t frecuencias[] = {440, 660};
uint8_t actual = 0;

// para el parpadeo del led
unsigned long ultimoCambioLed = 0;
bool ledEncendido = false;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
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
  if (millis() - ultimoCambioLed >= 300)
  {
    ultimoCambioLed = millis();
    ledEncendido = !ledEncendido;
    digitalWrite(LED_BUILTIN, ledEncendido);
  }
}
