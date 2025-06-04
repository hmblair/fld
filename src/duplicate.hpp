#ifndef DUPLICATE_HEADER
#define DUPLICATE_HEADER

#include <string>
#include <vector>
#include <fstream>
#include "nuc.hpp"
#include "utils.hpp"

class DuplicateArgs: public Program {
public:

    Arg<std::string> input;
    Arg<std::string> output;
    Arg<bool> overwrite;
    Arg<int> count;

    DuplicateArgs();

};

void _duplicate(
    const std::string& input,
    const std::string& output,
    bool overwrite,
    int count
);


#endif
