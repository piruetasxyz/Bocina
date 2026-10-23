# Bocina

Biblioteca para hacer sonar bocinas con microcontroladores: tonos con onda cuadrada, formas de onda y grabaciones.

Programada en C++, diseñada para uso con placas de desarrollo de las marcas Arduino y Raspberry Pi Pico.

## Modos

La bocina tiene dos modos:

- `Bocina::TONOS`: ondas cuadradas en cualquier patita, para un buzzer pasivo o un piezo. Funciona en todas las placas. Es el modo inicial.
- `Bocina::MUESTRAS`: la biblioteca calcula cada muestra del sonido, a 22050 muestras por segundo. Así puede tocar ondas seno, triángulo, sierra, cuadrada y ruido, y también grabaciones.

El modo `MUESTRAS` depende de la placa:

| Placa | Salida |
| --- | --- |
| Raspberry Pi Pico y Pico 2, con Pico SDK | conversor I2S, por ejemplo MAX98357A |
| Arduino SAMD21 con I2S, por ejemplo MKR Zero o Arduino Zero | conversor I2S, por ejemplo MAX98357A |
| Arduino Uno R4, Minima o WiFi | conversor análogo de la patita A0, a un amplificador |
| las demás, por ejemplo Arduino Uno | no tiene, `setModo(Bocina::MUESTRAS)` devuelve `false` |

## Conexiones

### Modo TONOS

Un buzzer pasivo o un piezo, con una patita a la placa y la otra a tierra (GND). Un buzzer activo (el que suena solo con conectarlo) no sirve, porque siempre suena a la misma frecuencia.

### Modo MUESTRAS con I2S

Un módulo conversor I2S, por ejemplo MAX98357A, con VIN a 3.3V, GND a tierra, y el parlante a sus salidas + y -. Las patitas de datos van así:

| Conversor | Pico SDK | MKR Zero | Arduino Zero |
| --- | --- | --- | --- |
| DIN | GPIO 9 | A6 | 9 |
| BCLK | GPIO 10 | 2 | 1 |
| LRC | GPIO 11 | 3 | 0 |

En Pico se pueden cambiar con `setPatitasI2S(datos, reloj)`, antes de `setModo()`. LRC va siempre en la patita siguiente a BCLK. En SAMD21 las patitas son fijas.

### Modo MUESTRAS en Uno R4

La patita A0 a la entrada de un módulo amplificador, por ejemplo PAM8302, y el parlante al amplificador. Nunca conectes un parlante directo a una patita.

## Uso

Para importar la biblioteca

```cpp
#include "Bocina.h"
```

Para declarar una instancia llamada bocina

```cpp
// bocina conectada a la patita 8
Bocina bocina(8);
```

Para tocar 440 Hz durante medio segundo, esperando a que termine

```cpp
bocina.sonar(440, 500);
```

## Sin bloquear

`sonar()` detiene el programa mientras suena. Si necesitas hacer otras cosas a la vez, usa `tocar()`: empieza el sonido y vuelve de inmediato. Hay que llamar a `actualizar()` en cada vuelta del loop, para que el sonido termine a tiempo.

```cpp
void loop()
{
  bocina.actualizar();

  if (!bocina.sonando())
  {
    bocina.tocar(440, 400);
  }

  // aquí puedes hacer otras cosas
}
```

Sin duración, `tocar(440)` suena hasta llamar a `silenciar()`. Una frecuencia 0 es un silencio que igual dura.

Ejemplo en [examples/ej01_sinBloquear/](./examples/ej01_sinBloquear/).

## Formas de onda

En modo `MUESTRAS` se puede elegir la forma de onda y el volumen. Hay que cambiar de modo en `setup()`, no al crear la bocina.

```cpp
void setup()
{
  if (!bocina.setModo(Bocina::MUESTRAS))
  {
    // esta placa no tiene modo MUESTRAS
  }

  bocina.setForma(Bocina::SENO);
  bocina.setVolumen(100);
}
```

Las formas son `Bocina::CUADRADA`, `Bocina::SENO`, `Bocina::TRIANGULO`, `Bocina::SIERRA` y `Bocina::RUIDO`. El volumen va de 0 a 255, y parte en 128.

Ejemplo en [examples/ej02_formasDeOnda/](./examples/ej02_formasDeOnda/).

## Grabaciones

En modo `MUESTRAS`, `tocarMuestras()` toca un arreglo de muestras de -32768 a 32767, a `Bocina::FRECUENCIA_MUESTREO` (22050) muestras por segundo. No bloquea el programa, y el arreglo no se copia, así que tiene que existir mientras suena.

```cpp
bocina.tocarMuestras(grabacion, largo);
```

Ejemplo en [examples/ej03_grabacion/](./examples/ej03_grabacion/), que calcula una gota al partir.

## Uso con Raspberry Pi Pico SDK

La carpeta [pico/](./pico/) tiene un proyecto CMake que compila la biblioteca y los ejemplos para Pico 2. Necesitas la [Pico SDK](https://github.com/raspberrypi/pico-sdk) instalada, o la extensión Raspberry Pi Pico de VS Code.

```bash
cd pico
mkdir build
cd build
PICO_SDK_PATH=/ruta/a/pico-sdk cmake ..
cmake --build .
```

Esto genera un archivo `.uf2` por ejemplo, como `ej00_sonar.uf2`. Para cargarlo, conecta la Pico manteniendo presionado el botón BOOTSEL y copia el archivo a la unidad que aparece.

En Pico, los tonos salen por PWM, y el I2S por un programa PIO propio, [pico/bocina_i2s.pio](./pico/bocina_i2s.pio), que solo necesita la Pico SDK. El modo `MUESTRAS` usa una máquina de estados PIO y un canal DMA.

Para otra placa, cambia `PICO_BOARD` en [pico/CMakeLists.txt](./pico/CMakeLists.txt) (por ejemplo `pico` para la Pico original).

## Ejemplos

Cada ejemplo existe para Arduino, en [examples/](./examples/), y para Pico SDK, en [pico/](./pico/). En Arduino el buzzer está conectado a la patita 8, y en Pico a GPIO 15.

| Ejemplo | Qué hace |
| --- | --- |
| `ej00_sonar` | toca tres frecuencias, una después de otra |
| `ej01_sinBloquear` | alterna dos frecuencias sin bloquear, mientras el led de la placa parpadea |
| `ej02_formasDeOnda` | toca 220 Hz con cada forma de onda, en modo `MUESTRAS` |
| `ej03_grabacion` | calcula una grabación al partir y la toca una y otra vez, en modo `MUESTRAS` |

## Referencia

| Método | Descripción |
| --- | --- |
| `Bocina(uint8_t patita)` | crea una bocina en modo `TONOS` y configura la patita como salida |
| `void setPatita(uint8_t patita)` | cambia la patita del modo `TONOS` |
| `bool setModo(Modo modo)` | cambia a `Bocina::TONOS` o `Bocina::MUESTRAS`. Devuelve `false` si la placa no tiene el modo |
| `Modo getModo()` | modo actual |
| `void setPatitasI2S(uint8_t datos, uint8_t reloj)` | patitas I2S en Pico SDK. LRC va en `reloj + 1`. Antes de `setModo()` |
| `void setForma(Forma forma)` | forma de onda del modo `MUESTRAS` |
| `Forma getForma()` | forma de onda actual |
| `void setVolumen(uint8_t volumen)` | volumen del modo `MUESTRAS`, de 0 a 255 |
| `uint8_t getVolumen()` | volumen actual |
| `void sonar(uint16_t frecuencia, uint32_t duracion)` | toca una frecuencia en Hz durante una duración en milisegundos, y espera a que termine |
| `void tocar(uint16_t frecuencia, uint32_t duracion)` | empieza a tocar sin bloquear. Sin duración, suena hasta `silenciar()` |
| `bool tocarMuestras(const int16_t *muestras, uint32_t cantidad)` | toca una grabación sin bloquear, solo en modo `MUESTRAS` |
| `void actualizar()` | termina el sonido cuando se cumple su duración. Hay que llamarla en cada vuelta del loop |
| `bool sonando()` | `true` mientras hay algo sonando |
| `void silenciar()` | detiene lo que esté sonando |

## Documentación

La documentación se genera con [Doxygen](https://www.doxygen.nl/) a partir de los comentarios en [src/](./src/), en español y en inglés, y se publica en <https://piruetasxyz.github.io/Bocina/>.

## Cómo está organizado

El código de [src/Bocina.cpp](./src/Bocina.cpp) no tiene dependencias. Ahí se calculan las formas de onda, con aritmética de enteros.

El código en [src/Hardware.h](./src/Hardware.h) contiene una implementación por placa:

- [src/arduino/ArduinoHardware.cpp](./src/arduino/ArduinoHardware.cpp): usa `tone` y `millis`, la biblioteca `I2S` en SAMD21, y `FspTimer` con `analogWrite` en Uno R4.
- [pico/PicoHardware.cpp](./pico/PicoHardware.cpp): usa PWM para los tonos, y PIO con DMA para el I2S.

Para agregar otra plataforma, basta con escribir otra implementación de `Hardware.h`. Si la plataforma no tiene salida de audio, `iniciarMuestras()` puede devolver `false`.

## Contenido

- [.github/](./.github): flujos de trabajo que compilan los ejemplos de distintas placas, revisan la biblioteca con Arduino Lint, y publican la documentación.
- [docs/](./docs/): página principal de la documentación y página para elegir idioma.
- [examples/](./examples/): ejemplos para Arduino.
- [Doxyfile](./Doxyfile): configuración de Doxygen.
- [pico/](./pico/): proyecto CMake, implementación, programa PIO y ejemplos para Pico SDK.
- [src/](./src/): código de la biblioteca.
- [keywords.txt](./keywords.txt): indicadores de sintaxis para Arduino IDE.
- [library.properties](./library.properties): metadatos para Arduino IDE.
- [LICENSE](./LICENSE): licencia MIT.
- [README.md](./README.md): este documento.

## Versiones

- v0.0.1: octubre 2026, primera versión, para Arduino y Raspberry Pi Pico SDK, con modo `TONOS` en todas las placas, y modo `MUESTRAS` por I2S en Pico SDK y SAMD21, y por el conversor análogo en Uno R4.

## Licencia

MIT, ver [LICENSE](./LICENSE).
