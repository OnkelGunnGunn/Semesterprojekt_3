
#include "pico/stdlib.h"

const uint SPEAKER_PIN = 15;

// Spil en tone med en bestemt frekvens og varighed
void playNote(uint frequency, uint durationMs)
{
    if (frequency == 0) {
        sleep_ms(durationMs);
        return;
    }

    uint32_t halfPeriodUs = 500000 / frequency;
    absolute_time_t endTime = make_timeout_time_ms(durationMs);

    while (absolute_time_diff_us(get_absolute_time(), endTime) > 0) {
        gpio_put(SPEAKER_PIN, 1);
        busy_wait_us(halfPeriodUs);

        gpio_put(SPEAKER_PIN, 0);
        busy_wait_us(halfPeriodUs);
    }
}

// Afspil en tone med en lille pause bagefter
void note(uint frequency, uint durationMs)
{
    playNote(frequency, durationMs);
    sleep_ms(35);
}

int main()
{
    gpio_init(SPEAKER_PIN);
    gpio_set_dir(SPEAKER_PIN, GPIO_OUT);
    gpio_put(SPEAKER_PIN, 0);

    sleep_ms(1000);

    // Jingle Bells - første frase
    while (true) {
        note(659, 250);  // E
        note(659, 250);  // E
        note(659, 500);  // E

        note(659, 250);  // E
        note(659, 250);  // E
        note(659, 500);  // E

        note(659, 250);  // E
        note(784, 250);  // G
        note(523, 350);  // C
        note(587, 150);  // D
        note(659, 750);  // E

        sleep_ms(250);

        note(698, 250);  // F
        note(698, 250);  // F
        note(698, 350);  // F
        note(698, 150);  // F

        note(698, 250);  // F
        note(659, 250);  // E
        note(659, 250);  // E
        note(659, 150);  // E
        note(659, 150);  // E

        note(659, 250);  // E
        note(587, 250);  // D
        note(587, 250);  // D
        note(659, 250);  // E
        note(587, 500);  // D
        note(784, 500);  // G

        sleep_ms(1000);
    }
}
