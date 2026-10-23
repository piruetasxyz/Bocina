// calcular una grabacion al partir y tocarla una y otra vez.
// es una gota: una onda seno que baja de tono y se apaga

// solo funciona en placas con modo MUESTRAS:
// Arduino Uno R4 (Minima o WiFi), y placas SAMD21 con I2S
// (por ejemplo Arduino Zero o MKR Zero)

// conexiones: las mismas que en ej02_formasDeOnda

// incluir biblioteca
#include "Bocina.h"

// crear una bocina. la patita 8 solo se usa en modo TONOS
Bocina bocina(8);

// 0.3 segundos de sonido.
// cada muestra ocupa 2 bytes, asi que son unos 13 KB de memoria
const uint32_t largo = Bocina::FRECUENCIA_MUESTREO * 3 / 10;
int16_t grabacion[largo];

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

  // calcular la gota, muestra por muestra
  float fase = 0;
  for (uint32_t i = 0; i < largo; i++)
  {
    // t va de 0 al principio a 1 al final
    float t = (float)i / largo;

    // la frecuencia baja de 1200 a 200 Hz, y el volumen de 1 a 0
    float frecuencia = 1200 - 1000 * t;
    float amplitud = 1 - t;

    grabacion[i] = sin(fase) * amplitud * 32767;

    // avanzar la fase lo que corresponde a una muestra
    fase += 2 * PI * frecuencia / Bocina::FRECUENCIA_MUESTREO;
  }
}

void loop()
{
  // cuando termina la gota, esperar medio segundo y tocarla de nuevo.
  // tocarMuestras() no bloquea: aqui puedes hacer otras cosas
  if (!bocina.sonando())
  {
    delay(500);
    bocina.tocarMuestras(grabacion, largo);
  }
}
