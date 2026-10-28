#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"


// -------------------------
// DTMF FREKVENSER
// -------------------------

const float dtmf_freqs[8] = {
    697, 770, 852, 941,
    1209, 1336, 1477, 1633
};


// -------------------------
// DTMF TABEL
// -------------------------

const char dtmf_keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};


// -------------------------
// GOERTZEL
// -------------------------

float goertzel(float* samples, int N, float frequency)
{
    float sample_rate = 8000.0f;

    float omega =
        2.0f * M_PI * frequency / sample_rate;

    float coeff = 2.0f * cos(omega);

    float s_prev = 0;
    float s_prev2 = 0;

    for (int n = 0; n < N; n++)
    {
        float s =
            samples[n] +
            coeff * s_prev -
            s_prev2;

        s_prev2 = s_prev;
        s_prev = s;
    }

    return
        s_prev2 * s_prev2 +
        s_prev * s_prev -
        coeff * s_prev * s_prev2;
}


// -------------------------
// MAIN
// -------------------------

int main()
{
    stdio_init_all();


    // -------------------------
    // ADC / MIKROFON
    // -------------------------

    const uint MIC_PIN = 26;

    adc_init();

    adc_gpio_init(MIC_PIN);

    // GP27 = ADC1
    adc_select_input(0);


    // -------------------------
    // SERVO PWM
    // -------------------------

    const uint SERVO_PIN = 2;

    gpio_set_function(
        SERVO_PIN,
        GPIO_FUNC_PWM
    );

    uint slice_num =
        pwm_gpio_to_slice_num(SERVO_PIN);

    uint chan =
        pwm_gpio_to_channel(SERVO_PIN);

    pwm_set_wrap(
        slice_num,
        24999
    );

    pwm_set_clkdiv(
        slice_num,
        100.0f
    );

    // Startposition
    pwm_set_chan_level(
        slice_num,
        chan,
        1875
    );

    pwm_set_enabled(
        slice_num,
        true
    );


    printf("Pico klar - lytter efter DTMF\n");


    // -------------------------
    // SAMPLES
    // -------------------------

    const int N = 205;

    float samples[N];
    bool servo_moved = false;


    // -------------------------
    // LOOP
    // -------------------------
    while (true)
    {

        // -------------------------
        // LÆS MIKROFON
        // -------------------------

        for (int n = 0; n < N; n++)
        {
            uint16_t adc_value = adc_read();

            // ADC går fra 0 til 4095
            // Lav det om til ca. -1 til +1

            samples[n] =
                ((float)adc_value - 2048.0f)
                / 2048.0f;

            // 8000 samples/sekund
            sleep_us(125);
        }


        // -------------------------
        // TEST ALLE 8 FREKVENSER
        // -------------------------

        float powers[8];

        for (int i = 0; i < 8; i++)
        {
            powers[i] =
                goertzel(
                    samples,
                    N,
                    dtmf_freqs[i]
                );
        }


        // -------------------------
        // FIND STÆRKESTE LAVE FREKVENS
        // -------------------------

        int low_index = 0;

        for (int i = 1; i < 4; i++)
        {
            if (powers[i] > powers[low_index])
            {
                low_index = i;
            }
        }


        // -------------------------
        // FIND STÆRKESTE HØJE FREKVENS
        // -------------------------

        int high_index = 4;

        for (int i = 5; i < 8; i++)
        {
            if (powers[i] > powers[high_index])
            {
                high_index = i;
            }
        }


        // -------------------------
        // VIS POWERS
        // -------------------------

        printf(
            "697: %.0f | 770: %.0f | 852: %.0f | 941: %.0f | "
            "1209: %.0f | 1336: %.0f | 1477: %.0f | 1633: %.0f\n",

            powers[0],
            powers[1],
            powers[2],
            powers[3],
            powers[4],
            powers[5],
            powers[6],
            powers[7]
        );

        sleep_ms(500);


        // -------------------------
        // CHECK DTMF
        // -------------------------

        /*
        if (powers[low_index] > 0.5 &&
            powers[high_index] > 0.5)
        {
            char key =
                dtmf_keys
                [low_index]
                [high_index - 4];


            printf(
                "DTMF: %c\n",
                key
            );


            if (key == '5' && servo_moved == false)
            {
                servo_moved = true;

                printf(
                    "DTMF 5 - AABNER SERVO!\n"
                );

                pwm_set_chan_level(
                    slice_num,
                    chan,
                    1000
                );

                sleep_ms(1000);

                pwm_set_chan_level(
                    slice_num,
                    chan,
                    1875
                );

                sleep_ms(1000);
            }
        }
        */
    }
}