#include "Vehicle.h"

#include <cctype>
#include <utility>

#include "Exceptions.h"

std::string Vehicle::normalizePlate(const std::string& raw) {
    std::string normalized;

    for (unsigned char character : raw) {
        if (std::isalnum(character) != 0) {
            normalized.push_back(static_cast<char>(std::toupper(character)));
        }
    }

    if (normalized.size() < 3) {
        throw InvalidInputException(
            "Plate must contain at least 3 letters or digits.");
    }

    if (normalized.size() > 12) {
        throw InvalidInputException(
            "Plate is too long (maximum 12 letters or digits).");
    }

    return normalized;
}

Vehicle::Vehicle(std::string plateNumber)
    : plate_(normalizePlate(plateNumber)) {}

const std::string& Vehicle::plate() const noexcept {
    return plate_;
}

bool Vehicle::operator==(const Vehicle& other) const noexcept {
    return plate_ == other.plate_;
}

bool Vehicle::operator!=(const Vehicle& other) const noexcept {
    return !(*this == other);
}

Bike::Bike(std::string plateNumber)
    : Vehicle(std::move(plateNumber)) {}

VehicleType Bike::type() const noexcept {
    return VehicleType::Bike;
}

std::string Bike::typeName() const {
    return "Bike";
}

double Bike::hourlyRate() const noexcept {
    return 10.0;
}

SpotType Bike::preferredSpotType() const noexcept {
    return SpotType::Compact;
}

bool Bike::canParkIn(SpotType spotType) const noexcept {
    return spotType == SpotType::Compact || spotType == SpotType::Regular;
}

Car::Car(std::string plateNumber)
    : Vehicle(std::move(plateNumber)) {}

VehicleType Car::type() const noexcept {
    return VehicleType::Car;
}

std::string Car::typeName() const {
    return "Car";
}

double Car::hourlyRate() const noexcept {
    return 20.0;
}

SpotType Car::preferredSpotType() const noexcept {
    return SpotType::Regular;
}

bool Car::canParkIn(SpotType spotType) const noexcept {
    return spotType == SpotType::Regular || spotType == SpotType::Large;
}

Truck::Truck(std::string plateNumber)
    : Vehicle(std::move(plateNumber)) {}

VehicleType Truck::type() const noexcept {
    return VehicleType::Truck;
}

std::string Truck::typeName() const {
    return "Truck";
}

double Truck::hourlyRate() const noexcept {
    return 40.0;
}

SpotType Truck::preferredSpotType() const noexcept {
    return SpotType::Large;
}

bool Truck::canParkIn(SpotType spotType) const noexcept {
    return spotType == SpotType::Large;
}

ElectricCar::ElectricCar(std::string plateNumber)
    : Vehicle(std::move(plateNumber)) {}

VehicleType ElectricCar::type() const noexcept {
    return VehicleType::ElectricCar;
}

std::string ElectricCar::typeName() const {
    return "ElectricCar";
}

double ElectricCar::hourlyRate() const noexcept {
    return 25.0;
}

SpotType ElectricCar::preferredSpotType() const noexcept {
    return SpotType::EV;
}

bool ElectricCar::canParkIn(SpotType spotType) const noexcept {
    return spotType == SpotType::EV || spotType == SpotType::Regular;
}
