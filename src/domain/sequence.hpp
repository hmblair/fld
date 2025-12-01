#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <string>
#include <random>

// Valid nucleotide bases
constexpr char BASE_A = 'A';
constexpr char BASE_C = 'C';
constexpr char BASE_G = 'G';
constexpr char BASE_T = 'T';
constexpr char BASE_U = 'U';

// A nucleotide sequence with validation and transformations
class Sequence {
public:
    Sequence() = default;
    explicit Sequence(std::string seq);

    // Access the underlying string
    const std::string& str() const;
    size_t length() const;
    bool empty() const;

    // Transformations (return new Sequence)
    Sequence to_dna() const;
    Sequence to_rna() const;
    Sequence replace_polybases(std::mt19937& gen) const;

    // Get complement of a base
    static char complement(char base);
    static bool is_valid_base(char c);

    // Concatenation
    Sequence operator+(const Sequence& other) const;
    Sequence& operator+=(const Sequence& other);

    // String conversion
    operator const std::string&() const { return _seq; }

private:
    std::string _seq;

    static std::string _to_dna_impl(const std::string& s);
    static std::string _to_rna_impl(const std::string& s);
    static std::string _replace_polybases_impl(const std::string& s, std::mt19937& gen);
};

#endif
