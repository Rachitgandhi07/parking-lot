#include <exception>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include "FeeStrategy.h"
#include "ParkingLot.h"
#include "TimeUtils.h"
#include "VehicleFactory.h"

namespace {

const std::string kStatePath = "data/parking_state.txt";

class InputClosed : public std::exception {
public:
    const char* what() const noexcept override {
        return "Input closed";
    }
};

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt << std::flush;

    std::string line;
    if (!std::getline(std::cin, line)) {
        throw InputClosed{};
    }

    return trim(line);
}

int readInt(const std::string& prompt) {
    while (true) {
        const std::string text = readLine(prompt);
        if (text.empty()) {
            std::cout << "Please enter a number.\n";
            continue;
        }

        std::istringstream input(text);
        int value = 0;
        char extra = '\0';
        if ((input >> value) && !(input >> extra)) {
            return value;
        }

        std::cout << "Please enter a valid whole number.\n";
    }
}

int readIntInRange(const std::string& prompt, int minimum, int maximum) {
    while (true) {
        const int value = readInt(prompt);
        if (value >= minimum && value <= maximum) {
            return value;
        }

        std::cout << "Enter a number between " << minimum << " and " << maximum
                  << ".\n";
    }
}

std::string readNonEmptyLine(const std::string& prompt) {
    while (true) {
        const std::string value = readLine(prompt);
        if (!value.empty()) {
            return value;
        }

        std::cout << "This field cannot be empty.\n";
    }
}

void saveState(const ParkingLot& lot, bool announce) {
    try {
        lot.save(kStatePath);
        if (announce) {
            std::cout << "State saved to " << kStatePath << ".\n";
        }
    } catch (const std::exception& error) {
        std::cout << "Warning: could not save state: " << error.what() << '\n';
    }
}

void reloadState(ParkingLot& lot) {
    if (!std::filesystem::exists(kStatePath)) {
        std::cout << "No saved state found at " << kStatePath << ".\n";
        return;
    }

    ParkingLot loaded = ParkingLot::load(kStatePath);
    lot = std::move(loaded);
    std::cout << "Reloaded state from " << kStatePath << ".\n";
}

void printMenu(const ParkingLot& lot) {
    const std::size_t freeSpots = lot.spotCount() - lot.occupiedSpotCount();

    std::cout << "\n========== PARKING LOT ==========\n"
              << "Spots: " << lot.spotCount()
              << " | Occupied: " << lot.occupiedSpotCount()
              << " | Free: " << freeSpots << '\n'
              << "Strategy: " << lot.feeStrategy().name() << '\n'
              << "Revenue: " << formatMoney(lot.totalRevenue()) << '\n'
              << "---------------------------------\n"
              << "1) Park a vehicle\n"
              << "2) Unpark a vehicle\n"
              << "3) Show lot status\n"
              << "4) Show active tickets\n"
              << "5) Show completed tickets\n"
              << "6) Show summary\n"
              << "7) Change fee strategy\n"
              << "8) Save state now\n"
              << "9) Reload saved state\n"
              << "0) Exit\n";
}

void parkVehicle(ParkingLot& lot) {
    std::cout << "\nVehicle types:\n"
              << "  1) Bike         - 10.00/hour - compact or regular\n"
              << "  2) Car          - 20.00/hour - regular or large\n"
              << "  3) Truck        - 40.00/hour - large\n"
              << "  4) Electric car - 25.00/hour - EV or regular\n";

    const int choice = readIntInRange("Choose type (1-4): ", 1, 4);
    const VehicleType type = static_cast<VehicleType>(choice - 1);
    const std::string plate = readNonEmptyLine("Enter plate number: ");

    auto vehicle = VehicleFactory::create(type, plate);
    const std::string typeName = vehicle->typeName();
    const Ticket ticket = lot.park(std::move(vehicle));

    std::cout << "\nParked " << typeName << " at spot " << ticket.spotId()
              << ".\n"
              << ticket << '\n';
    saveState(lot, false);
}

void unparkVehicle(ParkingLot& lot) {
    const std::string plate = readNonEmptyLine("Enter plate number: ");
    const Ticket ticket = lot.unpark(plate);

    std::cout << "\nVehicle unparked.\n"
              << ticket << "\nTotal charged: " << formatMoney(ticket.fee())
              << '\n';
    saveState(lot, false);
}

void changeFeeStrategy(ParkingLot& lot) {
    std::cout << "\nFee strategies:\n"
              << "  1) Hourly (minimum 1 hour, 8-hour cap)\n"
              << "  2) Flat day pass (120.00)\n";

    const int choice = readIntInRange("Choose strategy (1-2): ", 1, 2);
    lot.setFeeStrategy(
        makeFeeStrategy(choice == 1 ? "hourly" : "daypass"));

    std::cout << "Fee strategy changed to " << lot.feeStrategy().name()
              << ".\n";
    saveState(lot, false);
}

void handleChoice(int choice, ParkingLot& lot) {
    switch (choice) {
        case 1:
            parkVehicle(lot);
            break;
        case 2:
            unparkVehicle(lot);
            break;
        case 3:
            lot.printStatus(std::cout);
            break;
        case 4:
            lot.printActiveTickets(std::cout);
            break;
        case 5:
            lot.printHistory(std::cout);
            break;
        case 6:
            lot.printSummary(std::cout);
            break;
        case 7:
            changeFeeStrategy(lot);
            break;
        case 8:
            saveState(lot, true);
            break;
        case 9:
            reloadState(lot);
            break;
        default:
            std::cout << "Unknown option.\n";
            break;
    }
}

}  // namespace

int main() {
    std::cout << "=== Parking Lot Management System ===\n";

    ParkingLot lot = ParkingLot::makeDefault();

    try {
        if (std::filesystem::exists(kStatePath)) {
            lot = ParkingLot::load(kStatePath);
            std::cout << "Loaded saved state from " << kStatePath << ".\n";
        } else {
            std::cout << "No saved state found. Starting with a fresh "
                         "12-spot lot.\n";
        }
    } catch (const std::exception& error) {
        std::cout << "Could not load state: " << error.what() << "\n"
                  << "Starting with a fresh 12-spot lot.\n";
    }

    while (true) {
        printMenu(lot);

        try {
            const int choice =
                readIntInRange("Choose an option (0-9): ", 0, 9);

            if (choice == 0) {
                saveState(lot, false);
                std::cout << "\nState saved. Goodbye.\n";
                break;
            }

            handleChoice(choice, lot);
        } catch (const InputClosed&) {
            std::cout << "\nInput closed. Saving and exiting.\n";
            saveState(lot, false);
            break;
        } catch (const ParkingException& error) {
            std::cout << "\nError: " << error.what() << '\n';
        } catch (const std::exception& error) {
            std::cout << "\nUnexpected error: " << error.what() << '\n';
        }
    }

    return 0;
}
