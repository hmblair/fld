#ifndef SORT_H
#define SORT_H

#include "utils.hpp"

class SortArgs : public Program {
public:
    Arg<std::string> file;
    Arg<std::string> reads;
    Arg<std::string> output;
    Arg<bool> overwrite;
    Arg<bool> descending;
    SortArgs();
};

void _sort(
    const std::string& csv_file,
    const std::string& reads_file,
    const std::string& output_prefix,
    bool overwrite,
    bool descending
);

#endif
