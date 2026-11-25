#include "DTMFEncoder.h"
#include <stdexcept>

DTMFEncoder::DTMFEncoder()
{
    frequencies = {
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
}

std::pair<int, int> DTMFEncoder::getFrequency(char key) const
{
    auto it = frequencies.find(key);

    if (it == frequencies.end())
    {
        throw std::invalid_argument("Invalid DTMF key");
    }

    return it->second;
}


std::string DTMFEncoder::encode(const std::string& data) const
{
    std::string encoded;

    for (char c : data)
    {
        if (c == '.')
        {
            encoded += '*';
        }
        else if (c == ',')
        {
            encoded += '#';
        }
        else
        {
            getFrequency(c);
            encoded += c;
        }
    }

    return encoded;
}