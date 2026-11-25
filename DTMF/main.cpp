#include <iostream>
#include <string>

#include "Coordinate/Coordinate.h"
#include "Coordinate/DTMFEncoder.h"
#include "Coordinate/AudioPlayer.h"

int main()
{

    DTMFEncoder encoder;
    AudioPlayer player;

    /*
    std::cout << "Enter latitude: ";
    double lat;
    std::cin >> lat;

    std::cout << "Enter longitude: ";
    double lon;
    std::cin >> lon;

    std::cout << "Enter orientation: ";
    double ori;
    std::cin >> ori;
*/
    Coordinate position(22.1111, 22.2222, 67);

    std::string PosToString = position.toString();

    std::string EncodedData = encoder.encode(PosToString);

    std::string message = "6666A1111B";

    player.initialize();

    player.playSequence(message, encoder);

    // player.playSequence(EncodedData, encoder);
    player.~AudioPlayer();
    return 0;
}