#ifndef CATEGORIZE_H
#define CATEGORIZE_H

#include "utils.hpp"
#include <vector>

class CategorizeArgs : public Program {
public:
    Arg<std::string> file;
    Arg<std::string> output;
    Arg<bool> overwrite;
    Arg<std::vector<int>> bins;
    CategorizeArgs();
};

void _categorize(
    const std::string& input,
    const std::string& output_dir,
    bool overwrite,
    const std::vector<int>& bins
);

#endif
