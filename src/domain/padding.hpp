#ifndef PADDING_DOMAIN_H
#define PADDING_DOMAIN_H

#include <string>
#include <random>
#include "../config/stem_config.hpp"

// Generates a padding sequence of exactly the given length, built from
// hairpins with poly-A spacers where space allows, and disordered random
// sequence otherwise.
std::string get_padding(
    size_t length,
    const StemConfig& config,
    std::mt19937& gen
);

#endif
