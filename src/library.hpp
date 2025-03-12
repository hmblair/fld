#ifndef LIBRARY_H
#define LIBRARY_H

#include "nuc.hpp"
#include "utils.hpp"
#include <unordered_set>

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
    void to_rna();
    void to_dna(std::mt19937 &gen);
    void primerize(
        const std::string& five,
        const std::string& three
    );
    void replace_barcode(
        size_t stem_length,
        std::mt19937 &gen,
        size_t max_stem_au,
        size_t max_stem_gc,
        size_t max_stem_gu,
        size_t closing_gc,
        std::unordered_set<std::string>& existing
    );
    void pad(
        size_t padded_size,
        size_t min_stem_length,
        size_t max_stem_length,
        size_t spacer_length,
        size_t max_stem_au,
        size_t max_stem_gc,
        size_t max_stem_gu,
        size_t closing_gc,
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

    void to_rna();
    void to_dna();

    void barcode(
        size_t stem_length,
        size_t max_stem_au,
        size_t max_stem_gc,
        size_t max_stem_gu,
        size_t closing_gc
    );
    void pad(
        size_t padded_size,
        size_t min_stem_length,
        size_t max_stem_length,
        size_t spacer_length,
        size_t max_stem_au,
        size_t max_stem_gc,
        size_t max_stem_gu,
        size_t closing_gc
    );
    void primerize(
        const std::string& five,
        const std::string& three
    );

    void to_csv(const std::string& filename) const;
    void to_txt(const std::string& filename) const;
    void to_fasta(const std::string& filename) const;
    void save(const std::string& prefix) const;


private:
    // For RNG
    std::mt19937 _gen;
    // The library constructs and barcodes
    std::vector<Construct> _sequences;
    std::unordered_set<std::string> _barcodes;
};

Construct _from_record(const std::string& record);
Library _from_csv(const std::string& filename);

void _add_library_elements(
    Library& library,
    size_t pad_to_length,
    size_t barcode_stem_length,
    size_t min_stem_length,
    size_t max_stem_length,
    size_t max_stem_au,
    size_t max_stem_gc,
    size_t max_stem_gu,
    size_t closing_gu,
    const std::string& five_const,
    const std::string& three_const
);


#endif


