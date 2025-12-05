#ifndef TORNA_H
#define TORNA_H

#include "utils.hpp"

class ToRnaArgs : public Program {
public:
    Arg<std::string> file;
    Arg<std::string> output;
    Arg<bool> overwrite;
    ToRnaArgs();
};

void _to_rna(
    const std::string& input_fasta,
    const std::string& output_fasta,
    bool overwrite
);

#endif
