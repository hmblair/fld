#ifndef NUC_H
#define NUC_H

#include <stdexcept>
#include <string>
#include <vector>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include <algorithm>
#include <iostream>

#define _A 'A'
#define _C 'C'
#define _G 'G'
#define _T 'T'
#define _U 'U'

enum class Side {
    fivep,
    threep
};

std::string _to_dna(
    const std::string& sequence,
    std::mt19937& gen
);

std::string _to_rna(const std::string& sequence);

std::string _random_sequence(
    size_t length,
    std::mt19937& gen
);

std::string _random_hairpin(
    size_t length,
    std::mt19937& gen,
    size_t max_stem_au,
    size_t max_stem_gc,
    size_t max_stem_gu,
    size_t closing_gc
);

std::string _get_padding(
    size_t length,
    size_t min_stem_length,
    size_t max_stem_length,
    size_t spacer_length,
    size_t max_stem_au,
    size_t max_stem_gc,
    size_t max_stem_gu,
    size_t closing_gc,
    std::mt19937& gen
);

bool _is_hamming_neighbour(
    const std::string& sequence,
    const std::unordered_set<std::string>& set
);

#endif
