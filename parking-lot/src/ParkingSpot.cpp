#include "ParkingSpot.h"

#include <string>

#include "Exceptions.h"
#include "Vehicle.h"

ParkingSpot::ParkingSpot(int spotId, SpotType spotType)
    : id_(spotId), type_(spotType) {
    if (id_ <= 0) {
        throw InvalidInputException("Parking spot id must be positive.");
    }
}

int ParkingSpot::id() const noexcept {
    return id_;
}

SpotType ParkingSpot::type() const noexcept {
    return type_;
}

bool ParkingSpot::isOccupied() const noexcept {
    return occupied_;
}

bool ParkingSpot::canFit(const Vehicle& vehicle) const noexcept {
    return !occupied_ && vehicle.canParkIn(type_);
}

void ParkingSpot::occupy() {
    if (occupied_) {
        throw InvalidOperationException("Parking spot " + std::to_string(id_) +
                                        " is already occupied.");
    }

    occupied_ = true;
}

void ParkingSpot::release() {
    if (!occupied_) {
        throw InvalidOperationException("Parking spot " + std::to_string(id_) +
                                        " is already free.");
    }

    occupied_ = false;
}
