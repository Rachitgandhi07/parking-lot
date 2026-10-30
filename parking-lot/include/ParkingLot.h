#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "FeeStrategy.h"
#include "ParkingSpot.h"
#include "Ticket.h"
#include "Vehicle.h"

class ParkingLot {
public:
    struct ActiveParking {
        std::unique_ptr<Vehicle> vehicle;
        Ticket ticket;
    };

    explicit ParkingLot(
        std::vector<ParkingSpot> spots,
        std::unique_ptr<FeeStrategy> strategy =
            std::make_unique<HourlyFeeStrategy>());

    ~ParkingLot() = default;

    ParkingLot(const ParkingLot&) = delete;
    ParkingLot& operator=(const ParkingLot&) = delete;
    ParkingLot(ParkingLot&&) = default;
    ParkingLot& operator=(ParkingLot&&) = default;

    Ticket park(std::unique_ptr<Vehicle> vehicle);
    Ticket unpark(const std::string& plate);

    void setFeeStrategy(std::unique_ptr<FeeStrategy> strategy);
    const FeeStrategy& feeStrategy() const noexcept;

    void printStatus(std::ostream& os) const;
    void printActiveTickets(std::ostream& os) const;
    void printHistory(std::ostream& os) const;
    void printSummary(std::ostream& os) const;

    std::size_t spotCount() const noexcept;
    std::size_t occupiedSpotCount() const noexcept;
    std::size_t activeCount() const noexcept;
    double totalRevenue() const noexcept;

    const std::map<std::string, ActiveParking>& activeParkings() const noexcept;
    const std::vector<Ticket>& history() const noexcept;

    void save(const std::string& path) const;
    static ParkingLot load(const std::string& path);
    static ParkingLot makeDefault(int compactSpots = 4, int regularSpots = 4,
                                  int largeSpots = 2, int evSpots = 2);

private:
    ParkingSpot* findAvailableSpot(const Vehicle& vehicle);
    ParkingSpot* findSpotById(int id);
    const ParkingSpot* findSpotById(int id) const;
    std::string plateAtSpot(int spotId) const;

    std::vector<ParkingSpot> spots_;
    std::map<std::string, ActiveParking> active_;
    std::vector<Ticket> history_;
    std::unique_ptr<FeeStrategy> strategy_;
    double revenue_ = 0.0;
};
