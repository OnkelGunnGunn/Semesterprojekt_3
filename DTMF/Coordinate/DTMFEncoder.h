#ifndef DTMFENCODER_H
#define DTMFENCODER_H

#include <string>
#include <map>
#include <utility>

class DTMFEncoder
{
private:
    std::map<char, std::pair<int, int>> frequencies;

public:
    DTMFEncoder();

    std::pair<int, int> getFrequency(char key) const;

    std::string encode(const std::string& data) const;
};

#endif