#pragma once

#include <cstddef>
#include <functional>
#include <numeric>
#include <thread>
#include <vector>

#include "common.hpp"

template <class Transform>
double parallelTransformReduce(const Sequence& data, Transform transform, std::size_t k) {
    const std::size_t n = data.size();
    std::vector<double> partialSums(k, 0.0);
    std::vector<std::thread> threads;
    threads.reserve(k);

    for (std::size_t i = 0; i < k; ++i) {
        const std::size_t begin = n * i / k;
        const std::size_t end = n * (i + 1) / k;

        threads.emplace_back([&, i, begin, end] {
            partialSums[i] = std::transform_reduce(data.begin() + begin, data.begin() + end, 0.0,
                                                   std::plus<>{}, transform);
        });
    }

    for (std::thread& t : threads) {
        t.join();
    }

    return std::reduce(partialSums.begin(), partialSums.end());
}