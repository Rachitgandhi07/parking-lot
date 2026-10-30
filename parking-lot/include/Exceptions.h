#pragma once

#include <stdexcept>
#include <string>

class ParkingException : public std::runtime_error {
public:
    explicit ParkingException(const std::string& message)
        : std::runtime_error(message) {}
};

class ParkingFullException : public ParkingException {
public:
    explicit ParkingFullException(const std::string& message)
        : ParkingException(message) {}
};

class VehicleNotFoundException : public ParkingException {
public:
    explicit VehicleNotFoundException(const std::string& message)
        : ParkingException(message) {}
};

class DuplicateVehicleException : public ParkingException {
public:
    explicit DuplicateVehicleException(const std::string& message)
        : ParkingException(message) {}
};

class InvalidOperationException : public ParkingException {
public:
    explicit InvalidOperationException(const std::string& message)
        : ParkingException(message) {}
};

class InvalidInputException : public ParkingException {
public:
    explicit InvalidInputException(const std::string& message)
        : ParkingException(message) {}
};

class FileIOException : public ParkingException {
public:
    explicit FileIOException(const std::string& message)
        : ParkingException(message) {}
};
