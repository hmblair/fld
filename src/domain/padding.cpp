#include "padding.hpp"
#include "hairpin.hpp"
#include "sequence.hpp"
#include "sampling.hpp"
#include <stdexcept>

enum class PadType {
    Hairpin,
    Disordered
};

static size_t stem_length_for(size_t length) {
    return (length - HAIRPIN_LOOP_LENGTH) / 2;
}

static size_t hairpin_length_for(size_t stem_length) {
    return 2 * stem_length + HAIRPIN_LOOP_LENGTH;
}

static std::string poly_a(size_t count) {
    return std::string(count, BASE_A);
}

static PadType get_pad_type(size_t length, const StemConfig& config) {
    size_t min_hairpin_length = hairpin_length_for(config.min_length);
    if (length >= min_hairpin_length + config.spacer_length) {
        return PadType::Hairpin;
    } else {
        return PadType::Disordered;
    }
}

static size_t get_pad_stem_length(
    size_t length,
    const StemConfig& config,
    std::mt19937& gen
) {
    size_t max_stem_length = std::min(
        stem_length_for(length - config.spacer_length),
        config.max_length
    );
    return sample_from_range(config.min_length, max_stem_length, gen);
}

static std::string get_single_pad(
    size_t length,
    const StemConfig& config,
    std::mt19937& gen
) {
    switch (get_pad_type(length, config)) {
        case PadType::Disordered: {
            return random_sequence(length, gen);
        }
        case PadType::Hairpin: {
            size_t stem_length = get_pad_stem_length(length, config, gen);
            std::string hairpin = Hairpin::random(stem_length, config, gen).str();
            std::string spacer = poly_a(config.spacer_length);
            // The spacer always goes on the 3' end
            return hairpin + spacer;
        }
        default: {
            throw std::runtime_error("Invalid pad type.");
        }
    }
}

std::string get_padding(
    size_t length,
    const StemConfig& config,
    std::mt19937& gen
) {
    std::string padding;
    while (length > 0) {
        std::string tmp = get_single_pad(length, config, gen);
        length -= tmp.length();
        padding += tmp;
    }
    return padding;
}
