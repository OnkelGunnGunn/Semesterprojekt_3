
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdint.h>
#include <stdio.h>

// GPIO og PWM-indstillinger
const uint SPEAKER_PIN = 15;

constexpr uint32_t PWM_WRAP = 499; 
constexpr uint32_t SAMPLE_RATE = 32000;
constexpr uint32_t PWM_MID = 250;
constexpr int32_t AMPLITUDE = 80;

// 256 punkters sinustabel
const int16_t sineTable[256] = {
    0, 804, 1608, 2410, 3212, 4011, 4808, 5602,
    6393, 7179, 7962, 8739, 9512, 10278, 11039, 11793,
    12539, 13278, 14009, 14732, 15446, 16150, 16845, 17530,
    18204, 18867, 19519, 20159, 20787, 21403, 22005, 22594,
    23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790,
    27245, 27683, 28004, 28308, 28594, 28863, 29114, 29347,
    29561, 29757, 29934, 30092, 30231, 30351, 30451, 30531,
    30592, 30633, 30653, 30654, 30633, 30592, 30531, 30451,
    30351, 30231, 30092, 29934, 29757, 29561, 29347, 29114,
    28863, 28594, 28308, 28004, 27683, 27245, 26790, 26319,
    25832, 25329, 24811, 24279, 23731, 23170, 22594, 22005,
    21403, 20787, 20159, 19519, 18867, 18204, 17530, 16845,
    16150, 15446, 14732, 14009, 13278, 12539, 11793, 11039,
    10278, 9512, 8739, 7962, 7179, 6393, 5602, 4808,
    4011, 3212, 2410, 1608, 804, 0, -804, -1608,
    -2410, -3212, -4011, -4808, -5602, -6393, -7179, -7962,
    -8739, -9512, -10278, -11039, -11793, -12539, -13278, -14009,
    -14732, -15446, -16150, -16845, -17530, -18204, -18867, -19519,
    -20159, -20787, -21403, -22005, -22594, -23170, -23731, -24279,
    -24811, -25329, -25832, -26319, -26790, -27245, -27683, -28004,
    -28308, -28594, -28863, -29114, -29347, -29561, -29757, -29934,
    -30092, -30231, -30351, -30451, -30531, -30592, -30633, -30653,
    -30654, -30633, -30592, -30531, -30451, -30351, -30231, -30092,
    -29934, -29757, -29561, -29347, -29114, -28863, -28594, -28308,
    -28004, -27683, -27245, -26790, -26319, -25832, -25329, -24811,
    -24279, -23731, -23170, -22594, -22005, -21403, -20787, -20159,
    -19519, -18867, -18204, -17530, -16845, -16150, -15446, -14732,
    -14009, -13278, -12539, -11793, -11039, -10278, -9512, -8739,
    -7962, -7179, -6393, -5602, -4808, -4011, -3212, -2410,
    -1608, -804
};

// DTMF-frekvenser
const uint16_t lowFreq[4]  = {697, 770, 852, 941};
const uint16_t highFreq[4] = {1209, 1336, 1477, 1633};

const char keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// Beregn faseinkrement til DDS
uint32_t phaseIncrement(uint32_t frequency)
{
    return (uint32_t)(((uint64_t)frequency << 32) / SAMPLE_RATE);
}

// Afspil to frekvenser samtidigt
void playDTMF(uint32_t low, uint32_t high, uint32_t durationMs)
{
    uint32_t phaseLow = 0;
    uint32_t phaseHigh = 0;

    uint32_t stepLow = phaseIncrement(low);
    uint32_t stepHigh = phaseIncrement(high);

    uint32_t samples = durationMs * SAMPLE_RATE / 1000;

    absolute_time_t nextSample = get_absolute_time();
    uint8_t fractionalUs = 0;

    for (uint32_t i = 0; i < samples; i++)
    {
        uint8_t indexLow = phaseLow >> 24;
        uint8_t indexHigh = phaseHigh >> 24;

        int32_t sample =
            (int32_t)sineTable[indexLow] +
            (int32_t)sineTable[indexHigh];

        int32_t pwmLevel =
            PWM_MID + (sample * AMPLITUDE) / 32767;

        pwm_set_gpio_level(SPEAKER_PIN, pwmLevel);

        phaseLow += stepLow;
        phaseHigh += stepHigh;

        // 32 kHz = 31,25 mikrosekunder pr. sample
        nextSample = delayed_by_us(nextSample, 31);

        fractionalUs++;

        if (fractionalUs == 4)
        {
            nextSample = delayed_by_us(nextSample, 1);
            fractionalUs = 0;
        }

        busy_wait_until(nextSample);
    }

    pwm_set_gpio_level(SPEAKER_PIN, PWM_MID);
}

// Afspil én DTMF-tast
void playKey(uint row, uint column)
{
    printf("Tast %c: %u + %u Hz\n",
           keys[row][column],
           lowFreq[row],
           highFreq[column]);

    playDTMF(lowFreq[row], highFreq[column], 500);

    sleep_ms(250);
}

int main()
{
    stdio_init_all();
    sleep_ms(1000);

    gpio_set_function(SPEAKER_PIN, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(SPEAKER_PIN);

    pwm_config config = pwm_get_default_config();

    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_config_set_wrap(&config, PWM_WRAP);

    pwm_init(slice, &config, true);

    pwm_set_gpio_level(SPEAKER_PIN, PWM_MID);

    printf("DTMF-test starter\n");

    while (true)
    {
        for (uint row = 0; row < 4; row++)
        {
            for (uint column = 0; column < 4; column++)
            {
                playKey(row, column);
            }
        }

        printf("Alle 16 toner afspillet\n");

        sleep_ms(1500);
    }
}
