#ifndef PREPEND_H
#define PREPEND_H

#include "utils.hpp"

class PrependArgs : public Program {
public:
    Arg<std::string> file;
    Arg<std::string> output;
    Arg<std::string> sequence;
    Arg<bool> overwrite;
    PrependArgs();
};

void _prepend(
    const std::string& input_fasta,
    const std::string& output_fasta,
    const std::string& prefix,
    bool overwrite
);

#endif
