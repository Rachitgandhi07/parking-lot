#pragma once

#include <string>

enum class VehicleType {
    Bike,
    Car,
    Truck,
    ElectricCar
};

enum class SpotType {
    Compact,
    Regular,
    Large,
    EV
};

std::string toString(VehicleType type);
std::string toString(SpotType type);

bool vehicleTypeFromString(const std::string& text, VehicleType& out);
bool spotTypeFromString(const std::string& text, SpotType& out);
