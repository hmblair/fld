#include "stem_config.hpp"

static constexpr size_t STEM_MIN = 4;

void StemConfig::validate() const {
    if (min_length < STEM_MIN) {
        throw std::invalid_argument(
            "Minimum stem length must be at least " + std::to_string(STEM_MIN)
        );
    }
    if (max_length < min_length) {
        throw std::invalid_argument(
            "Maximum stem length must be >= minimum stem length"
        );
    }
    if (!can_accommodate(max_length)) {
        throw std::invalid_argument(
            "Combined base pair limits insufficient for maximum stem length"
        );
    }
}

bool StemConfig::can_accommodate(size_t length) const {
    return max_possible_length() >= length;
}

size_t StemConfig::max_possible_length() const {
    return max_au + max_gc + max_gu;
}

StemConfig StemConfig::for_padding() {
    StemConfig config;
    config.min_length = 7;
    config.max_length = 9;
    config.max_au = INT_MAX;
    config.max_gc = 5;
    config.max_gu = 0;
    config.closing_gc = 1;
    config.spacer_length = 2;
    return config;
}

StemConfig StemConfig::for_barcode(size_t stem_length) {
    StemConfig config;
    config.min_length = stem_length;
    config.max_length = stem_length;
    config.max_au = INT_MAX;
    config.max_gc = 5;
    config.max_gu = 0;
    config.closing_gc = 1;
    config.spacer_length = 0;
    return config;
}
