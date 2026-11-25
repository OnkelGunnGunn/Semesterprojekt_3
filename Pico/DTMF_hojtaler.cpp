
#include "pico/stdlib.h"

const uint SPEAKER_PIN = 15;

// Genererer to samtidige firkantsignaler ved at blande dem
// digitalt på én GPIO-pin.
void playDTMF(uint lowFreq, uint highFreq, uint durationMs)
{
    const uint32_t sampleRate = 40000;
    const uint32_t totalSamples = sampleRate * durationMs / 1000;

    uint32_t phaseLow = 0;
    uint32_t phaseHigh = 0;

    uint32_t stepLow =
        (uint64_t(lowFreq) << 32) / sampleRate;
    uint32_t stepHigh =
        (uint64_t(highFreq) << 32) / sampleRate;

    absolute_time_t nextSample = get_absolute_time();

    for (uint32_t i = 0; i < totalSamples; i++) {
        phaseLow += stepLow;
        phaseHigh += stepHigh;

        bool lowState = (phaseLow & 0x80000000u) != 0;
        bool highState = (phaseHigh & 0x80000000u) != 0;

        // Blanding af de to signaler
        gpio_put(SPEAKER_PIN, lowState || highState);

        nextSample = delayed_by_us(nextSample, 25);
        busy_wait_until(nextSample);
    }

    gpio_put(SPEAKER_PIN, 0);
}

int main()
{
    gpio_init(SPEAKER_PIN);
    gpio_set_dir(SPEAKER_PIN, GPIO_OUT);

    while (true) {
        // DTMF 6: 770 Hz + 1477 Hz
        playDTMF(770, 1477, 500);
        sleep_ms(500);

        // DTMF 5: 852 Hz + 1209 Hz
        playDTMF(852, 1209, 500);
        sleep_ms(500);
        
        // DTMF D: 941 Hz + 1633 Hz
        playDTMF(941, 1633, 500);
        sleep_ms(500);
    }
}
