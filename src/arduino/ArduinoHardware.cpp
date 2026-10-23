#include "./../Hardware.h"

#include <Arduino.h>

// elegir la salida de muestras segun la placa.
// samd21 con i2s, por ejemplo arduino zero o mkr zero
#if defined(ARDUINO_ARCH_SAMD) && defined(I2S_INTERFACES_COUNT) && (I2S_INTERFACES_COUNT > 0)
#define BOCINA_I2S_SAMD
#include <I2S.h>
// uno r4, con el conversor analogo de la patita A0
#elif defined(ARDUINO_UNOR4_MINIMA) || defined(ARDUINO_UNOR4_WIFI)
#define BOCINA_DAC_R4
#include "FspTimer.h"
#endif

namespace BocinaHardware
{

    void configurarSalida(uint8_t patita)
    {
        pinMode(patita, OUTPUT);
    }

    void sonarTono(uint8_t patita, uint16_t frecuencia)
    {
        tone(patita, frecuencia);
    }

    void silenciarTono(uint8_t patita)
    {
        noTone(patita);
    }

    uint32_t milisegundos()
    {
        return millis();
    }

#if defined(BOCINA_I2S_SAMD) || defined(BOCINA_DAC_R4)
    static volatile FuncionLlenar llenarActual = 0;
    static void *contextoActual = 0;
#endif

#if defined(BOCINA_I2S_SAMD)

    static const uint16_t MUESTRAS_POR_BLOQUE = 64;

    // la biblioteca I2S llama a esta funcion cuando termina de mandar
    // un bloque. escribir bloques mientras haya espacio
    static void alTransmitir()
    {
        int16_t mono[MUESTRAS_POR_BLOQUE];
        int16_t estereo[MUESTRAS_POR_BLOQUE * 2];

        while (I2S.availableForWrite() >= (int)sizeof(estereo))
        {
            // copiar el puntero, detenerMuestras() lo puede borrar entremedio
            FuncionLlenar llenar = llenarActual;
            if (llenar != 0)
            {
                llenar(mono, MUESTRAS_POR_BLOQUE, contextoActual);
            }
            else
            {
                memset(mono, 0, sizeof(mono));
            }

            // el mismo sonido en el canal izquierdo y el derecho
            for (uint16_t i = 0; i < MUESTRAS_POR_BLOQUE; i++)
            {
                estereo[2 * i] = mono[i];
                estereo[2 * i + 1] = mono[i];
            }

            I2S.write(estereo, sizeof(estereo));
        }
    }

    bool iniciarMuestras(uint32_t frecuenciaMuestreo, uint8_t patitaDatos, uint8_t patitaReloj, FuncionLlenar llenar, void *contexto)
    {
        // en samd las patitas de i2s son fijas
        (void)patitaDatos;
        (void)patitaReloj;

        llenarActual = llenar;
        contextoActual = contexto;

        static bool iniciado = false;
        if (iniciado)
        {
            return true;
        }

        if (!I2S.begin(I2S_PHILIPS_MODE, frecuenciaMuestreo, 16))
        {
            return false;
        }
        I2S.onTransmit(alTransmitir);

        // el primer bloque pone en marcha la transmision
        alTransmitir();

        iniciado = true;
        return true;
    }

    void detenerMuestras()
    {
        // no cerrar el i2s, porque volver a abrirlo no siempre funciona.
        // seguir mandando silencio, sin llamar a llenar()
        llenarActual = 0;
    }

#elif defined(BOCINA_DAC_R4)

    static const uint16_t MUESTRAS_POR_BLOQUE = 32;

    static FspTimer temporizador;
    static int16_t bloque[MUESTRAS_POR_BLOQUE];
    static uint16_t posicion = MUESTRAS_POR_BLOQUE;

    // el temporizador llama a esta funcion una vez por muestra
    static void alTick(timer_callback_args_t *argumentos)
    {
        (void)argumentos;

        if (posicion >= MUESTRAS_POR_BLOQUE)
        {
            llenarActual(bloque, MUESTRAS_POR_BLOQUE, contextoActual);
            posicion = 0;
        }

        // pasar de -32768..32767 a la resolucion de analogWrite,
        // 8 bits si no se ha cambiado con analogWriteResolution()
        uint16_t valor = (uint16_t)(bloque[posicion] + 32768);
        analogWrite(DAC, valor >> (16 - analogWriteResolution()));
        posicion++;
    }

    bool iniciarMuestras(uint32_t frecuenciaMuestreo, uint8_t patitaDatos, uint8_t patitaReloj, FuncionLlenar llenar, void *contexto)
    {
        // el conversor analogo esta siempre en A0
        (void)patitaDatos;
        (void)patitaReloj;

        llenarActual = llenar;
        contextoActual = contexto;

        static bool iniciado = false;
        if (!iniciado)
        {
            uint8_t tipo;
            int8_t canal = FspTimer::get_available_timer(tipo);
            if (canal < 0)
            {
                return false;
            }

            if (!temporizador.begin(TIMER_MODE_PERIODIC, tipo, canal, (float)frecuenciaMuestreo, 0.0f, alTick))
            {
                return false;
            }
            temporizador.setup_overflow_irq();
            temporizador.open();

            iniciado = true;
        }

        temporizador.start();
        return true;
    }

    void detenerMuestras()
    {
        temporizador.stop();

        // dejar la salida en la mitad, que es el silencio
        analogWrite(DAC, 1 << (analogWriteResolution() - 1));
    }

#else

    // esta placa no tiene salida de muestras, solo tonos
    bool iniciarMuestras(uint32_t frecuenciaMuestreo, uint8_t patitaDatos, uint8_t patitaReloj, FuncionLlenar llenar, void *contexto)
    {
        (void)frecuenciaMuestreo;
        (void)patitaDatos;
        (void)patitaReloj;
        (void)llenar;
        (void)contexto;

        return false;
    }

    void detenerMuestras()
    {
    }

#endif
}
