#ifndef PREPROCESS_H
#define PREPROCESS_H

#include "utils.hpp"
#include "library.hpp"
#include <fstream>

class PreprocessArgs : public Program{
public:
    Arg<std::string> file;
    Arg<std::string> output;
    Arg<bool> overwrite;
    Arg<std::string> sublibrary;
    PreprocessArgs();
};

void _preprocess(
    const std::string& fasta,
    const std::string& csv,
    bool overwrite,
    const std::string& sublibrary
);

#endif
