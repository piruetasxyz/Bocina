// tocar tres frecuencias con un buzzer pasivo, una despues de otra

// conexiones:
// buzzer pasivo o piezo: una patita a la patita 8, la otra a tierra (GND)

// incluir biblioteca
#include "Bocina.h"

// crear una bocina en la patita 8
Bocina bocina(8);

void setup()
{
}

void loop()
{
  // 440, 550 y 660 Hz, de medio segundo cada una.
  // sonar() espera a que termine
  bocina.sonar(440, 500);
  bocina.sonar(550, 500);
  bocina.sonar(660, 500);

  // un segundo de silencio antes de repetir
  delay(1000);
}
