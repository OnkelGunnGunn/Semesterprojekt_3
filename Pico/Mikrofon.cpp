
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

const uint ADC_PIN = 26;       // GP26 = ADC0
const uint SAMPLE_COUNT = 1000;
const uint SAMPLE_DELAY_US = 100;

int main() {
    stdio_init_all();

    // Vent på, at USB Serial bliver klar
    sleep_ms(2000);

    // Initialiser ADC
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);       // ADC0 = GP26

    printf("W104 mikrofontest startet!\n");
    printf("Tal, klap eller afspil en tone.\n\n");

    while (true) {
        uint16_t min_value = 4095;
        uint16_t max_value = 0;
        uint32_t sum = 0;

        // Indsaml lydsamples
        for (uint i = 0; i < SAMPLE_COUNT; i++) {
            uint16_t value = adc_read();

            if (value < min_value) {
                min_value = value;
            }

            if (value > max_value) {
                max_value = value;
            }

            sum += value;
            sleep_us(SAMPLE_DELAY_US);
        }

        uint16_t average = sum / SAMPLE_COUNT;
        uint16_t peak_to_peak = max_value - min_value;

        printf("Gennemsnit: %u | Min: %u | Max: %u | Variation: %u\n",
               average, min_value, max_value, peak_to_peak);

        sleep_ms(100);
    }
}
