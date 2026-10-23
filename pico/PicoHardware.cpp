#include "../src/Hardware.h"

#include <string.h>

#include "bocina_i2s.pio.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

namespace BocinaHardware
{

    void configurarSalida(uint8_t patita)
    {
        // los tonos salen por pwm, con la patita en 0 mientras no suena
        gpio_set_function(patita, GPIO_FUNC_PWM);
        pwm_set_gpio_level(patita, 0);
        pwm_set_enabled(pwm_gpio_to_slice_num(patita), true);
    }

    void sonarTono(uint8_t patita, uint16_t frecuencia)
    {
        uint slice = pwm_gpio_to_slice_num(patita);
        uint32_t reloj = clock_get_hz(clk_sys);

        // el contador del pwm llega como maximo a 65535. dividir el reloj
        // lo justo para que el periodo quepa. el divisor tiene 4 bits de
        // fraccion, asi que se cuenta en dieciseisavos, redondeando hacia
        // arriba
        uint32_t divisor16 = (reloj + frecuencia * 4096u - 1) / (frecuencia * 4096u);
        if (divisor16 < 16)
        {
            divisor16 = 16;
        }
        if (divisor16 > 4095)
        {
            divisor16 = 4095;
        }

        uint64_t tope = (uint64_t)reloj * 16 / ((uint64_t)divisor16 * frecuencia) - 1;
        if (tope > 65535)
        {
            tope = 65535;
        }

        // un dieciseisavo se representa exacto en float
        pwm_set_clkdiv(slice, divisor16 / 16.0f);
        pwm_set_wrap(slice, tope);

        // la mitad del periodo arriba, la mitad abajo
        pwm_set_gpio_level(patita, (tope + 1) / 2);
    }

    void silenciarTono(uint8_t patita)
    {
        pwm_set_gpio_level(patita, 0);
    }

    uint32_t milisegundos()
    {
        return to_ms_since_boot(get_absolute_time());
    }

    // las muestras salen por i2s con la maquina de estados de bocina_i2s.pio.
    // el dma le pasa un bloque mientras se llena el otro

    static const uint16_t MUESTRAS_POR_BLOQUE = 128;

    static PIO pio;
    static uint maquina;
    static uint inicioPrograma;
    static int canalDma = -1;
    static bool corriendo = false;

    static uint32_t bloques[2][MUESTRAS_POR_BLOQUE];
    static int16_t mono[MUESTRAS_POR_BLOQUE];
    static uint8_t bloqueSonando = 0;

    static volatile FuncionLlenar llenarActual = 0;
    static void *volatile contextoActual = 0;

    static void llenarBloque(uint8_t indice)
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
            uint16_t muestra = (uint16_t)mono[i];
            bloques[indice][i] = ((uint32_t)muestra << 16) | muestra;
        }
    }

    static void alTerminarDma()
    {
        // la interrupcion es compartida, puede ser de otro canal
        if (!dma_channel_get_irq0_status(canalDma))
        {
            return;
        }
        dma_channel_acknowledge_irq0(canalDma);

        // mandar de inmediato el otro bloque, que ya esta lleno,
        // y despues llenar el que acaba de terminar
        uint8_t terminado = bloqueSonando;
        bloqueSonando = 1 - bloqueSonando;
        dma_channel_set_read_addr(canalDma, bloques[bloqueSonando], true);

        llenarBloque(terminado);
    }

    bool iniciarMuestras(uint32_t frecuenciaMuestreo, uint8_t patitaDatos, uint8_t patitaReloj, FuncionLlenar llenar, void *contexto)
    {
        llenarActual = llenar;
        contextoActual = contexto;

        if (corriendo)
        {
            return true;
        }

        // reservar la maquina de estados y el canal dma una sola vez
        if (canalDma < 0)
        {
            if (!pio_claim_free_sm_and_add_program(&bocina_i2s_program, &pio, &maquina, &inicioPrograma))
            {
                return false;
            }

            canalDma = dma_claim_unused_channel(false);
            if (canalDma < 0)
            {
                pio_remove_program_and_unclaim_sm(&bocina_i2s_program, pio, maquina, inicioPrograma);
                return false;
            }

            irq_add_shared_handler(DMA_IRQ_0, alTerminarDma, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
            irq_set_enabled(DMA_IRQ_0, true);
        }

        // maquina de estados: datos por out, BCLK y LRC por side-set
        pio_gpio_init(pio, patitaDatos);
        pio_gpio_init(pio, patitaReloj);
        pio_gpio_init(pio, patitaReloj + 1);

        pio_sm_config configuracion = bocina_i2s_program_get_default_config(inicioPrograma);
        sm_config_set_out_pins(&configuracion, patitaDatos, 1);
        sm_config_set_sideset_pins(&configuracion, patitaReloj);

        // sacar primero el bit mas significativo, y pedir
        // una palabra nueva al fifo cada 32 bits
        sm_config_set_out_shift(&configuracion, false, true, 32);
        sm_config_set_fifo_join(&configuracion, PIO_FIFO_JOIN_TX);

        // 64 ciclos por cuadro, ver bocina_i2s.pio
        sm_config_set_clkdiv(&configuracion, (float)clock_get_hz(clk_sys) / (frecuenciaMuestreo * 64.0f));

        pio_sm_init(pio, maquina, inicioPrograma, &configuracion);

        uint32_t mascara = (1u << patitaDatos) | (3u << patitaReloj);
        pio_sm_set_pindirs_with_mask(pio, maquina, mascara, mascara);
        pio_sm_set_pins(pio, maquina, 0);

        // dma: de los bloques al fifo de la maquina, al ritmo que ella pida
        dma_channel_config configuracionDma = dma_channel_get_default_config(canalDma);
        channel_config_set_transfer_data_size(&configuracionDma, DMA_SIZE_32);
        channel_config_set_read_increment(&configuracionDma, true);
        channel_config_set_write_increment(&configuracionDma, false);
        channel_config_set_dreq(&configuracionDma, pio_get_dreq(pio, maquina, true));
        dma_channel_configure(canalDma, &configuracionDma, &pio->txf[maquina], bloques[0], MUESTRAS_POR_BLOQUE, false);
        dma_channel_set_irq0_enabled(canalDma, true);

        // llenar los dos bloques antes de partir
        llenarBloque(0);
        llenarBloque(1);
        bloqueSonando = 0;

        pio_sm_set_enabled(pio, maquina, true);
        dma_channel_start(canalDma);

        corriendo = true;
        return true;
    }

    void detenerMuestras()
    {
        if (!corriendo)
        {
            return;
        }

        // apagar la interrupcion antes de abortar,
        // si no el aborto la puede disparar
        dma_channel_set_irq0_enabled(canalDma, false);
        dma_channel_abort(canalDma);
        dma_channel_acknowledge_irq0(canalDma);

        pio_sm_set_enabled(pio, maquina, false);
        pio_sm_clear_fifos(pio, maquina);

        llenarActual = 0;
        corriendo = false;
    }

}
