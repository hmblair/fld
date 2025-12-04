/**
 * @file library.hpp
 * @brief Core data structures for RNA/DNA library design.
 *
 * This module defines the main data structures used in library design:
 * - Construct: A single sequence with its library elements (primers, padding, barcode)
 * - Library: A collection of Constructs with shared operations
 *
 * A Construct has the following structure (5' to 3'):
 *   [5' constant] [5' padding] [design] [3' padding] [barcode] [3' constant]
 *
 * The design region contains the actual sequence of interest, while padding
 * is added to reach a target length, and barcodes enable sequence identification.
 */

#ifndef LIBRARY_H
#define LIBRARY_H

#include "nuc.hpp"
#include "barcodes.hpp"
#include "utils.hpp"
#include "config/design_config.hpp"

/**
 * Command-line arguments for the 'design' subcommand.
 */
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

/**
 * A single library construct consisting of a design sequence and its
 * surrounding library elements (primers, padding, barcode).
 *
 * The construct structure (5' to 3'):
 *   [5' const] [5' padding] [design] [3' padding] [barcode] [3' const]
 */
class Construct {
public:
    Construct(
        std::string name,
        std::string sublibrary,
        std::string fivep_const,
        std::string fivep_padding,
        std::string design,
        std::string threep_padding,
        std::string barcode,
        std::string threep_const
    );

    /// Get the full construct sequence.
    std::string str() const;

    /// Get the construct as a CSV record.
    std::string csv_record() const;

    /// Get the construct name.
    std::string name() const;

    /// Get the total length of the construct.
    size_t length() const;

    /// Get the length of the design region (including padding).
    size_t design_length() const;

    /// Convert all bases to DNA (U -> T).
    void to_dna();

    /// Convert all bases to RNA (T -> U).
    void to_rna();

    /// Replace degenerate bases (N, R, Y, etc.) with random valid bases.
    void replace_polybases(std::mt19937& gen);

    /// Add 5' and 3' constant regions.
    void primerize(const std::string& five, const std::string& three);

    /// Generate a unique barcode for this construct.
    void replace_barcode(
        size_t stem_length,
        const StemConfig& config,
        std::mt19937& gen,
        std::unordered_set<std::string>& existing
    );

    /// Add padding hairpins to reach the target size.
    void pad(size_t padded_size, const StemConfig& config, std::mt19937& gen);

    /// Check if this construct has a barcode.
    bool has_barcode() const;

    /// Get the barcode sequence.
    std::string barcode() const;

    /// Remove the barcode from this construct.
    void remove_barcode();

    /// Remove padding from this construct.
    void remove_padding();

private:
    std::string _name;
    std::string _sublibrary;
    std::string _fivep_const;
    std::string _fivep_padding;
    std::string _design;
    std::string _threep_padding;
    std::string _barcode;
    std::string _threep_const;
};

/**
 * A collection of Constructs that can be processed as a batch.
 *
 * Provides operations for:
 * - Adding padding to all sequences
 * - Generating unique barcodes
 * - Converting between DNA/RNA
 * - Exporting to CSV, FASTA, and TXT formats
 */
class Library {
public:
    Library() = default;
    Library(std::vector<Construct>& sequences);

    /// Get the number of constructs in the library.
    size_t size() const;

    /// Get the number of constructs with barcodes.
    size_t barcodes() const;

    /// Export to CSV format.
    void to_csv(const std::string& filename) const;

    /// Export to plain text (one sequence per line).
    void to_txt(const std::string& filename) const;

    /// Export to FASTA format.
    void to_fasta(const std::string& filename) const;

    /// Export to all formats (CSV, FASTA, TXT).
    void save(const std::string& prefix) const;

    /// Convert all sequences to RNA.
    void to_rna();

    /// Convert all sequences to DNA.
    void to_dna();

    /// Replace degenerate bases in all sequences.
    void replace_polybases();

    /// Generate unique barcodes for all sequences.
    void barcode(size_t stem_length, const StemConfig& config);

    /// Add padding to all sequences to reach target size.
    void pad(size_t padded_size, const StemConfig& config);

    /// Add constant regions to all sequences.
    void primerize(const std::string& five, const std::string& three);

private:
    std::mt19937 _gen;
    std::vector<Construct> _sequences;
    std::unordered_set<std::string> _barcodes;
};

/// Load a library from a CSV file.
Library _from_csv(const std::string& filename);

/// Run the design pipeline with the given configuration.
void _design(const DesignConfig& config);

#endif
