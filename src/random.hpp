#ifndef RANDOM_HEADER
#define RANDOM_HEADER

#include <string>
#include <vector>
#include <fstream>
#include "nuc.hpp"
#include "utils.hpp"

class RandomArgs: public Program {
public:

    Arg<std::string> output;
    Arg<bool> overwrite;
    Arg<int> count;
    Arg<int> length;
    Arg<bool> fasta;

    RandomArgs();

};

void _random(
    const std::string& output,
    bool overwrite,
    int count,
    int length,
    bool fasta
);


#endif
