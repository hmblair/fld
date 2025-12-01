#ifndef STEM_CONFIG_H
#define STEM_CONFIG_H

#include <cstddef>
#include <climits>
#include <stdexcept>
#include <string>

struct StemConfig {
    size_t min_length = 7;
    size_t max_length = 9;
    size_t max_au = INT_MAX;
    size_t max_gc = 5;
    size_t max_gu = 0;
    size_t closing_gc = 1;
    size_t spacer_length = 2;

    // Validation
    void validate() const;

    // Check if the config can accommodate a stem of given length
    bool can_accommodate(size_t length) const;

    // Get maximum possible stem length given the base pair constraints
    size_t max_possible_length() const;

    // Factory methods for common configurations
    static StemConfig for_padding();
    static StemConfig for_barcode(size_t stem_length);
};

#endif
