/**
 * @file csv_format.hpp
 * @brief CSV format definitions for library files.
 *
 * Defines the column structure for library CSV files:
 *
 *   name,sublibrary,five_const,five_padding,design,three_padding,barcode,three_const
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
    NAME = 0,           ///< Sequence identifier
    SUBLIBRARY = 1,     ///< Sublibrary grouping
    FIVE_CONST = 2,     ///< 5' constant/primer region
    FIVE_PADDING = 3,   ///< 5' padding hairpins
    DESIGN = 4,         ///< The design sequence of interest
    THREE_PADDING = 5,  ///< 3' padding hairpins
    BARCODE = 6,        ///< Barcode hairpin for identification
    THREE_CONST = 7,    ///< 3' constant/primer region
    COUNT = 8           ///< Total number of columns
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
