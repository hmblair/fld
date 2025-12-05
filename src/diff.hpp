#ifndef DIFF_H
#define DIFF_H

#include "utils.hpp"

class DiffArgs : public Program {
public:
    Arg<std::string> file1;
    Arg<std::string> file2;
    DiffArgs();
};

// Returns true if files are identical, false if they differ
bool _diff(
    const std::string& file1,
    const std::string& file2
);

#endif
