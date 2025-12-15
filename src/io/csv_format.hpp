/**
 * @file csv_format.hpp
 * @brief CSV format definitions for library files.
 *
 * Defines the column structure for library CSV files:
 *
 *   index,name,sublibrary,five_const,five_padding,design,three_padding,barcode,three_const
 *
 * Each row represents a single construct. Empty fields are allowed for
 * columns that haven't been filled yet (e.g., barcode before barcoding).
 */

#ifndef CSV_FORMAT_H
#define CSV_FORMAT_H

#include <string>
#include <vector>

namespace csv {

/**
 * Column indices for type-safe CSV field access.
 *
 * Use these constants instead of magic numbers when accessing
 * CSV fields to ensure code remains correct if the format changes.
 */
enum Column {
    INDEX = 0,          ///< 1-based index in original input file
    NAME = 1,           ///< Sequence identifier
    SUBLIBRARY = 2,     ///< Sublibrary grouping
    FIVE_CONST = 3,     ///< 5' constant/primer region
    FIVE_PADDING = 4,   ///< 5' padding hairpins
    DESIGN = 5,         ///< The design sequence of interest
    THREE_PADDING = 6,  ///< 3' padding hairpins
    BARCODE = 7,        ///< Barcode hairpin for identification
    THREE_CONST = 8,    ///< 3' constant/primer region
    BEGIN = 9,          ///< 1-based start position of design in full sequence
    END = 10,           ///< 1-based end position of design in full sequence
    COUNT = 11          ///< Total number of columns
};

/// Get the list of column names.
const std::vector<std::string>& columns();

/// Get the CSV header string.
const std::string& header();

/// Check if a header line matches the expected format.
/// Accepts both exact matches and extended headers (with extra columns appended).
bool is_valid_header(const std::string& line);

}  // namespace csv

#endif
