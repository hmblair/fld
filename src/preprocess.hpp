#ifndef PREPROCESS_H
#define PREPROCESS_H

#include "utils.hpp"
#include "library.hpp"
#include <fstream>

void _preprocess(
    const std::string& fasta,
    const std::string& csv,
    bool overwrite,
    const std::string& sublibrary
);

#endif
