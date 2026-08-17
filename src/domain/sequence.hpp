#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <string>
#include <vector>
#include <random>

// Valid nucleotide bases
constexpr char BASE_A = 'A';
constexpr char BASE_C = 'C';
constexpr char BASE_G = 'G';
constexpr char BASE_T = 'T';
constexpr char BASE_U = 'U';

// The two nucleotide alphabets. DNA uses T; RNA uses U.
enum class Alphabet {
    DNA,
    RNA
};

// Returns RNA if the sequence contains U, and DNA otherwise.
Alphabet detect_alphabet(const std::string& seq);

// Returns the four bases of the given alphabet.
const std::vector<char>& alphabet_bases(Alphabet alphabet);

// Returns the Watson-Crick complement of a base, written in the given
// alphabet. Throws on an invalid base.
char complement(char base, Alphabet alphabet);

// Converts a sequence between alphabets.
std::string to_dna(const std::string& seq);
std::string to_rna(const std::string& seq);

// Replaces IUPAC polybase codes with random concrete bases.
// Emits DNA bases; generation is DNA-canonical, so convert at output
// boundaries with to_rna.
std::string replace_polybases(const std::string& seq, std::mt19937& gen);

// Generates a uniform random sequence. Emits DNA bases; generation is
// DNA-canonical, so convert at output boundaries with to_rna.
std::string random_sequence(size_t length, std::mt19937& gen);

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

    static bool is_valid_base(char c);

    // Concatenation
    Sequence operator+(const Sequence& other) const;
    Sequence& operator+=(const Sequence& other);

    // String conversion
    operator const std::string&() const { return _seq; }

private:
    std::string _seq;
};

#endif
