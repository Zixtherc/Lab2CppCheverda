// Варіант 7: transform_reduce.
//
// Компілятор (вкажіть свій!):
//   MSVC (Visual Studio 2022):
//     без оптимізації : cl /EHsc /std:c++20 /utf-8 /Od main.cpp /Fe:exp_O0.exe
//     з оптимізацією  : cl /EHsc /std:c++20 /utf-8 /O2 main.cpp /Fe:exp_O2.exe
//   GCC / Clang (потрібна бібліотека TBB для паралельних політик):
//     без оптимізації : g++ -std=c++20 -O0 main.cpp -o exp_O0 -ltbb
//     з оптимізацією  : g++ -std=c++20 -O3 main.cpp -o exp_O3 -ltbb
//
// Запуск (перший аргумент - лише підпис рівня оптимізації для виводу):
//   exp_O0 O0 > out_O0.txt
//   exp_O2 O2 > out_O2.txt
//
// Перед запуском потрібні файли data_<N>.txt (створює generate_data.cpp).

#include <algorithm>
#include <execution>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

#include "common.hpp"
#include "parallel_transform_reduce.hpp"
#include "timer.hpp"

namespace {

struct KMeasurement {
    std::size_t k;
    Measurement measurement;
};

void printSection(const std::string& title) {
    std::cout << "\n--- " << title << " ---\n";
}

void printRow(const std::string& label, const Measurement& m) {
    std::cout << std::left << std::setw(24) << label << std::right << std::setw(12) << m.timeMs
              << " ms    result = " << m.result << '\n';
}


template <class Transform>
Measurement measureNoPolicy(const Sequence& data, Transform transform) {
    return measure([&] {
        return std::transform_reduce(data.begin(), data.end(), 0.0, std::plus<>{}, transform);
    });
}


template <class Policy, class Transform>
Measurement measureWithPolicy(Policy&& policy, const Sequence& data, Transform transform) {
    return measure([&] {
        return std::transform_reduce(policy, data.begin(), data.end(), 0.0, std::plus<>{},
                                     transform);
    });
}

template <class Transform>
void runLibraryExperiments(const Sequence& data, Transform transform) {
    printSection("Бібліотечний transform_reduce: без політики та з політиками");
    printRow("no policy", measureNoPolicy(data, transform));
    printRow("execution::seq", measureWithPolicy(std::execution::seq, data, transform));
    printRow("execution::par", measureWithPolicy(std::execution::par, data, transform));
    printRow("execution::par_unseq", measureWithPolicy(std::execution::par_unseq, data, transform));
}


std::vector<std::size_t> makeThreadCounts() {
    std::vector<std::size_t> counts;
    for (std::size_t k = 1; k <= 1024; k *= 2) {
        counts.push_back(k);
    }
    counts.push_back(std::thread::hardware_concurrency());
    std::sort(counts.begin(), counts.end());
    counts.erase(std::unique(counts.begin(), counts.end()), counts.end());
    return counts;
}

void printTable(const std::vector<KMeasurement>& rows) {
    const double baselineMs = rows.front().measurement.timeMs;  // K = 1

    std::cout << std::setw(6) << "K" << std::setw(14) << "time, ms" << std::setw(12) << "speedup"
              << std::setw(20) << "result" << '\n';
    for (const KMeasurement& row : rows) {
        std::cout << std::setw(6) << row.k << std::setw(14) << row.measurement.timeMs
                  << std::setw(12) << baselineMs / row.measurement.timeMs << std::setw(20)
                  << row.measurement.result << '\n';
    }
}

void printBestK(const std::vector<KMeasurement>& rows) {
    const auto best = std::min_element(rows.begin(), rows.end(), [](const auto& a, const auto& b) {
        return a.measurement.timeMs < b.measurement.timeMs;
    });
    const unsigned hardwareThreads = std::thread::hardware_concurrency();

    std::cout << "\nНайкраще K = " << best->k << " (час " << best->measurement.timeMs << " ms)\n"
              << "Кількість потоків процесора = " << hardwareThreads << '\n'
              << "Відношення K / кількість потоків = "
              << static_cast<double>(best->k) / hardwareThreads << '\n';
}

template <class Transform>
void runOwnExperiments(const Sequence& data, Transform transform) {
    printSection("Власний паралельний transform_reduce: залежність часу від K");

    std::vector<KMeasurement> rows;
    for (std::size_t k : makeThreadCounts()) {
        rows.push_back({k, measure([&] { return parallelTransformReduce(data, transform, k); })});
    }
    printTable(rows);
    printBestK(rows);
}


template <class Transform>
void runForTransform(const std::string& transformName, const Sequence& data, Transform transform) {
    std::cout << "\n=============== N = " << data.size() << ", перетворення: " << transformName
              << " ===============\n";
    runLibraryExperiments(data, transform);
    runOwnExperiments(data, transform);
}

void printGeneralInfo(const std::string& optimization) {
    std::cout << "Варіант 7: transform_reduce (сума f(x) по всіх елементах)\n"
              << "Рівень оптимізації: " << optimization << '\n'
              << "Потоків процесора (hardware_concurrency): " << std::thread::hardware_concurrency()
              << '\n'
              << "Кожен час - середнє з " << kRepeats << " запусків (плюс 1 прогрівальний), мс\n";
}

}

int main(int argc, char* argv[]) {
    const std::string optimization = argc > 1 ? argv[1] : "не вказано";

    std::cout << std::fixed << std::setprecision(3);
    printGeneralInfo(optimization);

    for (std::size_t size : kSizes) {
        const Sequence data = readSequence(dataFileName(size));
        runForTransform("швидке x*x", data, FastTransform{});
        runForTransform("повільніше sqrt(x*x+1)*cos(x)", data, SlowTransform{});
    }
}