#include "TimeUtils.h"

#include <iomanip>
#include <sstream>

std::string formatTime(TimePoint timePoint) {
    const std::time_t raw = Clock::to_time_t(timePoint);
    const std::tm* local = std::localtime(&raw);

    if (local == nullptr) {
        return "unknown";
    }

    const std::tm copy = *local;
    std::ostringstream out;
    out << std::put_time(&copy, "%d-%b-%Y %H:%M");
    return out.str();
}

std::string formatDuration(std::chrono::minutes duration) {
    long long totalMinutes = duration.count();
    if (totalMinutes < 0) {
        totalMinutes = 0;
    }

    const long long days = totalMinutes / (24 * 60);
    const long long hours = (totalMinutes / 60) % 24;
    const long long minutes = totalMinutes % 60;

    std::ostringstream out;
    if (days > 0) {
        out << days << "d ";
    }
    if (days > 0 || hours > 0) {
        out << hours << "h ";
    }
    out << minutes << "m";
    return out.str();
}

std::string formatMoney(double amount) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << amount;
    return out.str();
}

std::time_t toTimeT(TimePoint timePoint) {
    return Clock::to_time_t(timePoint);
}

TimePoint fromTimeT(std::time_t value) {
    return Clock::from_time_t(value);
}
