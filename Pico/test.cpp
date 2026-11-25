
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <math.h>

const uint PIN = 15;

const int PWM_MID = 250;
const int AMPLITUDE = 50;

const uint32_t SAMPLE_RATE = 32000;
const uint32_t PWM_WRAP = 499;

const float PI = 3.14159265f;

int main()
{
    gpio_set_function(PIN, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(PIN);

    pwm_config config = pwm_get_default_config();

    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_config_set_wrap(&config, PWM_WRAP);

    pwm_init(slice, &config, true);

    float phase = 0.0f;
    float increment = 2.0f * PI * 1000.0f / SAMPLE_RATE;

    absolute_time_t nextSample = get_absolute_time();

    while (true)
    {
        float signal = sinf(phase);

        int pwmValue = PWM_MID + (int)(signal * AMPLITUDE);

        pwm_set_gpio_level(PIN, pwmValue);

        phase += increment;

        if (phase >= 2.0f * PI)
            phase -= 2.0f * PI;

        nextSample = delayed_by_us(nextSample, 31);

        // Korriger gennemsnittet til 31,25 us
        static int count = 0;

        count++;

        if (count == 4)
        {
            nextSample = delayed_by_us(nextSample, 1);
            count = 0;
        }

        busy_wait_until(nextSample);
    }
}
