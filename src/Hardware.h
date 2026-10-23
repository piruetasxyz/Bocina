#ifndef BOCINA_HARDWARE_H
#define BOCINA_HARDWARE_H

#include <stdint.h>

/**
 * \~spanish
 * @brief Funciones que tocan el hardware, con una implementacion por plataforma.
 *
 * Bocina.cpp solo usa estas funciones, asi no depende de ninguna plataforma.
 * Las implementaciones estan en src/arduino/ArduinoHardware.cpp y
 * pico/PicoHardware.cpp.
 *
 * Las funciones de tonos son obligatorias. Las de muestras son opcionales:
 * una plataforma sin salida de audio puede hacer que iniciarMuestras()
 * devuelva `false`.
 *
 * \~english
 * @brief Functions that touch the hardware, with one implementation per platform.
 *
 * Bocina.cpp only uses these functions, so it does not depend on any platform.
 * The implementations are in src/arduino/ArduinoHardware.cpp and
 * pico/PicoHardware.cpp.
 *
 * The tone functions are required. The sample functions are optional:
 * a platform without an audio output can make iniciarMuestras() return
 * `false`.
 */
namespace BocinaHardware
{

    /**
     * \~spanish
     * @brief Funcion que llena un bloque de muestras.
     *
     * La plataforma la llama desde una interrupcion cada vez que necesita
     * mas audio. Cada muestra va de -32768 a 32767, en mono.
     *
     * \~english
     * @brief Function that fills a block of samples.
     *
     * The platform calls it from an interrupt every time it needs more
     * audio. Each sample goes from -32768 to 32767, in mono.
     */
    typedef void (*FuncionLlenar)(int16_t *muestras, uint16_t cantidad, void *contexto);

    /**
     * \~spanish
     * @brief Configura la patita como salida de tonos.
     * @param patita patita a configurar.
     *
     * \~english
     * @brief Configures the pin as a tone output.
     * @param patita pin to configure.
     */
    void configurarSalida(uint8_t patita);

    /**
     * \~spanish
     * @brief Saca una onda cuadrada por la patita, hasta silenciarTono().
     * @param patita patita de salida.
     * @param frecuencia frecuencia en Hz, mayor que 0.
     *
     * \~english
     * @brief Outputs a square wave on the pin, until silenciarTono().
     * @param patita output pin.
     * @param frecuencia frequency in Hz, greater than 0.
     */
    void sonarTono(uint8_t patita, uint16_t frecuencia);

    /**
     * \~spanish
     * @brief Detiene la onda cuadrada y deja la patita en 0.
     * @param patita patita de salida.
     *
     * \~english
     * @brief Stops the square wave and leaves the pin at 0.
     * @param patita output pin.
     */
    void silenciarTono(uint8_t patita);

    /**
     * \~spanish
     * @brief Empieza a sacar muestras de audio.
     *
     * Desde ahi llama a llenar() cada vez que necesita mas muestras, hasta
     * detenerMuestras(). Si ya estaba sacando muestras, cambia la funcion.
     *
     * @param frecuenciaMuestreo muestras por segundo.
     * @param patitaDatos patita de datos I2S, si la plataforma permite elegirla.
     * @param patitaReloj patita de reloj I2S, si la plataforma permite elegirla.
     * @param llenar funcion que calcula las muestras.
     * @param contexto puntero que se le pasa a llenar().
     * @return `false` si la plataforma no puede sacar muestras.
     *
     * \~english
     * @brief Starts outputting audio samples.
     *
     * From then on it calls llenar() every time it needs more samples,
     * until detenerMuestras(). If it was already outputting samples, it
     * changes the function.
     *
     * @param frecuenciaMuestreo samples per second.
     * @param patitaDatos I2S data pin, if the platform lets you choose it.
     * @param patitaReloj I2S clock pin, if the platform lets you choose it.
     * @param llenar function that computes the samples.
     * @param contexto pointer passed to llenar().
     * @return `false` if the platform cannot output samples.
     */
    bool iniciarMuestras(uint32_t frecuenciaMuestreo, uint8_t patitaDatos, uint8_t patitaReloj, FuncionLlenar llenar, void *contexto);

    /**
     * \~spanish
     * @brief Deja de sacar muestras y de llamar a llenar().
     *
     * \~english
     * @brief Stops outputting samples and calling llenar().
     */
    void detenerMuestras();

    /**
     * \~spanish
     * @brief Milisegundos desde que partio la placa.
     * @return milisegundos transcurridos.
     *
     * \~english
     * @brief Milliseconds since the board started.
     * @return elapsed milliseconds.
     */
    uint32_t milisegundos();
}

#endif
