#include "Ticket.h"

#include <ostream>
#include <utility>

#include "Exceptions.h"

int Ticket::nextId_ = 1;

Ticket::Ticket(int ticketId, std::string ticketPlate, int ticketSpotId,
               TimePoint ticketEntryTime)
    : id_(ticketId),
      plate_(std::move(ticketPlate)),
      spotId_(ticketSpotId),
      entry_(ticketEntryTime) {
    if (id_ <= 0) {
        throw InvalidInputException("Ticket id must be positive.");
    }
    if (plate_.empty()) {
        throw InvalidInputException("Ticket plate cannot be empty.");
    }
    if (spotId_ <= 0) {
        throw InvalidInputException("Ticket spot id must be positive.");
    }
}

int Ticket::id() const noexcept {
    return id_;
}

const std::string& Ticket::plate() const noexcept {
    return plate_;
}

int Ticket::spotId() const noexcept {
    return spotId_;
}

TimePoint Ticket::entryTime() const noexcept {
    return entry_;
}

TimePoint Ticket::exitTime() const noexcept {
    return exit_;
}

double Ticket::fee() const noexcept {
    return fee_;
}

bool Ticket::isActive() const noexcept {
    return active_;
}

std::chrono::minutes Ticket::duration() const noexcept {
    return duration(active_ ? Clock::now() : exit_);
}

std::chrono::minutes Ticket::duration(TimePoint end) const noexcept {
    if (end <= entry_) {
        return std::chrono::minutes(0);
    }

    return std::chrono::duration_cast<std::chrono::minutes>(end - entry_);
}

void Ticket::close(TimePoint closingTime, double amount) {
    if (!active_) {
        throw InvalidOperationException("Ticket #" + std::to_string(id_) +
                                        " is already closed.");
    }
    if (closingTime < entry_) {
        throw InvalidInputException("Exit time cannot be before entry time.");
    }
    if (amount < 0.0) {
        throw InvalidInputException("Ticket fee cannot be negative.");
    }

    exit_ = closingTime;
    fee_ = amount;
    active_ = false;
}

bool Ticket::operator<(const Ticket& other) const noexcept {
    if (entry_ != other.entry_) {
        return entry_ < other.entry_;
    }
    return id_ < other.id_;
}

bool Ticket::operator==(const Ticket& other) const noexcept {
    return id_ == other.id_;
}

std::ostream& operator<<(std::ostream& os, const Ticket& ticket) {
    os << '#' << ticket.id_ << " | " << ticket.plate_ << " | spot "
       << ticket.spotId_ << " | in " << formatTime(ticket.entry_) << " | ";

    if (ticket.active_) {
        os << "ACTIVE (" << formatDuration(ticket.duration())
           << ") | fee pending";
    } else {
        os << "out " << formatTime(ticket.exit_) << " | "
           << formatDuration(ticket.duration()) << " | fee "
           << formatMoney(ticket.fee_);
    }

    return os;
}

int Ticket::nextId() {
    return nextId_++;
}

void Ticket::setNextId(int value) noexcept {
    nextId_ = (value > 0) ? value : 1;
}

void Ticket::ensureNextIdAbove(int usedId) noexcept {
    if (nextId_ <= usedId) {
        nextId_ = usedId + 1;
    }
}
