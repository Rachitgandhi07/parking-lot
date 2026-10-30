#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "Exceptions.h"
#include "FeeStrategy.h"
#include "ParkingLot.h"
#include "Ticket.h"
#include "VehicleFactory.h"

namespace {

int checks = 0;

[[noreturn]] void fail(const std::string& message, const char* expression,
                       const char* file, int line) {
    std::cerr << file << ':' << line << ": check failed: " << expression
              << "\n  " << message << '\n';
    std::exit(1);
}

void check(bool condition, const char* expression, const char* file,
           int line) {
    ++checks;
    if (!condition) {
        fail("condition was false", expression, file, line);
    }
}

void checkNear(double actual, double expected, double tolerance,
               const char* expression, const char* file, int line) {
    ++checks;
    if (std::fabs(actual - expected) > tolerance) {
        fail("expected " + std::to_string(expected) + ", got " +
                 std::to_string(actual),
             expression, file, line);
    }
}

template <typename ExceptionType, typename Callable>
void checkThrows(Callable&& callable, const char* exceptionName,
                 const char* expression, const char* file, int line) {
    ++checks;
    try {
        callable();
    } catch (const ExceptionType&) {
        return;
    } catch (const std::exception& error) {
        fail(std::string("expected ") + exceptionName + ", caught: " +
                 error.what(),
             expression, file, line);
    }

    fail(std::string("expected ") + exceptionName +
             ", but no exception was thrown",
         expression, file, line);
}

}  // namespace

#define CHECK(condition) check((condition), #condition, __FILE__, __LINE__)

#define CHECK_NEAR(actual, expected, tolerance) \
    checkNear((actual), (expected), (tolerance), #actual, __FILE__, __LINE__)

#define CHECK_THROWS(exception_type, ...)                            \
    checkThrows<exception_type>([&] { __VA_ARGS__; }, #exception_type, \
                                #__VA_ARGS__, __FILE__, __LINE__)

void testParkingRules() {
    Ticket::setNextId(1);
    ParkingLot lot = ParkingLot::makeDefault(1, 1, 1, 1);

    const Ticket carOne =
        lot.park(VehicleFactory::create(VehicleType::Car, "CAR-1"));
    CHECK(carOne.spotId() == 2);
    CHECK(carOne.plate() == "CAR1");

    const Ticket carTwo =
        lot.park(VehicleFactory::create(VehicleType::Car, "CAR-2"));
    CHECK(carTwo.spotId() == 3);
    CHECK(lot.occupiedSpotCount() == 2);

    CHECK_THROWS(DuplicateVehicleException,
                 lot.park(VehicleFactory::create(VehicleType::Car, "CAR-1")));
    CHECK_THROWS(ParkingFullException,
                 lot.park(VehicleFactory::create(VehicleType::Car, "CAR-3")));
    CHECK_THROWS(VehicleNotFoundException, lot.unpark("GHOST-1"));
    CHECK_THROWS(InvalidInputException, lot.park(nullptr));
    CHECK_THROWS(InvalidInputException,
                 lot.park(VehicleFactory::create(VehicleType::Bike, "AB")));

    const Ticket bike =
        lot.park(VehicleFactory::create(VehicleType::Bike, "BIKE-1"));
    CHECK(bike.spotId() == 1);

    CHECK_THROWS(ParkingFullException,
                 lot.park(VehicleFactory::create(VehicleType::Truck, "TRUCK-1")));

    const Ticket electric =
        lot.park(VehicleFactory::create(VehicleType::ElectricCar, "EV-1"));
    CHECK(electric.spotId() == 4);
    CHECK(lot.occupiedSpotCount() == 4);

    const Ticket closed = lot.unpark("CAR-1");
    CHECK_NEAR(closed.fee(), 20.0, 1e-9);
    CHECK(lot.activeCount() == 3);
    CHECK(lot.history().size() == 1);
    CHECK_NEAR(lot.totalRevenue(), 20.0, 1e-9);

    std::cout << "testParkingRules passed\n";
}

void testSpotCompatibility() {
    Ticket::setNextId(1);
    ParkingLot lot = ParkingLot::makeDefault(1, 1, 1, 1);

    const Ticket bike =
        lot.park(VehicleFactory::create(VehicleType::Bike, "BIKE-1"));
    const Ticket car =
        lot.park(VehicleFactory::create(VehicleType::Car, "CAR-1"));
    const Ticket truck =
        lot.park(VehicleFactory::create(VehicleType::Truck, "TRUCK-1"));
    const Ticket electric =
        lot.park(VehicleFactory::create(VehicleType::ElectricCar, "EV-1"));

    CHECK(bike.spotId() == 1);
    CHECK(car.spotId() == 2);
    CHECK(truck.spotId() == 3);
    CHECK(electric.spotId() == 4);
    CHECK(lot.occupiedSpotCount() == 4);
    CHECK_THROWS(ParkingFullException,
                 lot.park(VehicleFactory::create(VehicleType::Bike, "BIKE-2")));

    std::cout << "testSpotCompatibility passed\n";
}

void testFeeStrategies() {
    Ticket::setNextId(1);
    ParkingLot lot = ParkingLot::makeDefault(1, 1, 1, 1);

    lot.setFeeStrategy(makeFeeStrategy("daypass"));
    CHECK(lot.feeStrategy().key() == "daypass");

    lot.park(VehicleFactory::create(VehicleType::Car, "DAY-1"));
    const Ticket dayPassTicket = lot.unpark("DAY-1");
    CHECK_NEAR(dayPassTicket.fee(), 120.0, 1e-9);

    lot.setFeeStrategy(makeFeeStrategy("hourly"));
    CHECK(lot.feeStrategy().key() == "hourly");

    lot.park(VehicleFactory::create(VehicleType::Truck, "TRUCK-1"));
    const Ticket hourlyTicket = lot.unpark("TRUCK-1");
    CHECK_NEAR(hourlyTicket.fee(), 40.0, 1e-9);

    CHECK_THROWS(InvalidInputException, makeFeeStrategy("bogus"));

    std::cout << "testFeeStrategies passed\n";
}

void testSaveLoad() {
    Ticket::setNextId(1);
    const std::string path = "data/test_state.txt";

    {
        ParkingLot lot = ParkingLot::makeDefault(2, 2, 2, 2);

        lot.park(VehicleFactory::create(VehicleType::Car, "SAVE-1"));
        lot.park(VehicleFactory::create(VehicleType::Bike, "SAVE-2"));
        lot.park(VehicleFactory::create(VehicleType::Car, "SAVE-3"));
        lot.unpark("SAVE-2");

        lot.setFeeStrategy(makeFeeStrategy("daypass"));
        lot.save(path);
    }

    ParkingLot loaded = ParkingLot::load(path);
    CHECK(loaded.spotCount() == 8);
    CHECK(loaded.activeCount() == 2);
    CHECK(loaded.history().size() == 1);
    CHECK(loaded.occupiedSpotCount() == 2);
    CHECK(loaded.feeStrategy().key() == "daypass");
    CHECK_NEAR(loaded.totalRevenue(), 10.0, 1e-9);
    CHECK(loaded.activeParkings().count("SAVE1") == 1);
    CHECK(loaded.activeParkings().at("SAVE1").ticket.spotId() == 3);
    CHECK(loaded.history().front().plate() == "SAVE2");
    CHECK_NEAR(loaded.history().front().fee(), 10.0, 1e-9);

    const Ticket closed = loaded.unpark("SAVE-1");
    CHECK_NEAR(closed.fee(), 120.0, 1e-9);
    CHECK_NEAR(loaded.totalRevenue(), 130.0, 1e-9);

    const Ticket newBike =
        loaded.park(VehicleFactory::create(VehicleType::Bike, "SAVE-4"));
    CHECK(newBike.id() > 3);
    CHECK(loaded.occupiedSpotCount() == 2);

    std::filesystem::remove(path);
    std::cout << "testSaveLoad passed\n";
}

void testFileErrors() {
    CHECK_THROWS(FileIOException,
                 ParkingLot::load("data/does_not_exist.txt"));
    std::cout << "testFileErrors passed\n";
}

int main() {
    testParkingRules();
    testSpotCompatibility();
    testFeeStrategies();
    testSaveLoad();
    testFileErrors();

    std::cout << "All " << checks << " checks passed.\n";
    return 0;
}
