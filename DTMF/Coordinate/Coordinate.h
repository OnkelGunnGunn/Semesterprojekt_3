#ifndef COORDINATE_H
#define COORDINATE_H
#include <string>

class Coordinate
{
private:
    double latitude;
    double longitude;
    double Orientation;

public:
    Coordinate(double lat, double lon, double ori);

    double getLatitude() const;
    double getLongitude() const;
    double getOrientation() const;
    static Coordinate fromString(const std::string& message);
    std::string toString() const;
};

#endif