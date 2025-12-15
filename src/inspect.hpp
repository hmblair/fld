#ifndef INSPECT_H
#define INSPECT_H

#include "utils.hpp"

class InspectArgs : public Program {
public:
    Arg<std::vector<std::string>> files;
    Arg<bool> sort;
    InspectArgs();
};

void _inspect(const std::vector<std::string>& filename, bool sort);

#endif
