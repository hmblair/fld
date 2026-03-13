#ifndef PADDING_H
#define PADDING_H

#include "config/stem_config.hpp"
#include <string>

// Generate padding sequences to a text file (one per line, in CSV row order).
// Reads the library CSV to determine design lengths, computes padding_length
// = pad_to - design_length for each row, and generates padding sequences.
void _generate_padding(
    const std::string& library_csv,
    size_t pad_to,
    const StemConfig& config,
    const std::string& output_file,
    bool overwrite
);

#endif
