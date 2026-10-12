#pragma once

#include <chrono>

constexpr int kRepeats = 5; 

struct Measurement {
    double timeMs;
    double result;
};


template <class Run>
Measurement measure(Run&& run) {
    using Clock = std::chrono::steady_clock;

    double result = run();
    double totalMs = 0.0;
    for (int i = 0; i < kRepeats; ++i) {
        const auto start = Clock::now();
        result = run();
        const auto finish = Clock::now();
        totalMs += std::chrono::duration<double, std::milli>(finish - start).count();
    }
    return {totalMs / kRepeats, result};
}