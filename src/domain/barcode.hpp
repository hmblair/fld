#ifndef BARCODE_DOMAIN_H
#define BARCODE_DOMAIN_H

#include <string>
#include <unordered_set>
#include <random>
#include "../config/stem_config.hpp"

// A barcode is a hairpin sequence used to uniquely identify constructs.
// Barcodes must maintain minimum Hamming distance from each other to
// allow error-tolerant identification.
class Barcode {
public:
    Barcode() = default;
    explicit Barcode(std::string sequence);

    // Get the sequence string
    const std::string& str() const { return _sequence; }
    size_t length() const { return _sequence.length(); }
    bool empty() const { return _sequence.empty(); }

    // Check if this barcode is within Hamming distance 1 of any in the set
    bool has_hamming_neighbor(const std::unordered_set<std::string>& existing) const;

    // Generate a random barcode that is not a Hamming neighbor of any existing
    static Barcode random(
        size_t stem_length,
        const StemConfig& config,
        std::mt19937& gen,
        const std::unordered_set<std::string>& existing
    );

    // String conversion
    operator const std::string&() const { return _sequence; }

    // Generate the Hamming ball (all sequences at distance 1)
    static std::vector<std::string> hamming_ball(const std::string& seq);

private:
    std::string _sequence;

    // Check if a given sequence has a Hamming neighbor in the set
    static bool is_hamming_neighbor(
        const std::string& seq,
        const std::unordered_set<std::string>& existing
    );
};

#endif
