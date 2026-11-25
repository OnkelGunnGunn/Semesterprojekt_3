
#ifndef FREQUENCY_METER_H
#define FREQUENCY_METER_H

#include <stdint.h>

class FrequencyMeter
{
public:
    FrequencyMeter();

    void init();

    uint16_t readMCP3002();

    // Måler frekvensen og returnerer Hz
    float measureFrequency();

private:
    static constexpr float ADC_VREF = 3.3f;

    static constexpr int SAMPLE_COUNT = 400;
    static constexpr int SAMPLE_RATE = 8000;

    static constexpr int PIN_MISO = 16;
    static constexpr int PIN_CS   = 17;
    static constexpr int PIN_SCK  = 18;
    static constexpr int PIN_MOSI = 19;

    uint16_t samples[SAMPLE_COUNT];
};

#endif
