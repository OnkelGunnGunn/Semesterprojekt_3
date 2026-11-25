#ifndef DTMF_RECEIVER_H
#define DTMF_RECEIVER_H

#include <stdint.h>

class DTMFReceiver
{
public:

    // Initialiserer SPI og MCP3002
    void begin();

    // Samler lyd, analyserer signalet og returnerer
    // et tastetryk, når en DTMF-tone er registreret.
    // Returnerer '\0', hvis ingen ny tast er registreret.
    char readKey();

private:

    // MCP3002
    uint16_t readMCP3002();

    // DTMF analyse
    double goertzel(const double* signal, double frequency);
    int strongest(double* powers, int start);
    char detectDTMF();

    // Sampling
    static constexpr int SAMPLE_RATE = 8000;
    static constexpr int N = 205;
    static constexpr int SAMPLE_PERIOD_US = 1000000 / SAMPLE_RATE;

    // Mikrofonmålinger
    uint16_t samples[N];

    // Registrering af gentagne tastetryk
    char previousKey = '\0';
    char candidateKey = '\0';
    int candidateCount = 0;
};

#endif