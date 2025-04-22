#ifndef INSPECT_H
#define INSPECT_H

#include "utils.hpp"
#include <unordered_map>
#include <fstream>

class InspectArgs : public Program {
private:
public:
    Arg<std::string> file;
    Arg<bool> sort;
    InspectArgs();
};

void _inspect(const std::string& filename, bool sort);

#endif
