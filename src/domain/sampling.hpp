#ifndef SAMPLING_H
#define SAMPLING_H

#include <random>
#include <vector>

// Samples a uniform integer from [low, high].
inline size_t sample_from_range(size_t low, size_t high, std::mt19937& gen) {
    std::uniform_int_distribution<size_t> dist(low, high);
    return dist(gen);
}

// Samples a uniform element from a vector.
template <typename T>
T sample_from_vector(const std::vector<T>& values, std::mt19937& gen) {
    size_t ix = sample_from_range(0, values.size() - 1, gen);
    return values[ix];
}

#endif
