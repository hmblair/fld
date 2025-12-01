#include "barcode_config.hpp"
#include <algorithm>
#include <vector>

static inline size_t _stem_counts(size_t length, const StemConfig& config) {
    size_t max_au = std::min(config.max_au, length);
    size_t max_gc = std::min(config.max_gc, length);
    size_t max_gu = std::min(config.max_gu, length);

    // Adjust for closing GC base pairs
    max_gc -= config.closing_gc;
    length -= config.closing_gc;

    std::vector<std::vector<size_t>> prev(max_au + 1, std::vector<size_t>(max_gc + 1, 0));
    std::vector<std::vector<size_t>> cur(max_au + 1, std::vector<size_t>(max_gc + 1, 0));

    prev[0][0] = 1;

    for (size_t i = 1; i <= length; ++i) {
        for (size_t a = 0; a <= max_au; ++a) {
            for (size_t b = 0; b <= max_gc; ++b) {
                size_t c = i - a - b;
                if (c > max_gu) continue;

                size_t count = 0;
                if (a > 0) count += prev[a - 1][b];
                if (b > 0) count += prev[a][b - 1];
                count += prev[a][b];

                cur[a][b] = count;
            }
        }
        std::swap(prev, cur);
    }

    size_t result = 0;
    for (size_t a = 0; a <= max_au; ++a) {
        for (size_t b = 0; b <= max_gc; ++b) {
            size_t c = length - a - b;
            if (c <= max_gu) result += prev[a][b];
        }
    }

    return result * (1ULL << (length + config.closing_gc));
}

void BarcodeConfig::validate(size_t library_size) const {
    if (!is_enabled()) return;

    stem.validate();

    size_t possible = _stem_counts(stem_length, stem);
    if (possible < library_size) {
        throw std::invalid_argument(
            "The barcode length (" + std::to_string(stem_length) +
            ") and stem constraints can only accommodate " +
            std::to_string(possible) + " of the required " +
            std::to_string(library_size) + " unique barcodes."
        );
    }
}
