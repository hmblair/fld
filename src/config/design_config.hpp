#ifndef DESIGN_CONFIG_H
#define DESIGN_CONFIG_H

#include "stem_config.hpp"
#include "barcode_config.hpp"
#include <string>

struct DesignConfig {
    // I/O
    std::string input_path;
    std::string output_prefix;
    bool overwrite = false;

    // Padding
    size_t pad_to_length = 0;

    // Barcoding
    BarcodeConfig barcode;

    // Stem configuration for padding
    StemConfig stem;

    // Constant regions
    std::string five_const = "ACTCGAGTAGAGTCGAAAA";
    std::string three_const = "AAAAGAAACAACAACAACAAC";

    void validate() const;
    void validate_with_library_size(size_t library_size) const;
};

#endif
