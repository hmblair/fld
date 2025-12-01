#include "sequence.hpp"
#include <stdexcept>
#include <vector>

static const std::vector<char> A_BASES = {'A'};
static const std::vector<char> C_BASES = {'C'};
static const std::vector<char> G_BASES = {'G'};
static const std::vector<char> T_BASES = {'T'};
static const std::vector<char> N_BASES = {'A', 'C', 'G', 'T'};
static const std::vector<char> R_BASES = {'A', 'G'};
static const std::vector<char> Y_BASES = {'C', 'T'};
static const std::vector<char> K_BASES = {'G', 'T'};
static const std::vector<char> M_BASES = {'A', 'C'};
static const std::vector<char> S_BASES = {'C', 'G'};
static const std::vector<char> W_BASES = {'A', 'T'};
static const std::vector<char> V_BASES = {'A', 'C', 'G'};
static const std::vector<char> D_BASES = {'A', 'G', 'T'};
static const std::vector<char> H_BASES = {'A', 'C', 'T'};
static const std::vector<char> B_BASES = {'C', 'G', 'T'};

static const std::vector<char>& get_polybase_arr(char base) {
    switch (base) {
        case 'A': return A_BASES;
        case 'C': return C_BASES;
        case 'G': return G_BASES;
        case 'T':
        case 'U': return T_BASES;
        case 'R': return R_BASES;
        case 'Y': return Y_BASES;
        case 'K': return K_BASES;
        case 'M': return M_BASES;
        case 'S': return S_BASES;
        case 'W': return W_BASES;
        case 'V': return V_BASES;
        case 'D': return D_BASES;
        case 'H': return H_BASES;
        case 'B': return B_BASES;
        default:  return N_BASES;
    }
}

static size_t sample_from_range(size_t low, size_t high, std::mt19937& gen) {
    std::uniform_int_distribution<size_t> dist(low, high);
    return dist(gen);
}

template <typename T>
static T sample_from_vector(const std::vector<T>& values, std::mt19937& gen) {
    size_t ix = sample_from_range(0, values.size() - 1, gen);
    return values[ix];
}

Sequence::Sequence(std::string seq) : _seq(std::move(seq)) {}

const std::string& Sequence::str() const {
    return _seq;
}

size_t Sequence::length() const {
    return _seq.length();
}

bool Sequence::empty() const {
    return _seq.empty();
}

char Sequence::complement(char base) {
    switch (base) {
        case BASE_A: return BASE_T;
        case BASE_C: return BASE_G;
        case BASE_G: return BASE_C;
        case BASE_U:
        case BASE_T: return BASE_A;
        default:
            throw std::runtime_error("Invalid base \"" + std::string{base} + "\".");
    }
}

bool Sequence::is_valid_base(char c) {
    return c == BASE_A || c == BASE_C || c == BASE_G || c == BASE_T || c == BASE_U;
}

std::string Sequence::_to_dna_impl(const std::string& s) {
    std::string dna(s.length(), '\0');
    for (size_t ix = 0; ix < s.length(); ix++) {
        dna[ix] = (s[ix] == BASE_U) ? BASE_T : s[ix];
    }
    return dna;
}

std::string Sequence::_to_rna_impl(const std::string& s) {
    std::string rna(s.length(), '\0');
    for (size_t ix = 0; ix < s.length(); ix++) {
        rna[ix] = (s[ix] == BASE_T) ? BASE_U : s[ix];
    }
    return rna;
}

std::string Sequence::_replace_polybases_impl(const std::string& s, std::mt19937& gen) {
    std::string result(s.length(), '\0');
    for (size_t ix = 0; ix < s.length(); ix++) {
        const std::vector<char>& bases = get_polybase_arr(s[ix]);
        result[ix] = sample_from_vector(bases, gen);
    }
    return result;
}

Sequence Sequence::to_dna() const {
    return Sequence(_to_dna_impl(_seq));
}

Sequence Sequence::to_rna() const {
    return Sequence(_to_rna_impl(_seq));
}

Sequence Sequence::replace_polybases(std::mt19937& gen) const {
    return Sequence(_replace_polybases_impl(_seq, gen));
}

Sequence Sequence::operator+(const Sequence& other) const {
    return Sequence(_seq + other._seq);
}

Sequence& Sequence::operator+=(const Sequence& other) {
    _seq += other._seq;
    return *this;
}
