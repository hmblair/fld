#ifndef HAIRPIN_H
#define HAIRPIN_H

#include <string>
#include <vector>
#include <random>
#include "../config/stem_config.hpp"

// Represents a base pair type
enum class BasePairType {
    AU,  // A-T or T-A (using DNA nomenclature)
    GC,  // G-C or C-G
    GU,  // G-U wobble pair
    None
};

// A single base pair in a stem
struct BasePair {
    char five_prime;   // 5' base
    char three_prime;  // 3' base

    BasePair() = default;
    BasePair(char fp, char tp) : five_prime(fp), three_prime(tp) {}

    BasePairType type() const;

    static bool is_au(char b1, char b2);
    static bool is_gc(char b1, char b2);
    static bool is_gu(char b1, char b2);
};

// A hairpin structure with stem and tetraloop
class Hairpin {
public:
    Hairpin() = default;

    // Create a random hairpin with the given stem length and configuration
    static Hairpin random(
        size_t stem_length,
        const StemConfig& config,
        std::mt19937& gen
    );

    // Get the string representation (5' -> stem -> loop -> antisense stem -> 3')
    std::string str() const;

    // Get the stem length
    size_t stem_length() const { return _pairs.size(); }

    // Get the tetraloop sequence
    const std::string& loop() const { return _loop; }

private:
    std::vector<BasePair> _pairs;
    std::string _loop;

    Hairpin(std::vector<BasePair> pairs, std::string loop);
};

#endif
