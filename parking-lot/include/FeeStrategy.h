#pragma once

#include <memory>
#include <string>

#include "TimeUtils.h"

class Vehicle;

class FeeStrategy {
public:
    virtual ~FeeStrategy() = default;

    virtual double calculateFee(const Vehicle& vehicle, TimePoint entry,
                                TimePoint exit) const = 0;
    virtual std::string key() const = 0;
    virtual std::string name() const = 0;
};

class HourlyFeeStrategy final : public FeeStrategy {
public:
    double calculateFee(const Vehicle& vehicle, TimePoint entry,
                        TimePoint exit) const override;
    std::string key() const override;
    std::string name() const override;
};

class DayPassFeeStrategy final : public FeeStrategy {
public:
    double calculateFee(const Vehicle& vehicle, TimePoint entry,
                        TimePoint exit) const override;
    std::string key() const override;
    std::string name() const override;
};

std::unique_ptr<FeeStrategy> makeFeeStrategy(const std::string& key);
