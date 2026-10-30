#pragma once

#include <chrono>
#include <iosfwd>
#include <string>

#include "TimeUtils.h"

class Ticket {
public:
    Ticket(int ticketId, std::string ticketPlate, int ticketSpotId,
           TimePoint ticketEntryTime);

    int id() const noexcept;
    const std::string& plate() const noexcept;
    int spotId() const noexcept;
    TimePoint entryTime() const noexcept;
    TimePoint exitTime() const noexcept;
    double fee() const noexcept;
    bool isActive() const noexcept;
    std::chrono::minutes duration() const noexcept;
    std::chrono::minutes duration(TimePoint end) const noexcept;

    void close(TimePoint closingTime, double amount);

    bool operator<(const Ticket& other) const noexcept;
    bool operator==(const Ticket& other) const noexcept;

    friend std::ostream& operator<<(std::ostream& os, const Ticket& ticket);

    static int nextId();
    static void setNextId(int value) noexcept;
    static void ensureNextIdAbove(int usedId) noexcept;

private:
    int id_;
    std::string plate_;
    int spotId_;
    TimePoint entry_;
    TimePoint exit_{};
    double fee_ = 0.0;
    bool active_ = true;

    static int nextId_;
};
