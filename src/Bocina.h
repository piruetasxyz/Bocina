#ifndef BOCINA_H
#define BOCINA_H

#include <stdint.h>

/**
 * \~spanish
 * @brief Hace sonar una bocina: tonos, formas de onda y grabaciones.
 *
 * Tiene dos modos:
 *
 * - `Bocina::TONOS`: ondas cuadradas en una patita, para un buzzer pasivo o
 *   un piezo. Funciona en todas las placas.
 * - `Bocina::MUESTRAS`: la biblioteca calcula cada muestra del sonido, asi
 *   puede tocar ondas seno, triangulo, sierra, cuadrada y ruido, y tambien
 *   grabaciones. En Raspberry Pi Pico (Pico SDK) y en placas SAMD21 (por
 *   ejemplo Arduino Zero o MKR Zero) sale por un conversor I2S, por ejemplo
 *   MAX98357A. En Arduino Uno R4 sale por el conversor analogo de la patita
 *   A0. En las demas placas este modo no existe.
 *
 * Cada sonido se puede tocar de dos formas: sonar() bloquea el programa
 * hasta que termina, y tocar() lo empieza y vuelve de inmediato. Con
 * tocar() hay que llamar a actualizar() en cada vuelta del loop.
 *
 * \~english
 * @brief Plays a speaker: tones, waveforms and recordings.
 *
 * It has two modes:
 *
 * - `Bocina::TONOS`: square waves on a pin, for a passive buzzer or a
 *   piezo. It works on every board.
 * - `Bocina::MUESTRAS`: the library computes every sample of the sound, so
 *   it can play sine, triangle, saw, square and noise waves, and also
 *   recordings. On Raspberry Pi Pico (Pico SDK) and on SAMD21 boards (for
 *   example Arduino Zero or MKR Zero) it goes out through an I2S converter,
 *   for example MAX98357A. On Arduino Uno R4 it goes out through the analog
 *   converter on pin A0. On other boards this mode does not exist.
 *
 * Each sound can be played in two ways: sonar() blocks the program until
 * it ends, and tocar() starts it and returns right away. With tocar(),
 * call actualizar() on every pass of the loop.
 */
class Bocina
{
public:
    /**
     * \~spanish
     * @brief Como sale el sonido.
     *
     * \~english
     * @brief How the sound goes out.
     */
    enum Modo
    {
        /**
         * \~spanish Ondas cuadradas en una patita.
         * \~english Square waves on a pin.
         */
        TONOS,
        /**
         * \~spanish Muestras calculadas, por I2S o por el conversor analogo.
         * \~english Computed samples, through I2S or the analog converter.
         */
        MUESTRAS
    };

    /**
     * \~spanish
     * @brief Forma de onda en el modo MUESTRAS.
     *
     * \~english
     * @brief Waveform in MUESTRAS mode.
     */
    enum Forma
    {
        /**
         * \~spanish Cuadrada, suena como el modo TONOS.
         * \~english Square, sounds like TONOS mode.
         */
        CUADRADA,
        /**
         * \~spanish Seno, el sonido mas suave.
         * \~english Sine, the softest sound.
         */
        SENO,
        /**
         * \~spanish Triangulo, suave pero con mas brillo que el seno.
         * \~english Triangle, soft but brighter than the sine.
         */
        TRIANGULO,
        /**
         * \~spanish Sierra, el sonido mas brillante.
         * \~english Saw, the brightest sound.
         */
        SIERRA,
        /**
         * \~spanish Ruido blanco, ignora la frecuencia.
         * \~english White noise, ignores the frequency.
         */
        RUIDO
    };

    /**
     * \~spanish
     * @brief Muestras por segundo en el modo MUESTRAS.
     *
     * Sirve para calcular grabaciones: un segundo de sonido son
     * 22050 muestras.
     *
     * \~english
     * @brief Samples per second in MUESTRAS mode.
     *
     * Useful for computing recordings: one second of sound is
     * 22050 samples.
     */
    static const uint16_t FRECUENCIA_MUESTREO = 22050;

    /**
     * \~spanish
     * @brief Crea una bocina en modo TONOS y configura la patita como salida.
     * @param nuevaPatita patita donde esta conectado el buzzer.
     *
     * \~english
     * @brief Creates a speaker in TONOS mode and configures the pin as an output.
     * @param nuevaPatita pin the buzzer is connected to.
     */
    Bocina(uint8_t nuevaPatita);

    /**
     * \~spanish
     * @brief Cambia la patita del modo TONOS y la configura como salida.
     * @param nuevaPatita nueva patita.
     *
     * \~english
     * @brief Changes the TONOS mode pin and configures it as an output.
     * @param nuevaPatita new pin.
     */
    void setPatita(uint8_t nuevaPatita);

    /**
     * \~spanish
     * @brief Cambia el modo, y detiene lo que este sonando.
     *
     * Solo una bocina a la vez puede estar en modo MUESTRAS. Hay que
     * llamarla en setup(), no al crear la bocina.
     *
     * @param nuevoModo Bocina::TONOS o Bocina::MUESTRAS.
     * @return `true` si se pudo, `false` si la placa no tiene el modo
     * MUESTRAS. En ese caso la bocina sigue en modo TONOS.
     *
     * \~english
     * @brief Changes the mode, and stops whatever is playing.
     *
     * Only one speaker at a time can be in MUESTRAS mode. Call it in
     * setup(), not when creating the speaker.
     *
     * @param nuevoModo Bocina::TONOS or Bocina::MUESTRAS.
     * @return `true` if it worked, `false` if the board does not have
     * MUESTRAS mode. In that case the speaker stays in TONOS mode.
     */
    bool setModo(Modo nuevoModo);

    /**
     * \~spanish
     * @brief Modo actual.
     * @return Bocina::TONOS o Bocina::MUESTRAS.
     *
     * \~english
     * @brief Current mode.
     * @return Bocina::TONOS or Bocina::MUESTRAS.
     */
    Modo getModo();

    /**
     * \~spanish
     * @brief Cambia las patitas I2S, solo en Pico SDK.
     *
     * En Pico la patita de reloj de palabra (LRC) es siempre la siguiente
     * a la de reloj de bits (BCLK). Por defecto son datos en gpio 9, BCLK
     * en gpio 10 y LRC en gpio 11. Hay que llamarla antes de setModo().
     * En placas SAMD21 las patitas son fijas, y se ignora.
     *
     * @param datos patita de datos (DIN).
     * @param reloj patita de reloj de bits (BCLK). LRC va en reloj + 1.
     *
     * \~english
     * @brief Changes the I2S pins, only on Pico SDK.
     *
     * On Pico the word clock pin (LRC) is always the one right after the
     * bit clock pin (BCLK). By default data is on gpio 9, BCLK on gpio 10
     * and LRC on gpio 11. Call it before setModo(). On SAMD21 boards the
     * pins are fixed, and it is ignored.
     *
     * @param datos data pin (DIN).
     * @param reloj bit clock pin (BCLK). LRC goes on reloj + 1.
     */
    void setPatitasI2S(uint8_t datos, uint8_t reloj);

    /**
     * \~spanish
     * @brief Cambia la forma de onda del modo MUESTRAS.
     *
     * El cambio se escucha de inmediato, aunque haya algo sonando.
     * En modo TONOS la onda siempre es cuadrada. El valor inicial es
     * Bocina::CUADRADA.
     *
     * @param nuevaForma una de las formas de Bocina::Forma.
     *
     * \~english
     * @brief Changes the MUESTRAS mode waveform.
     *
     * The change is heard right away, even if something is playing.
     * In TONOS mode the wave is always square. The initial value is
     * Bocina::CUADRADA.
     *
     * @param nuevaForma one of the shapes in Bocina::Forma.
     */
    void setForma(Forma nuevaForma);

    /**
     * \~spanish
     * @brief Forma de onda actual.
     * @return una de las formas de Bocina::Forma.
     *
     * \~english
     * @brief Current waveform.
     * @return one of the shapes in Bocina::Forma.
     */
    Forma getForma();

    /**
     * \~spanish
     * @brief Cambia el volumen del modo MUESTRAS.
     *
     * Va de 0 (silencio) a 255 (el maximo). El valor inicial es 128.
     * En modo TONOS el volumen no cambia.
     *
     * @param nuevoVolumen volumen de 0 a 255.
     *
     * \~english
     * @brief Changes the MUESTRAS mode volume.
     *
     * It goes from 0 (silence) to 255 (the maximum). The initial value is
     * 128. In TONOS mode the volume does not change.
     *
     * @param nuevoVolumen volume from 0 to 255.
     */
    void setVolumen(uint8_t nuevoVolumen);

    /**
     * \~spanish
     * @brief Volumen actual.
     * @return volumen de 0 a 255.
     *
     * \~english
     * @brief Current volume.
     * @return volume from 0 to 255.
     */
    uint8_t getVolumen();

    /**
     * \~spanish
     * @brief Toca una frecuencia y espera a que termine.
     *
     * Bloquea el programa mientras dura. Con duracion 0 no hace nada.
     *
     * @param frecuencia frecuencia en Hz, por ejemplo 440. Con 0 es
     * un silencio.
     * @param duracion duracion en milisegundos.
     *
     * \~english
     * @brief Plays a frequency and waits for it to end.
     *
     * It blocks the program while it lasts. With duracion 0 it does nothing.
     *
     * @param frecuencia frequency in Hz, for example 440. With 0 it is
     * a rest.
     * @param duracion length in milliseconds.
     */
    void sonar(uint16_t frecuencia, uint32_t duracion);

    /**
     * \~spanish
     * @brief Empieza a tocar una frecuencia sin bloquear el programa.
     *
     * Hay que llamar a actualizar() en cada vuelta del loop para que
     * termine a tiempo. Corta lo que este sonando.
     *
     * @param frecuencia frecuencia en Hz, por ejemplo 440. Con 0 es
     * un silencio.
     * @param duracion duracion en milisegundos. Con 0, o sin indicarla,
     * suena hasta llamar a silenciar().
     *
     * \~english
     * @brief Starts playing a frequency without blocking the program.
     *
     * Call actualizar() on every pass of the loop so it ends on time.
     * It cuts off whatever is playing.
     *
     * @param frecuencia frequency in Hz, for example 440. With 0 it is
     * a rest.
     * @param duracion length in milliseconds. With 0, or if not given, it
     * plays until silenciar() is called.
     */
    void tocar(uint16_t frecuencia, uint32_t duracion = 0);

    /**
     * \~spanish
     * @brief Toca una grabacion, solo en modo MUESTRAS.
     *
     * Cada muestra va de -32768 a 32767, a FRECUENCIA_MUESTREO muestras
     * por segundo. No bloquea el programa. El arreglo no se copia, asi que
     * tiene que existir mientras suena.
     *
     * @param muestras arreglo con la grabacion.
     * @param cantidad cuantas muestras tiene.
     * @return `false` si la bocina esta en modo TONOS.
     *
     * \~english
     * @brief Plays a recording, only in MUESTRAS mode.
     *
     * Each sample goes from -32768 to 32767, at FRECUENCIA_MUESTREO samples
     * per second. It does not block the program. The array is not copied,
     * so it must exist while it plays.
     *
     * @param muestras array with the recording.
     * @param cantidad how many samples it has.
     * @return `false` if the speaker is in TONOS mode.
     */
    bool tocarMuestras(const int16_t *muestras, uint32_t cantidad);

    /**
     * \~spanish
     * @brief Termina lo que toca tocar() cuando se cumple su duracion.
     *
     * Hay que llamarla en cada vuelta del loop cuando se usa tocar().
     * Mientras mas seguido, mas precisa la duracion. En modo MUESTRAS la
     * duracion se cuenta sola, pero igual conviene llamarla.
     *
     * \~english
     * @brief Ends what tocar() plays when its length is up.
     *
     * Call it on every pass of the loop when using tocar(). The more
     * often, the more precise the length. In MUESTRAS mode the length is
     * counted on its own, but calling it is still a good idea.
     */
    void actualizar();

    /**
     * \~spanish
     * @brief Si hay algo sonando.
     * @return `true` mientras suena una frecuencia, un silencio o una grabacion.
     *
     * \~english
     * @brief Whether something is playing.
     * @return `true` while a frequency, a rest or a recording is playing.
     */
    bool sonando();

    /**
     * \~spanish
     * @brief Detiene lo que este sonando.
     *
     * \~english
     * @brief Stops whatever is playing.
     */
    void silenciar();

private:
    // se llama desde la interrupcion de audio, con contexto = esta bocina
    static void llenar(int16_t *muestras, uint16_t cantidad, void *contexto);
    int16_t siguienteMuestra();
    int16_t calcularOnda();
    void empezarNota(uint16_t frecuencia, uint32_t duracion);

    uint8_t patita;
    uint8_t patitaDatos;
    uint8_t patitaReloj;
    Modo modo;

    // tambien las lee la interrupcion de audio
    volatile Forma forma;
    volatile uint8_t volumen;

    // estado de la nota, lo comparten el programa y la interrupcion.
    // sonandoAhora se escribe al final, asi la interrupcion nunca
    // lee una nota a medio configurar
    volatile bool sonandoAhora;
    volatile bool sinFin;
    volatile uint16_t frecuencia;
    volatile uint32_t fase;
    volatile uint32_t incremento;
    volatile uint32_t muestrasRestantes;
    const int16_t *volatile grabacion;
    volatile uint32_t largoGrabacion;
    volatile uint32_t posicionGrabacion;
    uint32_t semillaRuido;

    // en modo TONOS la nota se termina mirando el reloj
    uint32_t inicioNota;
    uint32_t duracionNota;
};

#endif
