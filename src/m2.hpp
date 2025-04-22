#ifndef M2_HEADER
#define M2_HEADER

#include <string>
#include <vector>
#include <fstream>
#include "nuc.hpp"
#include "utils.hpp"

class M2Args: public Program {
public:

    Arg<std::string> input;
    Arg<std::string> output;
    Arg<bool> all;
    Arg<bool> overwrite;

    M2Args();

};

void _m2(
    const std::string& input,
    const std::string& output,
    bool all,
    bool overwrite
);


#endif
