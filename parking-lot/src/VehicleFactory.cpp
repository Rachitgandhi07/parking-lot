#include "VehicleFactory.h"

#include "Exceptions.h"
#include "Vehicle.h"

std::unique_ptr<Vehicle> VehicleFactory::create(VehicleType type,
                                                const std::string& plate) {
    switch (type) {
        case VehicleType::Bike:
            return std::make_unique<Bike>(plate);
        case VehicleType::Car:
            return std::make_unique<Car>(plate);
        case VehicleType::Truck:
            return std::make_unique<Truck>(plate);
        case VehicleType::ElectricCar:
            return std::make_unique<ElectricCar>(plate);
    }

    throw InvalidInputException("Unsupported vehicle type.");
}

std::unique_ptr<Vehicle> VehicleFactory::createFromName(
    const std::string& typeName, const std::string& plate) {
    VehicleType type{};
    if (!vehicleTypeFromString(typeName, type)) {
        throw InvalidInputException("Unknown vehicle type: " + typeName);
    }

    return create(type, plate);
}
