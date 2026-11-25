#include "../../Pico/DTMFReceiver.h"
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/timer.h"

// -------------------------
// SPI / MCP3002
// -------------------------

#define SPI_PORT spi0

#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// -------------------------
// DTMF frekvenser
// -------------------------

namespace
{
    const double frequencies[8] = {
        697, 770, 852, 941,
        1209, 1336, 1477, 1633
    };

    const char keys[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };
}

// -------------------------
// Initialisering
// -------------------------

void DTMFReceiver::begin()
{
    // Start SPI
    spi_init(SPI_PORT, 1000000);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // CS styres manuelt
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    spi_set_format(
        SPI_PORT,
        8,
        SPI_CPOL_0,
        SPI_CPHA_0,
        SPI_MSB_FIRST
    );
}

// -------------------------
// Læs MCP3002 kanal 0
// -------------------------

uint16_t DTMFReceiver::readMCP3002()
{
    uint8_t tx[2] = {0x68, 0x00};
    uint8_t rx[2] = {0, 0};

    gpio_put(PIN_CS, 0);

    spi_write_read_blocking(SPI_PORT, tx, rx, 2);

    gpio_put(PIN_CS, 1);

    return ((rx[0] & 0x03) << 8) | rx[1];
}

// -------------------------
// Goertzel algoritme
// -------------------------

double DTMFReceiver::goertzel(
    const double* signal,
    double frequency
)
{
    const double pi = 3.141592653589793;

    double omega = 2.0 * pi * frequency / SAMPLE_RATE;
    double coeff = 2.0 * cos(omega);

    double q0 = 0.0;
    double q1 = 0.0;
    double q2 = 0.0;

    for (int i = 0; i < N; i++)
    {
        q0 = coeff * q1 - q2 + signal[i];

        q2 = q1;
        q1 = q0;
    }

    return q1 * q1 + q2 * q2 - coeff * q1 * q2;
}

// -------------------------
// Find stærkeste frekvens
// -------------------------

int DTMFReceiver::strongest(double* powers, int start)
{
    int best = start;

    for (int i = start + 1; i < start + 4; i++)
    {
        if (powers[i] > powers[best])
        {
            best = i;
        }
    }

    return best;
}

// -------------------------
// DTMF detektion
// -------------------------

char DTMFReceiver::detectDTMF()
{
    double signal[N];
    double powers[8];

    // Beregn DC-offset
    double mean = 0.0;

    for (int i = 0; i < N; i++)
    {
        mean += samples[i];
    }

    mean /= N;

    // Fjern DC-offset og beregn RMS
    double energy = 0.0;

    for (int i = 0; i < N; i++)
    {
        signal[i] = samples[i] - mean;
        energy += signal[i] * signal[i];
    }

    double rms = sqrt(energy / N);

    // Afvis stilhed og meget svage signaler
    if (rms < 3.0)
    {
        return '\0';
    }

    // Hann-vindue reducerer spektral lækage
    const double pi = 3.141592653589793;

    for (int i = 0; i < N; i++)
    {
        double window =
            0.5 - 0.5 * cos(2.0 * pi * i / (N - 1));

        signal[i] *= window;
    }

    // Beregn styrken ved alle 8 frekvenser
    for (int i = 0; i < 8; i++)
    {
        powers[i] = goertzel(signal, frequencies[i]);
    }

    // Find stærkeste frekvens i hver gruppe
    int row = strongest(powers, 0);
    int col = strongest(powers, 4);

    // Find næststærkeste frekvens i hver gruppe
    double secondRow = 0.0;
    double secondCol = 0.0;

    for (int i = 0; i < 4; i++)
    {
        if (i != row && powers[i] > secondRow)
        {
            secondRow = powers[i];
        }

        if (i + 4 != col && powers[i + 4] > secondCol)
        {
            secondCol = powers[i + 4];
        }
    }

    // Kræv tydelige toppe i begge frekvensgrupper
    if (powers[row] < secondRow * 2.0)
    {
        return '\0';
    }

    if (powers[col] < secondCol * 2.0)
    {
        return '\0';
    }

    // Kræv at begge frekvenser er stærke nok
    double threshold = N * N * rms * rms * 0.015;

    if (powers[row] < threshold ||
        powers[col] < threshold)
    {
        return '\0';
    }

    return keys[row][col - 4];
}

// -------------------------
// Saml og analyser lyd
// -------------------------

char DTMFReceiver::readKey()
{
    // Saml 205 samples med 8 kHz
    absolute_time_t start = get_absolute_time();

    for (int i = 0; i < N; i++)
    {
        absolute_time_t target =
            delayed_by_us(start, i * SAMPLE_PERIOD_US);

        busy_wait_until(target);

        samples[i] = readMCP3002();
    }

    // Analyser lydblokken
    char key = detectDTMF();

    if (key == '\0')
    {
        candidateKey = '\0';
        candidateCount = 0;
        previousKey = '\0';

        return '\0';
    }

    if (key == candidateKey)
    {
        candidateCount++;
    }
    else
    {
        candidateKey = key;
        candidateCount = 1;
    }

    // Kræv to sammenhængende målinger
    if (candidateCount >= 2 && key != previousKey)
    {
        previousKey = key;
        return key;
    }

    return '\0';
}
