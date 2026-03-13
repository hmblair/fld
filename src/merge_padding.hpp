#ifndef MERGE_PADDING_H
#define MERGE_PADDING_H

#include "utils.hpp"
#include <string>

// Merge padding into library using inverse read-count balancing.
// Groups designs by design length and, within each group, pairs
// low-read designs with high-read padding sequences.
void _merge_padding(
    const std::string& library_csv,
    const std::string& library_reads_file,
    const std::string& padding_file,
    const std::string& padding_reads_file,
    const std::string& output_prefix,
    bool overwrite
);

#endif
