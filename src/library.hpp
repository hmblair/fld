#ifndef LIBRARY_H
#define LIBRARY_H

#include "nuc.hpp"
#include "barcodes.hpp"
#include "utils.hpp"
#include "config/design_config.hpp"

class DesignArgs : public Program {
public:

    Arg<std::string> file;
    Arg<std::string> output;
    Arg<int> pad_to;
    Arg<int> barcode_length;
    Arg<bool> overwrite;
    Arg<std::string> five_const;
    Arg<std::string> three_const;
    Arg<int> min_stem_length;
    Arg<int> max_stem_length;
    Arg<int> max_au;
    Arg<int> max_gc;
    Arg<int> max_gu;
    Arg<int> closing_gc;
    Arg<int> spacer;

    DesignArgs();

};

class Construct {
public:

    Construct(
        std::string name,
        std::string sublibrary,
        std::string fivep_const,
        std::string fivep_padding,
        std::string design,
        std::string threep_pading,
        std::string barcode,
        std::string threep_const
    );
    std::string str() const;
    std::string csv_record() const;
    std::string name() const;
    size_t length() const;
    size_t design_length() const;
    void to_dna();
    void to_rna();
    void replace_polybases(std::mt19937 &gen);
    void primerize(
        const std::string& five,
        const std::string& three
    );
    void replace_barcode(
        size_t stem_length,
        const StemConfig& config,
        std::mt19937 &gen,
        std::unordered_set<std::string>& existing
    );
    void pad(
        size_t padded_size,
        const StemConfig& config,
        std::mt19937 &gen
    );
    bool has_barcode() const;
    std::string barcode() const;
    void remove_barcode();
    void remove_padding();

private:

    // Metadata
    std::string _name;
    std::string _sublibrary;
    // Different portions of the construct
    std::string _fivep_const;
    std::string _fivep_padding;
    std::string _design;
    std::string _threep_padding;
    std::string _barcode;
    std::string _threep_const;

};

class Library {
public:

    Library() = default;
    Library(std::vector<Construct>& sequences);

    size_t size() const;
    size_t barcodes() const;

    void to_csv(const std::string& filename) const;
    void to_txt(const std::string& filename) const;
    void to_fasta(const std::string& filename) const;
    void save(const std::string& prefix) const;

    void to_rna();
    void to_dna();
    void replace_polybases();

    void barcode(
        size_t stem_length,
        const StemConfig& config
    );
    void pad(
        size_t padded_size,
        const StemConfig& config
    );
    void primerize(
        const std::string& five,
        const std::string& three
    );

private:

    // For RNG
    std::mt19937 _gen;
    // The library constructs
    std::vector<Construct> _sequences;
    // A hash map for easy barcode lookup
    std::unordered_set<std::string> _barcodes;

};


Library _from_csv(const std::string& filename);


void _design(const DesignConfig& config);

#endif


