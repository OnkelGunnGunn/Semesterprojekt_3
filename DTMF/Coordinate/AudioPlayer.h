#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <portaudio.h>
#include <vector>
#include <string>
#include "DTMFEncoder.h"

class AudioPlayer
{
private:
    PaStream* stream;

    const int SAMPLE_RATE = 44100;
    const double AMPLITUDE = 0.7;
    const double DURATION = 0.35;

public:
    AudioPlayer();
    ~AudioPlayer();
    void initialize();
    void playTone(int lowFreq, int highFreq);
    void playSequence(const std::string& data, const DTMFEncoder& encoder);
};

#endif