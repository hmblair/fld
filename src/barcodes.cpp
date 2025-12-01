#include "barcodes.hpp"
#include <climits>
#include <machine/limits.h>


//
// Hamming ball of hairpin
//

static inline char _pairing_complement(const char& base) {

    // The only point mutations which preserve base-pairing (and therefore could
    // potentially correspond to another sample from _random_hairpin) are
    // A -> G
    // C -> T
    // G -> A
    // T -> C
    switch (base) {
        case _A: { return _G; }
        case _C: { return _T; }
        case _G: { return _A; }
        case _T: { return _C; }
        default: {
            throw std::runtime_error("Invalid base " + std::string{base} + ".");
        }
    }

}

static inline std::vector<std::string> _unit_hamming_ball(
    const std::string& sequence
) {

    std::vector<std::string> results;

    for (size_t ix = 0; ix < sequence.length(); ix++) {
        std::string tmp = sequence;
        tmp[ix] = _pairing_complement(sequence[ix]);
        results.push_back(tmp);
    }

    results.push_back(sequence);
    return results;

}

static inline bool _has_hamming_neighbour(
    const std::string& sequence,
    const std::unordered_set<std::string>& set
) {

    std::vector<std::string> ball = _unit_hamming_ball(sequence);

    for (const auto& element : ball) {
        if (set.find(element) != set.end()) {
            return true;
        }
    }
    return false;

}


//
// DP algorithm for computing the number of barcodes satisfying given constraints
//


static inline std::vector<std::vector<size_t>> _inner_loop(
    size_t length,
    size_t max_au,
    size_t max_gc,
    size_t max_gu
) {

    std::vector<std::vector<size_t>> prev(max_au + 1, std::vector<size_t>(max_gc + 1, 0));
    std::vector<std::vector<size_t>> curr(max_au + 1, std::vector<size_t>(max_gc + 1, 0));

    prev[0][0] = 1;
    for (size_t i = 1; i <= length; ++i) {
        for (size_t a = 0; a <= max_au; ++a) {
            for (size_t b = 0; b <= max_gc; ++b) {
                size_t c = i - a - b;
                if (c > max_gu || c < 0) { continue; }

                size_t count = 0;
                if (a > 0) count += prev[a - 1][b];
                if (b > 0) count += prev[a][b - 1];
                count += prev[a][b];

                curr[a][b] = count;
            }
        }
        std::swap(prev, curr);
    }

    return prev;

}

static inline size_t _accumulate(
    const std::vector<std::vector<size_t>>& dp_arr,
    size_t length,
    size_t max_au,
    size_t max_gc,
    size_t max_gu
) {

    size_t result = 0;
    for (size_t num_au = 0; num_au <= max_au; ++num_au) {
        for (size_t num_gc = 0; num_gc <= max_gc; ++num_gc) {
            size_t num_gu = length - num_au - num_gc;
            if (num_gu <= max_gu) {
                result += dp_arr[num_au][num_gc];
            }
        }
    }

    return result;

}


static inline size_t _stem_counts(
    size_t length,
    const StemConfig& config
) {
    size_t max_au = std::min(config.max_au, length);
    size_t max_gc = std::min(config.max_gc, length);
    size_t max_gu = std::min(config.max_gu, length);
    size_t closing_gc = config.closing_gc;

    // Adjust for closing GC base pairs
    max_gc -= closing_gc;
    length -= closing_gc;

    // Fill the DP array
    std::vector<std::vector<size_t>> dp_arr = _inner_loop(length, max_au, max_gc, max_gu);

    // Accumulate results
    size_t count = _accumulate(dp_arr, length, max_au, max_gc, max_gu);

    // Account for both orientations of each base pair
    size_t o_factor = 1ULL << (length + closing_gc);
    return count * o_factor;

}

static inline void _check_if_enough_barcodes(
    size_t count,
    size_t stem_length,
    const StemConfig& config
) {
    size_t _counts = _stem_counts(stem_length, config);
    if (_counts < count) {
        throw std::runtime_error(
            "The barcode length (" + std::to_string(stem_length) +
            ") and maximum base-pair counts (" + std::to_string(config.max_au) +
            ", " + std::to_string(config.max_gc) + " and " + std::to_string(config.max_gu) +
            ") are only large enough to accomodate " + std::to_string(_counts) +
            " of the required " + std::to_string(count) + " unique barcodes."
        );
    }
}

std::string _random_barcode(
    size_t stem_length,
    const StemConfig& config,
    std::mt19937 &gen,
    const std::unordered_set<std::string>& existing
) {
    std::string barcode;

    do {
        barcode = _random_hairpin(stem_length, config, gen);
    }
    while (_has_hamming_neighbour(barcode, existing));

    return barcode;
}

bool _insert_if_not_neighbour(
    const std::string& sequence,
    std::unordered_set<std::string>& barcodes
) {
    bool is_neighbour = _has_hamming_neighbour(sequence, barcodes);
    if (!is_neighbour) {
        barcodes.insert(sequence);
    }
    return !is_neighbour;
}

void _get_barcodes(
    size_t count,
    size_t stem_length,
    const StemConfig& config,
    std::mt19937 &gen,
    std::unordered_set<std::string>& barcodes
) {
    _check_if_enough_barcodes(count, stem_length, config);

    ProgressBar bar("Barcoding ");
    for (size_t ix = 0; ix < count; ix++) {
        std::string barcode = _random_barcode(stem_length, config, gen, barcodes);
        barcodes.insert(barcode);
        bar.update(ix + 1, count);
    }
}

void _barcodes(
    size_t count,
    const std::string& output,
    bool overwrite,
    size_t stem_length,
    const StemConfig& config
) {
    _remove_if_exists(output, overwrite);

    std::mt19937 gen = _init_gen();
    std::unordered_set<std::string> barcodes;
    _get_barcodes(count, stem_length, config, gen, barcodes);

    std::ofstream file(output);
    for (const std::string& barcode : barcodes) {
        file << barcode << "\n";
    }
}


//
// Argument parsing
//


static inline std::string _PARSER_NAME = "barcodes";

static inline std::string _COUNT_NAME = "--count";
static inline std::string _COUNT_HELP = "The number of barcodes to generate.";

static inline std::string _OUTPUT_NAME = "--output";
static inline std::string _OUTPUT_HELP = "The output file.";

// Required arguments
static inline std::string _BARCODE_LENGTH_NAME = "--length";
static inline std::string _BARCODE_LENGTH_HELP = "The length of the stem of each barcode.";

// Optional arguments with defaults
static inline std::string _OVERWRITE_NAME = "--overwrite";
static inline std::string _OVERWRITE_HELP = "Overwrite any existing file.";

static inline std::string _MAX_AU_NAME = "--max-au";
static inline int _MAX_AU_DEFAULT = INT_MAX;
static inline std::string _MAX_AU_HELP = "The maximum AU content of the stem.";

static inline std::string _MAX_GC_NAME = "--max-gc";
static inline int _MAX_GC_DEFAULT = INT_MAX;
static inline std::string _MAX_GC_HELP = "The maximum GC content of the stem.";

static inline std::string _MAX_GU_NAME = "--max-gu";
static inline int _MAX_GU_DEFAULT = 0;
static inline std::string _MAX_GU_HELP = "The maximum GU content of the stem.";

static inline std::string _CLOSING_GC_NAME = "--closing-gc";
static inline int _CLOSING_GC_DEFAULT = 1;
static inline std::string _CLOSING_GC_HELP = "The number of GC pairs to close the stem with.";

BarcodesArgs::BarcodesArgs() :
    Program(_PARSER_NAME),
    count(_parser, _COUNT_NAME, _COUNT_HELP),
    output(_parser, _OUTPUT_NAME, _OUTPUT_HELP),
    overwrite(_parser, _OVERWRITE_NAME, _OVERWRITE_HELP),
    stem_length(_parser, _BARCODE_LENGTH_NAME, _BARCODE_LENGTH_HELP),
    max_au(_parser, _MAX_AU_NAME, _MAX_AU_HELP, _MAX_AU_DEFAULT),
    max_gc(_parser, _MAX_GC_NAME, _MAX_GC_HELP, _MAX_GC_DEFAULT),
    max_gu(_parser, _MAX_GU_NAME, _MAX_GU_HELP, _MAX_GU_DEFAULT),
    closing_gc(_parser, _CLOSING_GC_NAME, _CLOSING_GC_HELP, _CLOSING_GC_DEFAULT) {
}
