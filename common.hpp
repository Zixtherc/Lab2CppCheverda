#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

using Sequence = std::vector<double>;

inline const std::array<std::size_t, 4> kSizes = {10'000, 100'000, 1'000'000, 5'000'000};

inline std::string dataFileName(std::size_t size) {
    return "data_" + std::to_string(size) + ".txt";
}

inline Sequence readSequence(const std::string& fileName) {
    std::ifstream in(fileName);
    std::size_t size = 0;
    in >> size;
    Sequence data(size);
    for (double& x : data) {
        in >> x;
    }
    return data;
}

struct FastTransform {
    double operator()(double x) const { return x * x; }
};

struct SlowTransform {
    double operator()(double x) const { return std::sqrt(x * x + 1.0) * std::cos(x); }
};