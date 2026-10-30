#include "ParkingLot.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <system_error>
#include <utility>

#include "Exceptions.h"
#include "VehicleFactory.h"

namespace {

constexpr int kStateVersion = 1;

template <typename T>
void readOrThrow(std::istream& input, T& value, const std::string& field) {
    if (!(input >> value)) {
        throw FileIOException("Corrupt state file: expected " + field + ".");
    }
}

}  // namespace

ParkingLot::ParkingLot(std::vector<ParkingSpot> spots,
                       std::unique_ptr<FeeStrategy> strategy)
    : spots_(std::move(spots)), strategy_(std::move(strategy)) {
    if (spots_.empty()) {
        throw InvalidInputException("Parking lot must have at least one spot.");
    }
    if (!strategy_) {
        throw InvalidInputException("Fee strategy cannot be null.");
    }

    std::vector<int> ids;
    ids.reserve(spots_.size());
    for (const ParkingSpot& spot : spots_) {
        ids.push_back(spot.id());
    }

    std::sort(ids.begin(), ids.end());
    if (std::adjacent_find(ids.begin(), ids.end()) != ids.end()) {
        throw InvalidInputException("Parking spot ids must be unique.");
    }
}

Ticket ParkingLot::park(std::unique_ptr<Vehicle> vehicle) {
    if (!vehicle) {
        throw InvalidInputException("Cannot park an empty vehicle.");
    }

    const std::string plate = vehicle->plate();
    if (active_.find(plate) != active_.end()) {
        throw DuplicateVehicleException("Vehicle " + plate +
                                        " is already parked.");
    }

    ParkingSpot* spot = findAvailableSpot(*vehicle);
    if (spot == nullptr) {
        throw ParkingFullException("No compatible spot is available for a " +
                                   vehicle->typeName() + " (" + plate + ").");
    }

    const Ticket ticket(Ticket::nextId(), plate, spot->id(), Clock::now());
    ActiveParking entry{std::move(vehicle), ticket};

    spot->occupy();
    try {
        active_.emplace(plate, std::move(entry));
    } catch (...) {
        spot->release();
        throw;
    }

    return ticket;
}

Ticket ParkingLot::unpark(const std::string& plate) {
    const std::string normalizedPlate = Vehicle::normalizePlate(plate);
    auto active = active_.find(normalizedPlate);
    if (active == active_.end()) {
        throw VehicleNotFoundException("No active ticket found for vehicle " +
                                       normalizedPlate + ".");
    }

    ParkingSpot* spot = findSpotById(active->second.ticket.spotId());
    if (spot == nullptr) {
        throw InvalidOperationException(
            "Parking state is inconsistent: ticket references a missing spot.");
    }

    const TimePoint exitTime = Clock::now();
    const double fee = strategy_->calculateFee(
        *active->second.vehicle, active->second.ticket.entryTime(), exitTime);

    Ticket closed = active->second.ticket;
    closed.close(exitTime, fee);

    history_.push_back(closed);
    revenue_ += fee;
    spot->release();
    active_.erase(active);

    return closed;
}

void ParkingLot::setFeeStrategy(std::unique_ptr<FeeStrategy> strategy) {
    if (!strategy) {
        throw InvalidInputException("Fee strategy cannot be null.");
    }

    strategy_ = std::move(strategy);
}

const FeeStrategy& ParkingLot::feeStrategy() const noexcept {
    return *strategy_;
}

void ParkingLot::printStatus(std::ostream& os) const {
    os << std::left << std::setw(6) << "Spot" << std::setw(10) << "Type"
       << std::setw(12) << "Status" << "Vehicle\n";
    os << "----------------------------------------\n";

    for (const ParkingSpot& spot : spots_) {
        os << std::left << std::setw(6) << spot.id() << std::setw(10)
           << toString(spot.type()) << std::setw(12)
           << (spot.isOccupied() ? "OCCUPIED" : "FREE")
           << plateAtSpot(spot.id()) << '\n';
    }

    os << "\n" << occupiedSpotCount() << "/" << spots_.size()
       << " spots occupied.\n";
}

void ParkingLot::printActiveTickets(std::ostream& os) const {
    if (active_.empty()) {
        os << "No active vehicles.\n";
        return;
    }

    std::vector<const Ticket*> tickets;
    tickets.reserve(active_.size());
    for (const auto& item : active_) {
        tickets.push_back(&item.second.ticket);
    }

    std::sort(tickets.begin(), tickets.end(),
              [](const Ticket* left, const Ticket* right) {
                  return *left < *right;
              });

    for (const Ticket* ticket : tickets) {
        os << *ticket << '\n';
    }
}

void ParkingLot::printHistory(std::ostream& os) const {
    if (history_.empty()) {
        os << "No completed tickets yet.\n";
        return;
    }

    std::vector<Ticket> sorted = history_;
    std::sort(sorted.begin(), sorted.end());

    for (const Ticket& ticket : sorted) {
        os << ticket << '\n';
    }
}

void ParkingLot::printSummary(std::ostream& os) const {
    const long long totalMinutes = std::accumulate(
        history_.begin(), history_.end(), 0LL,
        [](long long sum, const Ticket& ticket) {
            return sum + ticket.duration().count();
        });

    os << "Spots: " << spotCount() << " (occupied: " << occupiedSpotCount()
       << ", free: " << spotCount() - occupiedSpotCount() << ")\n";
    os << "Active vehicles: " << activeCount() << '\n';
    os << "Completed tickets: " << history_.size() << '\n';
    os << "Total revenue: " << formatMoney(totalRevenue()) << '\n';

    os << "Average stay: ";
    if (history_.empty()) {
        os << "n/a";
    } else {
        const auto averageMinutes =
            totalMinutes / static_cast<long long>(history_.size());
        os << formatDuration(std::chrono::minutes(averageMinutes));
    }
    os << '\n';

    os << "Current fee strategy: " << strategy_->name() << '\n';
}

std::size_t ParkingLot::spotCount() const noexcept {
    return spots_.size();
}

std::size_t ParkingLot::occupiedSpotCount() const noexcept {
    const auto occupied = std::count_if(
        spots_.begin(), spots_.end(),
        [](const ParkingSpot& spot) { return spot.isOccupied(); });
    return static_cast<std::size_t>(occupied);
}

std::size_t ParkingLot::activeCount() const noexcept {
    return active_.size();
}

double ParkingLot::totalRevenue() const noexcept {
    return revenue_;
}

const std::map<std::string, ParkingLot::ActiveParking>&
ParkingLot::activeParkings() const noexcept {
    return active_;
}

const std::vector<Ticket>& ParkingLot::history() const noexcept {
    return history_;
}

void ParkingLot::save(const std::string& path) const {
    const std::filesystem::path statePath(path);
    const std::filesystem::path parent = statePath.parent_path();

    if (!parent.empty()) {
        std::error_code error;
        std::filesystem::create_directories(parent, error);
        if (error) {
            throw FileIOException("Could not create state directory: " +
                                  error.message());
        }
    }

    std::ofstream output(path);
    if (!output) {
        throw FileIOException("Could not open state file for writing: " + path);
    }

    output << "PARKING_LOT " << kStateVersion << '\n';
    output << "SPOTS " << spots_.size() << '\n';
    for (const ParkingSpot& spot : spots_) {
        output << "SPOT " << spot.id() << ' ' << toString(spot.type()) << '\n';
    }

    output << "STRATEGY " << strategy_->key() << '\n';

    for (const auto& item : active_) {
        const ActiveParking& entry = item.second;
        output << "ACTIVE " << entry.ticket.id() << ' ' << item.first << ' '
               << entry.vehicle->typeName() << ' ' << entry.ticket.spotId()
               << ' ' << toTimeT(entry.ticket.entryTime()) << '\n';
    }

    output << std::fixed << std::setprecision(2);
    for (const Ticket& ticket : history_) {
        output << "HISTORY " << ticket.id() << ' ' << ticket.plate() << ' '
               << ticket.spotId() << ' ' << toTimeT(ticket.entryTime()) << ' '
               << toTimeT(ticket.exitTime()) << ' ' << ticket.fee() << '\n';
    }

    output << "REVENUE " << revenue_ << '\n';
    output << "END\n";

    if (!output) {
        throw FileIOException("Failed while writing state file: " + path);
    }
}

ParkingLot ParkingLot::load(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw FileIOException("Could not open state file: " + path);
    }

    std::string tag;
    int version = 0;

    readOrThrow(input, tag, "PARKING_LOT header");
    if (tag != "PARKING_LOT") {
        throw FileIOException("Not a parking lot state file: " + path);
    }

    readOrThrow(input, version, "state version");
    if (version != kStateVersion) {
        throw FileIOException("Unsupported state file version: " +
                              std::to_string(version));
    }

    readOrThrow(input, tag, "SPOTS section");
    if (tag != "SPOTS") {
        throw FileIOException("Expected SPOTS section.");
    }

    int storedSpotCount = 0;
    readOrThrow(input, storedSpotCount, "spot count");
    if (storedSpotCount <= 0) {
        throw FileIOException("State file must contain at least one spot.");
    }

    std::vector<ParkingSpot> spots;
    spots.reserve(static_cast<std::size_t>(storedSpotCount));

    for (int index = 0; index < storedSpotCount; ++index) {
        readOrThrow(input, tag, "SPOT entry");
        if (tag != "SPOT") {
            throw FileIOException("Expected SPOT entry.");
        }

        int id = 0;
        std::string spotTypeName;
        readOrThrow(input, id, "spot id");
        readOrThrow(input, spotTypeName, "spot type");

        SpotType spotType{};
        if (!spotTypeFromString(spotTypeName, spotType)) {
            throw FileIOException("Unknown spot type: " + spotTypeName);
        }

        spots.emplace_back(id, spotType);
    }

    readOrThrow(input, tag, "STRATEGY section");
    if (tag != "STRATEGY") {
        throw FileIOException("Expected STRATEGY section.");
    }

    std::string strategyKey;
    readOrThrow(input, strategyKey, "fee strategy");

    std::unique_ptr<FeeStrategy> strategy;
    try {
        strategy = makeFeeStrategy(strategyKey);
    } catch (const ParkingException& error) {
        throw FileIOException(std::string("Invalid fee strategy in state file: ") +
                              error.what());
    }

    ParkingLot lot(std::move(spots), std::move(strategy));

    int maxTicketId = 0;
    bool ended = false;

    while (input >> tag) {
        if (tag == "ACTIVE") {
            int id = 0;
            int spotId = 0;
            std::string plate;
            std::string vehicleTypeName;
            long long entryEpoch = 0;

            readOrThrow(input, id, "active ticket id");
            readOrThrow(input, plate, "active plate");
            readOrThrow(input, vehicleTypeName, "active vehicle type");
            readOrThrow(input, spotId, "active spot id");
            readOrThrow(input, entryEpoch, "active entry time");

            ParkingSpot* spot = lot.findSpotById(spotId);
            if (spot == nullptr) {
                throw FileIOException(
                    "Active ticket references an unknown spot id.");
            }

            auto vehicle =
                VehicleFactory::createFromName(vehicleTypeName, plate);
            if (!vehicle->canParkIn(spot->type())) {
                throw FileIOException(
                    "Active vehicle is not compatible with its saved spot.");
            }
            if (spot->isOccupied()) {
                throw FileIOException(
                    "State file assigns two vehicles to the same spot.");
            }

            const Ticket ticket(
                id, vehicle->plate(), spotId,
                fromTimeT(static_cast<std::time_t>(entryEpoch)));

            spot->occupy();
            auto inserted = lot.active_.emplace(
                vehicle->plate(), ActiveParking{std::move(vehicle), ticket});
            if (!inserted.second) {
                spot->release();
                throw FileIOException(
                    "State file contains a duplicate active plate: " + plate);
            }

            maxTicketId = std::max(maxTicketId, id);
        } else if (tag == "HISTORY") {
            int id = 0;
            int spotId = 0;
            std::string plate;
            long long entryEpoch = 0;
            long long exitEpoch = 0;
            double fee = 0.0;

            readOrThrow(input, id, "history ticket id");
            readOrThrow(input, plate, "history plate");
            readOrThrow(input, spotId, "history spot id");
            readOrThrow(input, entryEpoch, "history entry time");
            readOrThrow(input, exitEpoch, "history exit time");
            readOrThrow(input, fee, "history fee");

            Ticket ticket(id, plate, spotId,
                          fromTimeT(static_cast<std::time_t>(entryEpoch)));
            ticket.close(fromTimeT(static_cast<std::time_t>(exitEpoch)), fee);
            lot.history_.push_back(ticket);

            maxTicketId = std::max(maxTicketId, id);
        } else if (tag == "REVENUE") {
            readOrThrow(input, lot.revenue_, "revenue");
        } else if (tag == "END") {
            ended = true;
            break;
        } else {
            throw FileIOException("Unknown state file entry: " + tag);
        }
    }

    if (!ended) {
        throw FileIOException("State file is incomplete (missing END marker).");
    }
    if (lot.revenue_ < 0.0) {
        throw FileIOException("State file contains a negative revenue value.");
    }
    if (lot.occupiedSpotCount() != lot.active_.size()) {
        throw FileIOException("State file occupancy does not match its tickets.");
    }

    Ticket::setNextId(maxTicketId + 1);
    return lot;
}

ParkingLot ParkingLot::makeDefault(int compactSpots, int regularSpots,
                                   int largeSpots, int evSpots) {
    if (compactSpots < 0 || regularSpots < 0 || largeSpots < 0 ||
        evSpots < 0) {
        throw InvalidInputException("Spot counts cannot be negative.");
    }

    const int total = compactSpots + regularSpots + largeSpots + evSpots;
    if (total <= 0) {
        throw InvalidInputException("At least one parking spot is required.");
    }

    std::vector<ParkingSpot> spots;
    spots.reserve(static_cast<std::size_t>(total));

    int id = 1;
    const auto addSpots = [&spots, &id](int count, SpotType type) {
        for (int index = 0; index < count; ++index) {
            spots.emplace_back(id, type);
            ++id;
        }
    };

    addSpots(compactSpots, SpotType::Compact);
    addSpots(regularSpots, SpotType::Regular);
    addSpots(largeSpots, SpotType::Large);
    addSpots(evSpots, SpotType::EV);

    return ParkingLot(std::move(spots), std::make_unique<HourlyFeeStrategy>());
}

ParkingSpot* ParkingLot::findAvailableSpot(const Vehicle& vehicle) {
    const SpotType preferred = vehicle.preferredSpotType();
    const auto preferredSpot = std::find_if(
        spots_.begin(), spots_.end(),
        [&vehicle, preferred](const ParkingSpot& candidate) {
            return candidate.type() == preferred && candidate.canFit(vehicle);
        });

    if (preferredSpot != spots_.end()) {
        return &*preferredSpot;
    }

    const auto spot = std::find_if(
        spots_.begin(), spots_.end(),
        [&vehicle](const ParkingSpot& candidate) {
            return candidate.canFit(vehicle);
        });

    return (spot == spots_.end()) ? nullptr : &*spot;
}

ParkingSpot* ParkingLot::findSpotById(int id) {
    const auto spot = std::find_if(
        spots_.begin(), spots_.end(),
        [id](const ParkingSpot& candidate) { return candidate.id() == id; });

    return (spot == spots_.end()) ? nullptr : &*spot;
}

const ParkingSpot* ParkingLot::findSpotById(int id) const {
    const auto spot = std::find_if(
        spots_.begin(), spots_.end(),
        [id](const ParkingSpot& candidate) { return candidate.id() == id; });

    return (spot == spots_.end()) ? nullptr : &*spot;
}

std::string ParkingLot::plateAtSpot(int spotId) const {
    for (const auto& item : active_) {
        if (item.second.ticket.spotId() == spotId) {
            return item.first;
        }
    }

    return "-";
}
