
#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"

const uint ADC_PIN = 26;             // GP26 = ADC0
const uint SAMPLE_RATE = 8000;
const uint SAMPLE_PERIOD_US = 125;
const int N = 512;

const float LOW_FREQS[4]  = {697, 770, 852, 941};
const float HIGH_FREQS[4] = {1209, 1336, 1477, 1633};

const char KEYPAD[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

uint16_t samples[N];

// Beregn signalstyrken ved en bestemt frekvens med Goertzel

float goertzel(float frequency, float mean) {
    const float PI = 3.14159265f;

    float omega = 2.0f * PI * frequency / SAMPLE_RATE;
    float coeff = 2.0f * cosf(omega);

    float s1 = 0.0f;
    float s2 = 0.0f;

    for (int i = 0; i < N; i++) {
        // Hann-vindue reducerer frekvenslækage
        float window = 0.5f - 0.5f *
            cosf(2.0f * PI * i / (N - 1));

        float sample = ((float)samples[i] - mean) * window;
        float s0 = sample + coeff * s1 - s2;

        s2 = s1;
        s1 = s0;
    }

    return s1 * s1 + s2 * s2 - coeff * s1 * s2;
}


// Find den stærkeste frekvens i en gruppe
int strongestFrequency(const float powers[4]) {
    int best = 0;

    for (int i = 1; i < 4; i++) {
        if (powers[i] > powers[best]) {
            best = i;
        }
    }

    return best;
}

// Find det næststærkeste signal
float secondStrongest(const float powers[4], int best) {
    float second = 0.0f;

    for (int i = 0; i < 4; i++) {
        if (i != best && powers[i] > second) {
            second = powers[i];
        }
    }

    return second;
}

char detectDTMF(int &lowIndex, int &highIndex) {
    uint32_t sum = 0;

    for (int i = 0; i < N; i++) {
        sum += samples[i];
    }

    float mean = (float)sum / N;

    // Kontroller lydniveauet efter fjernelse af DC-offset
    float sumSquares = 0.0f;

    for (int i = 0; i < N; i++) {
        float x = (float)samples[i] - mean;
        sumSquares += x * x;
    }

    float rms = sqrtf(sumSquares / N);

    // Filtrer meget svage signaler og almindelig støj
    if (rms < 12.0f) {
        return '\0';
    }

    float lowPower[4];
    float highPower[4];

    for (int i = 0; i < 4; i++) {
        lowPower[i] = goertzel(LOW_FREQS[i], mean);
        highPower[i] = goertzel(HIGH_FREQS[i], mean);
    }

    int low = strongestFrequency(lowPower);
    int high = strongestFrequency(highPower);

    float lowSecond = secondStrongest(lowPower, low);
    float highSecond = secondStrongest(highPower, high);

    // Kræv at de valgte frekvenser er tydeligt stærkere
    // end de øvrige frekvenser i hver gruppe.
    if (lowSecond > 0.0f &&
        lowPower[low] < 2.0f * lowSecond) {
        return '\0';
    }

    if (highSecond > 0.0f &&
        highPower[high] < 2.0f * highSecond) {
        return '\0';
    }

    lowIndex = low;
    highIndex = high;

    return KEYPAD[low][high];
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);

    printf("\nDTMF-modtager startet!\n");
    printf("Afspil en DTMF-tone foran mikrofonen.\n\n");

    char candidate = '\0';
    char lastPrinted = '\0';
    int sameCount = 0;
    int silenceCount = 0;

    while (true) {
        // Sampl med cirka 8 kHz
        uint64_t start = time_us_64();

        for (int i = 0; i < N; i++) {
            busy_wait_until(
                from_us_since_boot(
                    start + (uint64_t)i * SAMPLE_PERIOD_US
                )
            );

            samples[i] = adc_read();
        }

        int lowIndex = 0;
        int highIndex = 0;

        char symbol = detectDTMF(lowIndex, highIndex);

        if (symbol == '\0') {
            sameCount = 0;
            candidate = '\0';
            silenceCount++;

            // Nulstil efter en kort periode uden genkendt tone
            if (silenceCount >= 3) {
                lastPrinted = '\0';
            }
        } else {
            silenceCount = 0;

            if (symbol == candidate) {
                sameCount++;
            } else {
                candidate = symbol;
                sameCount = 1;
            }

            // Kræv to ens analyser, før symbolet udskrives
            if (sameCount >= 2 && symbol != lastPrinted) {
                printf("DTMF: %c | Lav: %.0f Hz | Hoj: %.0f Hz\n",
                       symbol,
                       LOW_FREQS[lowIndex],
                       HIGH_FREQS[highIndex]);

                lastPrinted = symbol;
            }
        }
    }

    return 0;
}
