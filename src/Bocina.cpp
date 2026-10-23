#include "Bocina.h"

#include "Hardware.h"

// la fase es un contador de 32 bits que da una vuelta por ciclo de la onda.
// cada muestra avanza incremento = frecuencia * 2^32 / FRECUENCIA_MUESTREO
static const uint32_t INCREMENTO_POR_HZ = (uint32_t)(4294967296.0 / Bocina::FRECUENCIA_MUESTREO);

// sobre la mitad de la frecuencia de muestreo la onda ya no se puede
// representar, y ademas la cuenta del incremento se desbordaria
static const uint16_t FRECUENCIA_MAXIMA = Bocina::FRECUENCIA_MUESTREO / 2;

// patitas i2s por defecto en pico
static const uint8_t PATITA_DATOS_I2S = 9;
static const uint8_t PATITA_RELOJ_I2S = 10;

Bocina::Bocina(uint8_t nuevaPatita)
{
    patita = nuevaPatita;
    patitaDatos = PATITA_DATOS_I2S;
    patitaReloj = PATITA_RELOJ_I2S;
    modo = TONOS;
    forma = CUADRADA;
    volumen = 128;

    sonandoAhora = false;
    sinFin = false;
    frecuencia = 0;
    fase = 0;
    incremento = 0;
    muestrasRestantes = 0;
    grabacion = 0;
    largoGrabacion = 0;
    posicionGrabacion = 0;

    // cualquier valor distinto de 0 sirve
    semillaRuido = 2463534242UL;

    inicioNota = 0;
    duracionNota = 0;

    BocinaHardware::configurarSalida(patita);
}

void Bocina::setPatita(uint8_t nuevaPatita)
{
    // no dejar sonando la patita anterior
    if (modo == TONOS)
    {
        silenciar();
    }

    patita = nuevaPatita;

    BocinaHardware::configurarSalida(patita);
}

bool Bocina::setModo(Modo nuevoModo)
{
    silenciar();

    if (nuevoModo == MUESTRAS)
    {
        if (!BocinaHardware::iniciarMuestras(FRECUENCIA_MUESTREO, patitaDatos, patitaReloj, llenar, this))
        {
            return false;
        }
    }
    else if (modo == MUESTRAS)
    {
        BocinaHardware::detenerMuestras();
    }

    modo = nuevoModo;
    return true;
}

Bocina::Modo Bocina::getModo()
{
    return modo;
}

void Bocina::setPatitasI2S(uint8_t datos, uint8_t reloj)
{
    patitaDatos = datos;
    patitaReloj = reloj;
}

void Bocina::setForma(Forma nuevaForma)
{
    forma = nuevaForma;
}

Bocina::Forma Bocina::getForma()
{
    return forma;
}

void Bocina::setVolumen(uint8_t nuevoVolumen)
{
    volumen = nuevoVolumen;
}

uint8_t Bocina::getVolumen()
{
    return volumen;
}

void Bocina::sonar(uint16_t nuevaFrecuencia, uint32_t duracion)
{
    // sin duracion esperaria para siempre
    if (duracion == 0)
    {
        return;
    }

    tocar(nuevaFrecuencia, duracion);
    while (sonando())
    {
        actualizar();
    }
}

void Bocina::tocar(uint16_t nuevaFrecuencia, uint32_t duracion)
{
    empezarNota(nuevaFrecuencia, duracion);
}

bool Bocina::tocarMuestras(const int16_t *muestras, uint32_t cantidad)
{
    if (modo != MUESTRAS)
    {
        return false;
    }

    sonandoAhora = false;
    grabacion = muestras;
    largoGrabacion = cantidad;
    posicionGrabacion = 0;
    sonandoAhora = cantidad > 0;

    return true;
}

void Bocina::actualizar()
{
    // en modo MUESTRAS la interrupcion cuenta las muestras y termina
    // la nota sola. en modo TONOS hay que mirar el reloj
    if (modo == TONOS && sonandoAhora && !sinFin)
    {
        // restar en vez de comparar con un tiempo final,
        // asi funciona aunque milisegundos() de la vuelta
        if (BocinaHardware::milisegundos() - inicioNota >= duracionNota)
        {
            BocinaHardware::silenciarTono(patita);
            sonandoAhora = false;
        }
    }
}

bool Bocina::sonando()
{
    return sonandoAhora;
}

void Bocina::silenciar()
{
    sonandoAhora = false;

    if (modo == TONOS)
    {
        BocinaHardware::silenciarTono(patita);
    }
}

void Bocina::empezarNota(uint16_t nuevaFrecuencia, uint32_t duracion)
{
    if (modo == TONOS)
    {
        if (nuevaFrecuencia == 0)
        {
            BocinaHardware::silenciarTono(patita);
        }
        else
        {
            BocinaHardware::sonarTono(patita, nuevaFrecuencia);
        }

        inicioNota = BocinaHardware::milisegundos();
        duracionNota = duracion;
        sinFin = duracion == 0;
        sonandoAhora = true;
        return;
    }

    if (nuevaFrecuencia > FRECUENCIA_MAXIMA)
    {
        nuevaFrecuencia = FRECUENCIA_MAXIMA;
    }

    // apagar mientras se configura, asi la interrupcion
    // no toca una nota a medias
    sonandoAhora = false;
    grabacion = 0;
    frecuencia = nuevaFrecuencia;
    incremento = nuevaFrecuencia * INCREMENTO_POR_HZ;

    // separar segundos y milisegundos, asi la cuenta cabe en 32 bits
    muestrasRestantes = (duracion / 1000) * FRECUENCIA_MUESTREO +
                        (duracion % 1000) * FRECUENCIA_MUESTREO / 1000;
    sinFin = duracion == 0;
    sonandoAhora = true;
}

void Bocina::llenar(int16_t *muestras, uint16_t cantidad, void *contexto)
{
    Bocina *bocina = (Bocina *)contexto;

    for (uint16_t i = 0; i < cantidad; i++)
    {
        muestras[i] = bocina->siguienteMuestra();
    }
}

int16_t Bocina::siguienteMuestra()
{
    if (!sonandoAhora)
    {
        return 0;
    }

    int32_t muestra;

    if (grabacion != 0)
    {
        muestra = grabacion[posicionGrabacion];
        posicionGrabacion = posicionGrabacion + 1;
        if (posicionGrabacion >= largoGrabacion)
        {
            grabacion = 0;
            sonandoAhora = false;
        }
    }
    else
    {
        if (!sinFin)
        {
            if (muestrasRestantes == 0)
            {
                sonandoAhora = false;
                return 0;
            }
            muestrasRestantes = muestrasRestantes - 1;
        }

        // una frecuencia 0 es un silencio que igual dura
        if (frecuencia == 0)
        {
            return 0;
        }

        muestra = calcularOnda();
        fase += incremento;
    }

    return (muestra * volumen) >> 8;
}

int16_t Bocina::calcularOnda()
{
    switch (forma)
    {
    case SENO:
    {
        // x va de -32768 a 32767, que representa de -pi a pi
        int32_t x = (int16_t)(fase >> 16);
        int32_t absX = x < 0 ? -x : x;

        // aproximar el seno con una parabola, 4t(1 - |t|),
        // y despues corregirla para que se parezca mas
        int32_t y = (x * (32768 - absX)) >> 13;
        int32_t absY = y < 0 ? -y : y;
        y += (((y * absY) >> 15) - y) * 7373 >> 15;

        if (y > 32767)
        {
            y = 32767;
        }
        if (y < -32767)
        {
            y = -32767;
        }
        return y;
    }

    case TRIANGULO:
    {
        // subir en la primera mitad del ciclo y bajar en la segunda
        int32_t u = fase >> 16;
        if (u < 32768)
        {
            return 2 * u - 32767;
        }
        return 32767 - 2 * (u - 32768);
    }

    case SIERRA:
        return (int32_t)(fase >> 16) - 32768;

    case RUIDO:
        // xorshift: un generador de numeros pseudoaleatorios muy rapido
        semillaRuido ^= semillaRuido << 13;
        semillaRuido ^= semillaRuido >> 17;
        semillaRuido ^= semillaRuido << 5;
        return (int16_t)(semillaRuido >> 16);

    case CUADRADA:
    default:
        return fase < 0x80000000UL ? 32767 : -32767;
    }
}
