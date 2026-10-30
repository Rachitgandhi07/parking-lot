#pragma once

#include <string>

#include "Types.h"

class Vehicle {
public:
    explicit Vehicle(std::string plateNumber);
    virtual ~Vehicle() = default;

    Vehicle(const Vehicle&) = default;
    Vehicle& operator=(const Vehicle&) = default;

    static std::string normalizePlate(const std::string& raw);
    const std::string& plate() const noexcept;
    virtual VehicleType type() const noexcept = 0;
    virtual std::string typeName() const = 0;
    virtual double hourlyRate() const noexcept = 0;
    virtual SpotType preferredSpotType() const noexcept = 0;
    virtual bool canParkIn(SpotType spotType) const noexcept = 0;

    bool operator==(const Vehicle& other) const noexcept;
    bool operator!=(const Vehicle& other) const noexcept;

protected:
    std::string plate_;
};

class Bike final : public Vehicle {
public:
    explicit Bike(std::string plateNumber);

    VehicleType type() const noexcept override;
    std::string typeName() const override;
    double hourlyRate() const noexcept override;
    SpotType preferredSpotType() const noexcept override;
    bool canParkIn(SpotType spotType) const noexcept override;
};

class Car final : public Vehicle {
public:
    explicit Car(std::string plateNumber);

    VehicleType type() const noexcept override;
    std::string typeName() const override;
    double hourlyRate() const noexcept override;
    SpotType preferredSpotType() const noexcept override;
    bool canParkIn(SpotType spotType) const noexcept override;
};

class Truck final : public Vehicle {
public:
    explicit Truck(std::string plateNumber);

    VehicleType type() const noexcept override;
    std::string typeName() const override;
    double hourlyRate() const noexcept override;
    SpotType preferredSpotType() const noexcept override;
    bool canParkIn(SpotType spotType) const noexcept override;
};

class ElectricCar final : public Vehicle {
public:
    explicit ElectricCar(std::string plateNumber);

    VehicleType type() const noexcept override;
    std::string typeName() const override;
    double hourlyRate() const noexcept override;
    SpotType preferredSpotType() const noexcept override;
    bool canParkIn(SpotType spotType) const noexcept override;
};
