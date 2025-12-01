#include "library.hpp"
#include "utils.hpp"

class txtArgs : public Program {
public:
    Arg<std::string> file;
    Arg<std::string> output;
    Arg<bool> overwrite;
    txtArgs();
};

void _to_txt(
    const std::string& file,
    const std::string& output,
    bool overwrite
);
