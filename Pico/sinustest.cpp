
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <math.h>
#include <stdio.h>

const uint SPEAKER_PIN = 15;

// PWM-indstillinger
const uint32_t PWM_WRAP = 499;
const int PWM_MID = 250;

// Lydindstillinger
const uint32_t SAMPLE_RATE = 32000;
const float AMPLITUDE = 50.0f;

const float PI = 3.14159265f;

// DTMF-frekvenser
const uint16_t lowFreq[4] = {697, 770, 852, 941};
const uint16_t highFreq[4] = {1209, 1336, 1477, 1633};

const char keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

void playDTMF(uint frequency1, uint frequency2, uint durationMs)
{
    float phase1 = 0.0f;
    float phase2 = 0.0f;

    float increment1 = 2.0f * PI * frequency1 / SAMPLE_RATE;
    float increment2 = 2.0f * PI * frequency2 / SAMPLE_RATE;

    uint32_t samples = SAMPLE_RATE * durationMs / 1000;

    absolute_time_t nextSample = get_absolute_time();

    for (uint32_t i = 0; i < samples; i++)
    {
        // Læg de to sinuskurver sammen
        float signal = (sinf(phase1) + sinf(phase2)) / 2.0f;

        // Beregn PWM-værdi
        int pwmValue = PWM_MID + (int)(signal * AMPLITUDE);

        pwm_set_gpio_level(SPEAKER_PIN, pwmValue);

        // Opdater faser
        phase1 += increment1;
        phase2 += increment2;

        if (phase1 >= 2.0f * PI)
            phase1 -= 2.0f * PI;

        if (phase2 >= 2.0f * PI)
            phase2 -= 2.0f * PI;

        // 32 kHz sample rate
        nextSample = delayed_by_us(nextSample, 31);

        if (i % 4 == 3)
            nextSample = delayed_by_us(nextSample, 1);

        busy_wait_until(nextSample);
    }

    pwm_set_gpio_level(SPEAKER_PIN, PWM_MID);
}

void playKey(uint row, uint column)
{
    printf("Tast: %c | %u Hz + %u Hz\n",
           keys[row][column],
           lowFreq[row],
           highFreq[column]);

    playDTMF(lowFreq[row], highFreq[column], 500);

    sleep_ms(500);
}

int main()
{
    stdio_init_all();
    sleep_ms(1500);

    gpio_set_function(SPEAKER_PIN, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(SPEAKER_PIN);

    pwm_config config = pwm_get_default_config();

    // 125 MHz / 500 = 250 kHz PWM
    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_config_set_wrap(&config, PWM_WRAP);

    pwm_init(slice, &config, true);

    pwm_set_gpio_level(SPEAKER_PIN, PWM_MID);

    printf("DTMF generator startet\n");

    while (true)
    {
        // Test kun tallet 1 i første omgang
        playKey(0, 0);

        sleep_ms(2000);
    }
}
