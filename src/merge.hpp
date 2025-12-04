#ifndef MERGE_H
#define MERGE_H

#include "utils.hpp"

class MergeArgs : public Program {
public:
    Arg<std::string> library;
    Arg<std::string> library_reads;
    Arg<std::string> barcodes;
    Arg<std::string> barcode_reads;
    Arg<std::string> output;
    Arg<bool> overwrite;
    MergeArgs();
};

void _merge(
    const std::string& library_csv,
    const std::string& library_reads_file,
    const std::string& barcodes_file,
    const std::string& barcode_reads_file,
    const std::string& output_prefix,
    bool overwrite
);

#endif
