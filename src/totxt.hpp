#ifndef FLD_TOTXT_HPP
#define FLD_TOTXT_HPP

#include "library.hpp"
#include "utils.hpp"

class TxtArgs : public Program {
public:
    Arg<std::string> file;
    Arg<std::string> output;
    Arg<bool> overwrite;
    TxtArgs();
};

void _to_txt(
    const std::string& file,
    const std::string& output,
    bool overwrite
);

#endif
