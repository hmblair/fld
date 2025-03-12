#ifndef FOLD_H
#define FOLD_H

#include <stdexcept>
#include <string>
#include <vector>
extern "C" {
    #include <ViennaRNA/fold.h>
    #include <ViennaRNA/part_func.h>
    #include <ViennaRNA/utils.h>
    #include <ViennaRNA/constraints.h>
}

double _score_unpaired(
    const std::string& sequence,
    const std::string& unpaired
);
double _score_hairpin(
    const std::string& sequence,
    const std::string& hairpin,
    size_t loop_length
);

#endif
