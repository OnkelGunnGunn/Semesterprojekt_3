
#include "AudioPlayer.h"
#include <iostream>
#include <cmath>

AudioPlayer::AudioPlayer()
{
    stream = nullptr;
}

AudioPlayer::~AudioPlayer()
{
    if (stream != nullptr)
    {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
    }

    Pa_Terminate();
}

void AudioPlayer::initialize()
{
    Pa_Initialize();

    Pa_OpenDefaultStream(
        &stream,
        0,
        1,
        paFloat32,
        SAMPLE_RATE,
        256,
        nullptr,
        nullptr
    );

    Pa_StartStream(stream);
}

void AudioPlayer::playTone(int lowFreq, int highFreq)
{
    const int N = SAMPLE_RATE * DURATION;

    std::vector<float> signal(N);

    for (int n = 0; n < N; n++)
    {
        double t = static_cast<double>(n) / SAMPLE_RATE;

        double low = sin(2 * M_PI * lowFreq * t);
        double high = sin(2 * M_PI * highFreq * t);

        signal[n] = (low + high) * AMPLITUDE;
    }

    Pa_WriteStream(stream, signal.data(), N);
}
void AudioPlayer::playSequence(const std::string& data, const DTMFEncoder& encoder)
{
    for (char c : data)
    {
        auto frequencies = encoder.getFrequency(c);

        playTone(frequencies.first, frequencies.second);

        Pa_Sleep(150);
    }
}
