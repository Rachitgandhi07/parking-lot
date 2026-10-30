#pragma once

#include "Types.h"

class Vehicle;

class ParkingSpot {
public:
    explicit ParkingSpot(int spotId, SpotType spotType);

    int id() const noexcept;
    SpotType type() const noexcept;
    bool isOccupied() const noexcept;
    bool canFit(const Vehicle& vehicle) const noexcept;

    void occupy();
    void release();

private:
    int id_;
    SpotType type_;
    bool occupied_ = false;
};
