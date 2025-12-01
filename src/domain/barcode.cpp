#include "barcode.hpp"
#include "hairpin.hpp"
#include "sequence.hpp"
#include <stdexcept>

// The pairing complement for Hamming ball calculation
// Only mutations that preserve base-pairing are considered:
// A <-> G, C <-> T
static char pairing_complement(char base) {
    switch (base) {
        case BASE_A: return BASE_G;
        case BASE_C: return BASE_T;
        case BASE_G: return BASE_A;
        case BASE_T: return BASE_C;
        default:
            throw std::runtime_error("Invalid base \"" + std::string{base} + "\".");
    }
}

Barcode::Barcode(std::string sequence) : _sequence(std::move(sequence)) {}

std::vector<std::string> Barcode::hamming_ball(const std::string& seq) {
    std::vector<std::string> results;
    results.reserve(seq.length() + 1);

    for (size_t ix = 0; ix < seq.length(); ix++) {
        std::string variant = seq;
        variant[ix] = pairing_complement(seq[ix]);
        results.push_back(std::move(variant));
    }

    results.push_back(seq);  // Include the original
    return results;
}

bool Barcode::is_hamming_neighbor(
    const std::string& seq,
    const std::unordered_set<std::string>& existing
) {
    std::vector<std::string> ball = hamming_ball(seq);
    for (const auto& element : ball) {
        if (existing.find(element) != existing.end()) {
            return true;
        }
    }
    return false;
}

bool Barcode::has_hamming_neighbor(const std::unordered_set<std::string>& existing) const {
    return is_hamming_neighbor(_sequence, existing);
}

Barcode Barcode::random(
    size_t stem_length,
    const StemConfig& config,
    std::mt19937& gen,
    const std::unordered_set<std::string>& existing
) {
    std::string barcode;
    do {
        Hairpin hp = Hairpin::random(stem_length, config, gen);
        barcode = hp.str();
    } while (is_hamming_neighbor(barcode, existing));

    return Barcode(std::move(barcode));
}
