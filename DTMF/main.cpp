#include <iostream>
#include <string>

#include "Coordinate/Coordinate.h"
#include "Coordinate/DTMFEncoder.h"
#include "Coordinate/AudioPlayer.h"
#include "Coordinate/AudioReceiver.h"

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

    Coordinate position(11.1111, 22.2222, 67);

    std::string PosToString = position.toString();

    std::string EncodedData = encoder.encode(PosToString);



    player.initialize();
    player.playSequence(EncodedData, encoder);
    player.~AudioPlayer();
    return 0;
}