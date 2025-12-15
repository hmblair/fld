/**
 * @file csv_format.hpp
 * @brief Flexible CSV format definitions for library files.
 *
 * Supports flexible column ordering. Required columns (sequence data):
 *   five_const, five_padding, design, three_padding, barcode, three_const
 *
 * Optional metadata columns:
 *   index, name, sublibrary, begin, end, reads
 *
 * When writing output, all standard columns are included.
 */

#ifndef CSV_FORMAT_H
#define CSV_FORMAT_H

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

namespace csv {

// Column name constants
constexpr const char* COL_INDEX = "index";
constexpr const char* COL_NAME = "name";
constexpr const char* COL_SUBLIBRARY = "sublibrary";
constexpr const char* COL_FIVE_CONST = "five_const";
constexpr const char* COL_FIVE_PADDING = "five_padding";
constexpr const char* COL_DESIGN = "design";
constexpr const char* COL_THREE_PADDING = "three_padding";
constexpr const char* COL_BARCODE = "barcode";
constexpr const char* COL_THREE_CONST = "three_const";
constexpr const char* COL_BEGIN = "begin";
constexpr const char* COL_END = "end";
constexpr const char* COL_READS = "reads";

/// Get the list of required column names (sequence data).
const std::vector<std::string>& required_columns();

/// Get the list of all standard column names (for output).
const std::vector<std::string>& columns();

/// Get the standard CSV header string (for output).
const std::string& header();

/**
 * @brief Parsed CSV header with dynamic column lookup.
 *
 * Parses a header line and provides index lookup for any column.
 * Validates that all required columns are present.
 */
class Header {
public:
    /// Parse a header line
    explicit Header(const std::string& line);

    /// Check if a column exists in this header
    bool has(const std::string& col) const;

    /// Get column index, or -1 if not present
    int index_of(const std::string& col) const;

    /// Get field from a parsed row, or default if column not present
    std::string get(const std::vector<std::string>& fields, const std::string& col,
                    const std::string& default_value = "") const;

    /// Get the original header string
    const std::string& str() const { return _header; }

    /// Get all column names in order
    const std::vector<std::string>& column_names() const { return _columns; }

    /// Number of columns
    size_t size() const { return _columns.size(); }

    /// Validate that all required columns are present
    /// Throws std::runtime_error if validation fails
    void validate() const;

private:
    std::string _header;
    std::vector<std::string> _columns;
    std::unordered_map<std::string, int> _col_indices;
};

// Legacy support - column indices for standard format
// Use Header class for flexible parsing instead
enum Column {
    INDEX = 0,
    NAME = 1,
    SUBLIBRARY = 2,
    FIVE_CONST = 3,
    FIVE_PADDING = 4,
    DESIGN = 5,
    THREE_PADDING = 6,
    BARCODE = 7,
    THREE_CONST = 8,
    BEGIN = 9,
    END = 10,
    COUNT = 11
};

/// Check if a header line is valid (has all required columns).
/// @deprecated Use Header class and validate() instead.
bool is_valid_header(const std::string& line);

}  // namespace csv

#endif
