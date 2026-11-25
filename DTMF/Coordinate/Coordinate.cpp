#include "Coordinate.h"
#include <sstream>
#include <iomanip>

// Constructor
Coordinate::Coordinate(double lat, double lon, double ori)
{
    latitude = lat;
    longitude = lon;
    Orientation = ori;
}

// Getters
double Coordinate::getLatitude() const
{
    return latitude;
}

double Coordinate::getLongitude() const
{
    return longitude;
}

double Coordinate::getOrientation() const
{
    return Orientation;
}

// Convert coordinate to string
std::string Coordinate::toString() const
{
    std::stringstream ss;

    ss << std::fixed << std::setprecision(4)
       << latitude << "," << longitude<<"," << std::setprecision(0) << Orientation;

    return ss.str();
}

Coordinate Coordinate::fromString(const std::string& message)
{
    std::stringstream ss(message);
    std::string latStr, lonStr, oriStr;

    std::getline(ss, latStr, ',');
    std::getline(ss, lonStr, ',');
    std::getline(ss, oriStr, ',');

    double lat = std::stod(latStr);
    double lon = std::stod(lonStr);
    double ori = std::stod(oriStr);

    return Coordinate(lat, lon, ori);
}