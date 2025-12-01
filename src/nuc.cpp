#include "nuc.hpp"
#include <algorithm>
#include <cinttypes>
#include <stdexcept>

//
// Constants
//

static inline constexpr size_t _LOOP_LENGTH = 4;

static inline const std::vector<char> A_BASES = {'A'};
static inline const std::vector<char> C_BASES = {'C'};
static inline const std::vector<char> G_BASES = {'G'};
static inline const std::vector<char> T_BASES = {'T'};
static inline const std::vector<char> N_BASES = {'A', 'C', 'G', 'T'};
static inline const std::vector<char> R_BASES = {'A', 'G'};
static inline const std::vector<char> Y_BASES = {'C', 'T'};
static inline const std::vector<char> K_BASES = {'G', 'T'};
static inline const std::vector<char> M_BASES = {'A', 'C'};
static inline const std::vector<char> S_BASES = {'C', 'G'};
static inline const std::vector<char> W_BASES = {'A', 'T'};
static inline const std::vector<char> V_BASES = {'A', 'C', 'G'};
static inline const std::vector<char> D_BASES = {'A', 'G', 'T'};
static inline const std::vector<char> H_BASES = {'A', 'C', 'T'};
static inline const std::vector<char> B_BASES = {'C', 'G', 'T'};

std::vector<char> _get_polybase_arr(const char& base) {
    switch (base) {
        case 'A': { return A_BASES; }
        case 'C': { return C_BASES; }
        case 'G': { return G_BASES; }
        case 'T':
        case 'U': { return T_BASES; }
        case 'R': { return R_BASES; }
        case 'Y': { return Y_BASES; }
        case 'K': { return K_BASES; }
        case 'M': { return M_BASES; }
        case 'S': { return S_BASES; }
        case 'W': { return W_BASES; }
        case 'V': { return V_BASES; }
        case 'D': { return D_BASES; }
        case 'H': { return H_BASES; }
        case 'B': { return B_BASES; }
        default:  { return N_BASES; }
    }
}

static inline const std::vector<std::string> TETRALOOPS = {"TTCG","GTGA","TACG"};

enum class _SEQ_SIDE {Five, Three};
enum class _BP_TYPE {AT, GC, GT, None};
enum class _PAD_TYPE {Hairpin, Disordered};



//
// Helpers
//



static inline size_t _sample_from_range(
    size_t low,
    size_t high,
    std::mt19937& gen
) {
    std::uniform_int_distribution<> dist(low, high);
    return dist(gen);
}

template <typename T>
static inline T _sample_from_vector(
    const std::vector<T>& values,
    std::mt19937& gen
) {
    size_t ix = _sample_from_range(0, values.size() - 1, gen);
    return values[ix];
}

static inline size_t _stem_length(size_t length) {
    return (length - _LOOP_LENGTH) / 2;
}

static inline size_t _hairpin_length(size_t stem_length) {
    return 2 * stem_length + _LOOP_LENGTH;
}

static inline std::string _poly_a(size_t count) {
    return std::string(count, _A);
}

char _complement(const char& base) {
    switch (base) {
        case _A: { return _T; }
        case _C: { return _G; }
        case _G: { return _C; }
        case _U:
        case _T: { return _A; }
        default: {
            throw std::runtime_error("Invalid base \"" + std::string{base} + "\".");
        }
    }
}

static inline std::string _reverse_complement(const std::string& sequence) {
    size_t length = sequence.length();
    std::string reverse(length, '\0');
    for (size_t ix = 0; ix < length; ix++) {
        reverse[ix] = _complement(sequence[length - ix - 1]);
    }
    return reverse;
}

static inline char _replace_polybase(
    const char& base,
    std::mt19937& gen
) {
    std::vector<char> values = _get_polybase_arr(base);
    return _sample_from_vector<char>(values, gen);
}

std::string _replace_polybases(
    const std::string& sequence,
    std::mt19937& gen
) {
    std::string new_sequence(sequence.length(), '\0');
    for (size_t ix = 0; ix < sequence.length(); ix++) {
        new_sequence[ix] = _replace_polybase(sequence[ix], gen);
    }
    return new_sequence;
}

static inline char _to_dna(char base) {
    switch (base) {
        case _U: { return _T; }
        default: { return base; }
    }
}

static inline char _to_rna(char base) {
    switch (base) {
        case _T: { return _U; }
        default: { return base; }
    }
}

std::string _to_dna(const std::string& sequence) {
    std::string dna(sequence.length(), '\0');
    for (size_t ix = 0; ix < sequence.length(); ix++) {
        dna[ix] = _to_dna(sequence[ix]);
    }
    return dna;
}

std::string _to_rna(const std::string& sequence) {
    std::string rna(sequence.length(), '\0');
    for (size_t ix = 0; ix < sequence.length(); ix++) {
        rna[ix] = _to_rna(sequence[ix]);
    }
    return rna;
}

// Sampling bases and sequences

static inline char _random_base(std::mt19937& gen) {
    return _sample_from_vector<char>(N_BASES, gen);
}

std::string _random_sequence(
    size_t length,
    std::mt19937& gen
) {
    std::string sequence(length, '\0');
    for (size_t ix = 0; ix < length; ix++) {
        sequence[ix] = _random_base(gen);
    }
    return sequence;
}

// Sampling base pairs and hairpins

static inline bool _is_at(char b1, char b2) {
    return (
        (b1 == _A && b2 == _T) ||
        (b1 == _T && b2 == _A)
    );
}

static inline bool _is_gc(char b1, char b2) {
    return (
        (b1 == _G && b2 == _C) ||
        (b1 == _C && b2 == _G)
    );
}

static inline bool _is_gt(char b1, char b2) {
    return (
        (b1 == _G && b2 == _T) ||
        (b1 == _T && b2 == _G)
    );
}

class _BP {
public:
    char b1, b2;
    _BP() = default;
    _BP(char b1, char b2) : b1(b1), b2(b2) { };
    _BP_TYPE type() const {
        if (_is_at(b1, b2)) {
            return _BP_TYPE::AT;
        } else if (_is_gc(b1, b2)) {
            return _BP_TYPE::GC;
        } else if (_is_gt(b1, b2)) {
            return _BP_TYPE::GT;
        } else {
            return _BP_TYPE::None;
        }
    }
};

static inline const std::vector<_BP> _AT_BPS = {
    _BP(_A, _T),
    _BP(_T, _A),
};

static inline const std::vector<_BP> _GC_BPS = {
    _BP(_C, _G),
    _BP(_G, _C),
};

static inline const std::vector<_BP> _GU_BPS = {
    _BP(_T, _G),
    _BP(_G, _T)
};

static inline std::string _hairpin_from_bp(
    const std::vector<_BP>& pairs,
    const std::string& tetraloop
) {
    size_t length = pairs.size();
    std::string sense(length, '\0');
    std::string antisense(length, '\0');
    for (size_t ix = 0; ix < length; ix++) {
        sense[ix] = pairs[ix].b1;
        antisense[length - ix - 1] = pairs[ix].b2;
    }
    return sense + tetraloop + antisense;
}

template <typename T>
static inline void _insert(
    std::vector<T>& arr,
    const std::vector<T>& to_insert
) {
    arr.insert(arr.end(), to_insert.begin(), to_insert.end());
}

static inline std::vector<_BP> _get_bp_sample_arr(
    bool use_au,
    bool use_gc,
    bool use_gu
) {
    std::vector<_BP> bp_arr;
    if (use_au) {
        _insert<_BP>(bp_arr, _AT_BPS);
    }
    if (use_gc) {
        _insert<_BP>(bp_arr, _GC_BPS);
    }
    if (use_gu) {
        _insert<_BP>(bp_arr, _GU_BPS);
    }
    return bp_arr;
}

static inline _BP _random_base_pair(
    std::mt19937& gen,
    bool use_au,
    bool use_gc,
    bool use_gu
) {
    std::vector<_BP> bp_arr = _get_bp_sample_arr(use_au, use_gc, use_gu);
    return _sample_from_vector<_BP>(bp_arr, gen);
}

static inline std::string _random_tetraloop(std::mt19937& gen) {
    return _sample_from_vector<std::string>(TETRALOOPS, gen);
}

static inline void _update_counts(
    const _BP& pair,
    size_t& max_au,
    size_t& max_gc,
    size_t& max_gu
) {
    switch (pair.type()) {
        case _BP_TYPE::AT:   { max_au--; break; }
        case _BP_TYPE::GC:   { max_gc--; break; }
        case _BP_TYPE::GT:   { max_gu--; break; }
        case _BP_TYPE::None: {           break; }
    }
}

std::string _random_hairpin(
    size_t stem_length,
    const StemConfig& config,
    std::mt19937& gen
) {
    size_t max_au = config.max_au;
    size_t max_gc = config.max_gc;
    size_t max_gu = config.max_gu;
    size_t closing_gc = config.closing_gc;

    std::vector<_BP> pairs(stem_length);
    for (size_t ix = 0; ix < closing_gc; ix++) {
        pairs[ix] = _random_base_pair(gen, false, true, false);
        max_gc--;
    }
    for (size_t ix = closing_gc; ix < stem_length; ix++) {
        _BP pair = _random_base_pair(
            gen,
            max_au > 0,
            max_gc > 0,
            max_gu > 0
        );
        pairs[ix] = pair;
        _update_counts(pair, max_au, max_gc, max_gu);
    }
    std::shuffle(pairs.begin() + closing_gc, pairs.end(), gen);
    std::string loop = _random_tetraloop(gen);
    return _hairpin_from_bp(pairs, loop);
}

//
// Tools for padding
//

_PAD_TYPE _get_pad_type(
    size_t length,
    const StemConfig& config
) {
    size_t _min_hairpin_length = _hairpin_length(config.min_length);
    if (length >= _min_hairpin_length + config.spacer_length) {
        return _PAD_TYPE::Hairpin;
    } else {
        return _PAD_TYPE::Disordered;
    }
}

size_t _get_pad_stem_length(
    size_t length,
    const StemConfig& config,
    std::mt19937& gen
) {
    size_t _max_stem_length = std::min(
        _stem_length(length - config.spacer_length),
        config.max_length
    );
    return _sample_from_range(
        config.min_length,
        _max_stem_length,
        gen
    );
}

typedef std::pair<std::string, _PAD_TYPE> _PAD;

static inline _PAD _get_single_pad(
    size_t length,
    const StemConfig& config,
    std::mt19937& gen
) {
    _PAD_TYPE type = _get_pad_type(length, config);
    switch (type) {
        case _PAD_TYPE::Disordered: {
            return {_random_sequence(length, gen), _PAD_TYPE::Disordered};
        }
        case _PAD_TYPE::Hairpin: {
            size_t stem_length = _get_pad_stem_length(length, config, gen);
            std::string hairpin = _random_hairpin(stem_length, config, gen);
            std::string spacer = _poly_a(config.spacer_length);
            // The spacer always goes on the 3' end
            return {hairpin + spacer, _PAD_TYPE::Hairpin};
        }
        default: {
            throw std::runtime_error("Invalid pad type.");
        }
    }
}

std::string _get_padding(
    size_t length,
    const StemConfig& config,
    std::mt19937& gen
) {
    std::string padding;
    while (length > 0) {
        std::string tmp = _get_single_pad(length, config, gen).first;
        length -= tmp.length();
        padding += tmp;
    }
    return padding;
}
