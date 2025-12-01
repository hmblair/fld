#include "hairpin.hpp"
#include "sequence.hpp"
#include <algorithm>

static const std::vector<std::string> TETRALOOPS = {"TTCG", "GTGA", "TACG"};

static const std::vector<BasePair> AT_PAIRS = {
    BasePair(BASE_A, BASE_T),
    BasePair(BASE_T, BASE_A),
};

static const std::vector<BasePair> GC_PAIRS = {
    BasePair(BASE_C, BASE_G),
    BasePair(BASE_G, BASE_C),
};

static const std::vector<BasePair> GU_PAIRS = {
    BasePair(BASE_T, BASE_G),
    BasePair(BASE_G, BASE_T)
};

static size_t sample_range(size_t low, size_t high, std::mt19937& gen) {
    std::uniform_int_distribution<size_t> dist(low, high);
    return dist(gen);
}

template <typename T>
static T sample_vector(const std::vector<T>& values, std::mt19937& gen) {
    size_t ix = sample_range(0, values.size() - 1, gen);
    return values[ix];
}

bool BasePair::is_au(char b1, char b2) {
    return (b1 == BASE_A && b2 == BASE_T) || (b1 == BASE_T && b2 == BASE_A);
}

bool BasePair::is_gc(char b1, char b2) {
    return (b1 == BASE_G && b2 == BASE_C) || (b1 == BASE_C && b2 == BASE_G);
}

bool BasePair::is_gu(char b1, char b2) {
    return (b1 == BASE_G && b2 == BASE_T) || (b1 == BASE_T && b2 == BASE_G);
}

BasePairType BasePair::type() const {
    if (is_au(five_prime, three_prime)) return BasePairType::AU;
    if (is_gc(five_prime, three_prime)) return BasePairType::GC;
    if (is_gu(five_prime, three_prime)) return BasePairType::GU;
    return BasePairType::None;
}

static std::vector<BasePair> get_bp_sample_arr(bool use_au, bool use_gc, bool use_gu) {
    std::vector<BasePair> bp_arr;
    if (use_au) {
        bp_arr.insert(bp_arr.end(), AT_PAIRS.begin(), AT_PAIRS.end());
    }
    if (use_gc) {
        bp_arr.insert(bp_arr.end(), GC_PAIRS.begin(), GC_PAIRS.end());
    }
    if (use_gu) {
        bp_arr.insert(bp_arr.end(), GU_PAIRS.begin(), GU_PAIRS.end());
    }
    return bp_arr;
}

static BasePair random_base_pair(std::mt19937& gen, bool use_au, bool use_gc, bool use_gu) {
    std::vector<BasePair> bp_arr = get_bp_sample_arr(use_au, use_gc, use_gu);
    return sample_vector(bp_arr, gen);
}

static void update_counts(const BasePair& pair, size_t& max_au, size_t& max_gc, size_t& max_gu) {
    switch (pair.type()) {
        case BasePairType::AU: max_au--; break;
        case BasePairType::GC: max_gc--; break;
        case BasePairType::GU: max_gu--; break;
        case BasePairType::None: break;
    }
}

Hairpin::Hairpin(std::vector<BasePair> pairs, std::string loop)
    : _pairs(std::move(pairs)), _loop(std::move(loop)) {}

Hairpin Hairpin::random(
    size_t stem_length,
    const StemConfig& config,
    std::mt19937& gen
) {
    size_t max_au = config.max_au;
    size_t max_gc = config.max_gc;
    size_t max_gu = config.max_gu;
    size_t closing_gc = config.closing_gc;

    std::vector<BasePair> pairs(stem_length);

    // First, add closing GC pairs
    for (size_t ix = 0; ix < closing_gc; ix++) {
        pairs[ix] = random_base_pair(gen, false, true, false);
        max_gc--;
    }

    // Then fill the rest of the stem
    for (size_t ix = closing_gc; ix < stem_length; ix++) {
        BasePair pair = random_base_pair(
            gen,
            max_au > 0,
            max_gc > 0,
            max_gu > 0
        );
        pairs[ix] = pair;
        update_counts(pair, max_au, max_gc, max_gu);
    }

    // Shuffle non-closing pairs
    std::shuffle(pairs.begin() + closing_gc, pairs.end(), gen);

    // Pick a random tetraloop
    std::string loop = sample_vector(TETRALOOPS, gen);

    return Hairpin(std::move(pairs), std::move(loop));
}

std::string Hairpin::str() const {
    size_t length = _pairs.size();
    std::string sense(length, '\0');
    std::string antisense(length, '\0');

    for (size_t ix = 0; ix < length; ix++) {
        sense[ix] = _pairs[ix].five_prime;
        antisense[length - ix - 1] = _pairs[ix].three_prime;
    }

    return sense + _loop + antisense;
}
