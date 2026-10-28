
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

const uint ADC_PIN = 26;        // GP26 = ADC0
const uint SAMPLE_COUNT = 1000;
const uint SAMPLE_DELAY_US = 100;

int main() {
    stdio_init_all();
    sleep_ms(2000);

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);

    printf("W104 frekvenstest startet!\n");
    printf("Afspil en konstant tone foran mikrofonen.\n\n");

    while (true) {
        uint16_t samples[SAMPLE_COUNT];
        uint32_t sum = 0;

        // Indsaml samples
        for (uint i = 0; i < SAMPLE_COUNT; i++) {
            samples[i] = adc_read();
            sum += samples[i];
            sleep_us(SAMPLE_DELAY_US);
        }

        // Find gennemsnittet, som bruges som signalets midterpunkt
        float average = (float)sum / SAMPLE_COUNT;

        // Tæl stigende nulgennemgange
        uint crossings = 0;

        for (uint i = 1; i < SAMPLE_COUNT; i++) {
            if (samples[i - 1] < average &&
                samples[i] >= average) {
                crossings++;
            }
        }

        // Beregn frekvensen
        float duration =
            (SAMPLE_COUNT - 1) * SAMPLE_DELAY_US / 1000000.0f;

        float frequency = crossings / duration;

        printf("Frekvens: %.1f Hz\n", frequency);

        sleep_ms(200);
    }
}
