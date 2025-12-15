#ifndef NUC_H
#define NUC_H

#include <string>
#include <random>
#include <unordered_set>

#include "config/stem_config.hpp"

#define _A 'A'
#define _C 'C'
#define _G 'G'
#define _T 'T'
#define _U 'U'

enum class Side {
    fivep,
    threep
};

char _complement(const char& base);

std::string _to_dna(const std::string& sequence);
std::string _to_rna(const std::string& sequence);
std::string _replace_polybases(
    const std::string& sequence,
    std::mt19937& gen
);

std::string _random_sequence(
    size_t length,
    std::mt19937& gen
);

std::string _random_hairpin(
    size_t stem_length,
    const StemConfig& config,
    std::mt19937& gen
);

std::string _get_padding(
    size_t length,
    const StemConfig& config,
    std::mt19937& gen
);

#endif
