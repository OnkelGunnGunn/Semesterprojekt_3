
#include <iostream>
#include <map>
#include <utility>
#include <cmath>
#include <vector>
#include <portaudio.h>
#include <fstream>

const int SAMPLE_RATE = 44100;
const double DURATION = 0.35;
const double PI = 3.14159265358979323846;


int main()
{

struct DTMF {
    int low_freq;
    int high_freq;
};
std::vector<char> keys = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'
};



std::map<char, DTMF> dtmf_frequencies = {
    {'1', {697, 1209}},
    {'2', {697, 1336}},
    {'3', {697, 1477}},
    {'A', {697, 1633}},

    {'4', {770, 1209}},
    {'5', {770, 1336}},
    {'6', {770, 1477}},
    {'B', {770, 1633}},

    {'7', {852, 1209}},
    {'8', {852, 1336}},
    {'9', {852, 1477}},
    {'C', {852, 1633}},

    {'*', {941, 1209}},
    {'0', {941, 1336}},
    {'#', {941, 1477}},
    {'D', {941, 1633}}
};

    Pa_Initialize();

    PaStream* stream;
    
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

    const int N = SAMPLE_RATE * DURATION;

    std::vector<float> signal(N);


    Pa_StartStream(stream);
    std::ofstream file("signal.txt");

    for (size_t i = 0; i < keys.size(); i++)
    {


    for (int n = 0; n < N; n++)
    {
        double t = (double)n / SAMPLE_RATE;

        double low = 1 * sin(2 * PI * dtmf_frequencies[keys[i]].low_freq * t);
        double high = 1 * sin(2 * PI * dtmf_frequencies[keys[i]].high_freq * t);

        signal[n] = (low + high) * 0.7;
                file << t << " "
                     << low << " "
                     << high << " "
                     << signal[n] << "\n";
    }
    file << "\n\n";
    std::cout << "Playing DTMF tone for key: " << keys[i] << std::endl;
    Pa_WriteStream(stream, signal.data(), N);
    Pa_Sleep(300); // Sleep for 300 milliseconds between tones

}
    file.close();



    Pa_StopStream(stream);
    Pa_CloseStream(stream);

    Pa_Terminate();

    return 0;
}
