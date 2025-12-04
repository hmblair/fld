/**
 * @file stem_config.hpp
 * @brief Configuration for hairpin stem generation.
 *
 * Hairpin stems are used in both padding and barcodes. They consist of
 * complementary base pairs that form a double-stranded region, capped
 * by a tetraloop. The base pair composition affects the stability and
 * experimental readout of the structure.
 *
 * Base pair types:
 * - AU/UA: Weak pairing (adenine-uracil)
 * - GC/CG: Strong pairing (guanine-cytosine)
 * - GU/UG: Wobble pairing (guanine-uracil)
 *
 * Typical constraints:
 * - max_gc limits strong pairs to prevent experimental artifacts
 * - max_gu limits wobble pairs which can cause structural heterogeneity
 * - closing_gc ensures stable stem closure (typically 1 GC pair)
 */

#ifndef STEM_CONFIG_H
#define STEM_CONFIG_H

#include <cstddef>
#include <climits>
#include <stdexcept>
#include <string>

/**
 * Configuration for hairpin stem generation.
 *
 * Controls the composition and length of hairpin stems used in
 * padding and barcoding. The constraints ensure stems have
 * appropriate stability and experimental properties.
 */
struct StemConfig {
    /// Minimum stem length in base pairs (default: 7)
    size_t min_length = 7;

    /// Maximum stem length in base pairs (default: 9)
    size_t max_length = 9;

    /// Maximum AU base pairs allowed (default: unlimited)
    size_t max_au = INT_MAX;

    /// Maximum GC base pairs allowed (default: 5)
    size_t max_gc = 5;

    /// Maximum GU wobble pairs allowed (default: 0)
    size_t max_gu = 0;

    /// Number of GC pairs at stem ends for stability (default: 1)
    size_t closing_gc = 1;

    /// Length of polyA spacer between consecutive stems (default: 2)
    size_t spacer_length = 2;

    /// Validate that the configuration is internally consistent.
    void validate() const;

    /// Check if the config can accommodate a stem of given length.
    bool can_accommodate(size_t length) const;

    /// Get maximum possible stem length given the base pair constraints.
    size_t max_possible_length() const;

    /// Create configuration for padding stems.
    static StemConfig for_padding();

    /// Create configuration for barcode stems with specified length.
    static StemConfig for_barcode(size_t stem_length);
};

#endif
