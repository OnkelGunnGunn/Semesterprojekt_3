
#include "FrequencyMeter.h"

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/timer.h"

#define SPI_PORT spi0

FrequencyMeter::FrequencyMeter()
{
}

// Initialiser SPI og MCP3002
void FrequencyMeter::init()
{
    spi_init(SPI_PORT, 1000000);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

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

// Læs kanal 0 på MCP3002
uint16_t FrequencyMeter::readMCP3002()
{
    uint8_t tx[2] = {0x68, 0x00};
    uint8_t rx[2] = {0, 0};

    gpio_put(PIN_CS, 0);

    spi_write_read_blocking(SPI_PORT, tx, rx, 2);

    gpio_put(PIN_CS, 1);

    return ((rx[0] & 0x03) << 8) | rx[1];
}

// Mål frekvens med zero-crossing
float FrequencyMeter::measureFrequency()
{
    const int SAMPLE_PERIOD_US = 1000000 / SAMPLE_RATE;

    // Indsaml samples med fast tidsinterval
    absolute_time_t start = get_absolute_time();

    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        absolute_time_t target =
            delayed_by_us(start, i * SAMPLE_PERIOD_US);

        busy_wait_until(target);

        samples[i] = readMCP3002();
    }

    // Beregn gennemsnit (DC-offset)
    uint32_t sum = 0;

    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        sum += samples[i];
    }

    float average = sum / static_cast<float>(SAMPLE_COUNT);

    // Find nulpassager i stigende retning
    int crossings = 0;

    int firstCrossing = -1;
    int lastCrossing = -1;

    for (int i = 1; i < SAMPLE_COUNT; i++)
    {
        if (samples[i - 1] < average &&
            samples[i] >= average)
        {
            if (firstCrossing == -1)
            {
                firstCrossing = i;
            }

            lastCrossing = i;
            crossings++;
        }
    }

    // Der skal være mindst to nulpassager
    if (crossings < 2)
    {
        return 0.0f;
    }

    // Beregn gennemsnitlig periode i antal samples
    float periods = crossings - 1;

    float sampleDistance =
        (lastCrossing - firstCrossing) / periods;

    // Beregn frekvens
    float frequency = SAMPLE_RATE / sampleDistance;

    return frequency;
}
