#include "FeeStrategy.h"

#include <algorithm>
#include <cmath>

#include "Exceptions.h"
#include "Vehicle.h"

namespace {

double billableHours(TimePoint entry, TimePoint exit) {
    if (exit <= entry) {
        return 1.0;
    }

    const auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(exit - entry).count();
    double hours = std::ceil(static_cast<double>(seconds) / 3600.0);

    // The demo tariff charges for at least one hour and caps a single
    // ticket at eight chargeable hours.
    hours = std::max(1.0, hours);
    hours = std::min(8.0, hours);
    return hours;
}

}  // namespace

double HourlyFeeStrategy::calculateFee(const Vehicle& vehicle, TimePoint entry,
                                       TimePoint exit) const {
    return vehicle.hourlyRate() * billableHours(entry, exit);
}

std::string HourlyFeeStrategy::key() const {
    return "hourly";
}

std::string HourlyFeeStrategy::name() const {
    return "Hourly (min 1h, 8h cap)";
}

double DayPassFeeStrategy::calculateFee(const Vehicle&, TimePoint,
                                        TimePoint) const {
    return 120.0;
}

std::string DayPassFeeStrategy::key() const {
    return "daypass";
}

std::string DayPassFeeStrategy::name() const {
    return "Flat day pass (120.00)";
}

std::unique_ptr<FeeStrategy> makeFeeStrategy(const std::string& key) {
    if (key == "hourly") {
        return std::make_unique<HourlyFeeStrategy>();
    }
    if (key == "daypass") {
        return std::make_unique<DayPassFeeStrategy>();
    }

    throw InvalidInputException("Unknown fee strategy: " + key);
}
