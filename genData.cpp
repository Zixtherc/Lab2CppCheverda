#include <iomanip>
#include <random>

#include "common.hpp"

void writeSequence(std::size_t size, std::mt19937& generator) {
    std::uniform_real_distribution<double> distribution(-1.0, 1.0);

    std::ofstream out(dataFileName(size));
    out << size << '\n' << std::fixed << std::setprecision(4);
    for (std::size_t i = 0; i < size; ++i) {
        out << distribution(generator) << ' ';
    }
    out << '\n';
}

int main() {
    std::mt19937 generator(12345);
    for (std::size_t size : kSizes) {
        writeSequence(size, generator);
    }
}