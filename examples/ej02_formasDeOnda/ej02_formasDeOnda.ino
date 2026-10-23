// tocar la misma nota con cada forma de onda,
// para escuchar como cambia el sonido

// solo funciona en placas con modo MUESTRAS:
// Arduino Uno R4 (Minima o WiFi), y placas SAMD21 con I2S
// (por ejemplo Arduino Zero o MKR Zero)

// conexiones en Uno R4:
// A0 a la entrada de un modulo amplificador (por ejemplo PAM8302),
// y el parlante al amplificador. nunca conectes el parlante directo a A0

// conexiones en SAMD21, con un conversor I2S (por ejemplo MAX98357A):
// VIN a 3.3V, GND a tierra (GND), y el parlante a las salidas + y -
// en MKR Zero: DIN a A6, BCLK a 2, LRC a 3
// en Arduino Zero: DIN a 9, BCLK a 1, LRC a 0

// incluir biblioteca
#include "Bocina.h"

// crear una bocina. la patita 8 solo se usa en modo TONOS
Bocina bocina(8);

// las formas, y sus nombres para imprimirlos
const Bocina::Forma formas[] = {
    Bocina::SENO, Bocina::TRIANGULO, Bocina::SIERRA, Bocina::CUADRADA, Bocina::RUIDO};
const char *nombres[] = {"seno", "triangulo", "sierra", "cuadrada", "ruido"};

void setup()
{
  // abrir comunicacion serial
  Serial.begin(9600);

  // pasar al modo MUESTRAS. si la placa no lo tiene, avisar y no seguir
  if (!bocina.setModo(Bocina::MUESTRAS))
  {
    while (true)
    {
      Serial.println("esta placa no tiene modo MUESTRAS");
      delay(1000);
    }
  }

  // volumen de 0 a 255
  bocina.setVolumen(100);
}

void loop()
{
  for (uint8_t i = 0; i < 5; i++)
  {
    Serial.println(nombres[i]);

    // cambiar la forma y tocar 220 Hz durante un segundo
    bocina.setForma(formas[i]);
    bocina.sonar(220, 1000);

    delay(300);
  }
}
