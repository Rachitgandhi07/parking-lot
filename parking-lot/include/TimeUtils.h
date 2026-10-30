#pragma once

#include <chrono>
#include <ctime>
#include <string>

using Clock = std::chrono::system_clock;
using TimePoint = Clock::time_point;

std::string formatTime(TimePoint timePoint);
std::string formatDuration(std::chrono::minutes duration);
std::string formatMoney(double amount);

std::time_t toTimeT(TimePoint timePoint);
TimePoint fromTimeT(std::time_t value);
