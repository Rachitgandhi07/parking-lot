#include "Types.h"

#include <algorithm>
#include <cctype>

namespace {

bool equalsIgnoreCase(const std::string& left, const std::string& right) {
    if (left.size() != right.size()) {
        return false;
    }

    return std::equal(
        left.begin(), left.end(), right.begin(),
        [](unsigned char a, unsigned char b) {
            return std::tolower(a) == std::tolower(b);
        });
}

}  // namespace

std::string toString(VehicleType type) {
    switch (type) {
        case VehicleType::Bike:
            return "Bike";
        case VehicleType::Car:
            return "Car";
        case VehicleType::Truck:
            return "Truck";
        case VehicleType::ElectricCar:
            return "ElectricCar";
    }

    return "Unknown";
}

std::string toString(SpotType type) {
    switch (type) {
        case SpotType::Compact:
            return "Compact";
        case SpotType::Regular:
            return "Regular";
        case SpotType::Large:
            return "Large";
        case SpotType::EV:
            return "EV";
    }

    return "Unknown";
}

bool vehicleTypeFromString(const std::string& text, VehicleType& out) {
    if (equalsIgnoreCase(text, "Bike")) {
        out = VehicleType::Bike;
        return true;
    }
    if (equalsIgnoreCase(text, "Car")) {
        out = VehicleType::Car;
        return true;
    }
    if (equalsIgnoreCase(text, "Truck")) {
        out = VehicleType::Truck;
        return true;
    }
    if (equalsIgnoreCase(text, "ElectricCar") || equalsIgnoreCase(text, "EV")) {
        out = VehicleType::ElectricCar;
        return true;
    }

    return false;
}

bool spotTypeFromString(const std::string& text, SpotType& out) {
    if (equalsIgnoreCase(text, "Compact")) {
        out = SpotType::Compact;
        return true;
    }
    if (equalsIgnoreCase(text, "Regular")) {
        out = SpotType::Regular;
        return true;
    }
    if (equalsIgnoreCase(text, "Large")) {
        out = SpotType::Large;
        return true;
    }
    if (equalsIgnoreCase(text, "EV")) {
        out = SpotType::EV;
        return true;
    }

    return false;
}
