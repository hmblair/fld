#include "sequence.hpp"
#include "sampling.hpp"
#include <stdexcept>

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

static const std::vector<char> DNA_BASES = {BASE_A, BASE_C, BASE_G, BASE_T};
static const std::vector<char> RNA_BASES = {BASE_A, BASE_C, BASE_G, BASE_U};

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

Alphabet detect_alphabet(const std::string& seq) {
    if (seq.find(BASE_U) != std::string::npos || seq.find('u') != std::string::npos) {
        return Alphabet::RNA;
    }
    return Alphabet::DNA;
}

const std::vector<char>& alphabet_bases(Alphabet alphabet) {
    return (alphabet == Alphabet::RNA) ? RNA_BASES : DNA_BASES;
}

char complement(char base, Alphabet alphabet) {
    switch (base) {
        case BASE_A: return (alphabet == Alphabet::RNA) ? BASE_U : BASE_T;
        case BASE_C: return BASE_G;
        case BASE_G: return BASE_C;
        case BASE_U:
        case BASE_T: return BASE_A;
        default:
            throw std::runtime_error("Invalid base \"" + std::string{base} + "\".");
    }
}

static char base_to_dna(char base) {
    switch (base) {
        case 'U': return 'T';
        case 'u': return 't';
        default:  return base;
    }
}

static char base_to_rna(char base) {
    switch (base) {
        case 'T': return 'U';
        case 't': return 'u';
        default:  return base;
    }
}

std::string to_dna(const std::string& seq) {
    std::string dna(seq.length(), '\0');
    for (size_t ix = 0; ix < seq.length(); ix++) {
        dna[ix] = base_to_dna(seq[ix]);
    }
    return dna;
}

std::string to_rna(const std::string& seq) {
    std::string rna(seq.length(), '\0');
    for (size_t ix = 0; ix < seq.length(); ix++) {
        rna[ix] = base_to_rna(seq[ix]);
    }
    return rna;
}

std::string replace_polybases(const std::string& seq, std::mt19937& gen) {
    std::string result(seq.length(), '\0');
    for (size_t ix = 0; ix < seq.length(); ix++) {
        const std::vector<char>& bases = get_polybase_arr(seq[ix]);
        result[ix] = sample_from_vector(bases, gen);
    }
    return result;
}

std::string random_sequence(size_t length, std::mt19937& gen) {
    std::string seq(length, '\0');
    for (size_t ix = 0; ix < length; ix++) {
        seq[ix] = sample_from_vector(N_BASES, gen);
    }
    return seq;
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

bool Sequence::is_valid_base(char c) {
    return c == BASE_A || c == BASE_C || c == BASE_G || c == BASE_T || c == BASE_U;
}

Sequence Sequence::to_dna() const {
    return Sequence(::to_dna(_seq));
}

Sequence Sequence::to_rna() const {
    return Sequence(::to_rna(_seq));
}

Sequence Sequence::replace_polybases(std::mt19937& gen) const {
    return Sequence(::replace_polybases(_seq, gen));
}

Sequence Sequence::operator+(const Sequence& other) const {
    return Sequence(_seq + other._seq);
}

Sequence& Sequence::operator+=(const Sequence& other) {
    _seq += other._seq;
    return *this;
}
