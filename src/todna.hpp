#ifndef TODNA_H
#define TODNA_H

#include "utils.hpp"

class ToDnaArgs : public Program {
public:
    Arg<std::string> file;
    Arg<std::string> output;
    Arg<bool> overwrite;
    ToDnaArgs();
};

void _to_dna(
    const std::string& input_fasta,
    const std::string& output_fasta,
    bool overwrite
);

#endif
