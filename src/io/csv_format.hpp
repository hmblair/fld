#ifndef CSV_FORMAT_H
#define CSV_FORMAT_H

#include <string>
#include <vector>

namespace csv {

// Column indices for type-safe access
enum Column {
    NAME = 0,
    SUBLIBRARY = 1,
    FIVE_CONST = 2,
    FIVE_PADDING = 3,
    DESIGN = 4,
    THREE_PADDING = 5,
    BARCODE = 6,
    THREE_CONST = 7,
    COUNT = 8  // Total number of columns
};

// Get the column names
const std::vector<std::string>& columns();

// Get the CSV header string
const std::string& header();

// Check if a header line matches the expected format
bool is_valid_header(const std::string& line);

}  // namespace csv

#endif
