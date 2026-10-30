#pragma once

#include <memory>
#include <string>

#include "Types.h"

class Vehicle;

class VehicleFactory {
public:
    static std::unique_ptr<Vehicle> create(VehicleType type,
                                           const std::string& plate);
    static std::unique_ptr<Vehicle> createFromName(const std::string& typeName,
                                                   const std::string& plate);
};
