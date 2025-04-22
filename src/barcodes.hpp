#ifndef BARCODE_H
#define BARCODE_H

#include "nuc.hpp"
#include "utils.hpp"
#include <fstream>
#include <unordered_set>
#include <indicators/block_progress_bar.hpp>



//
// Get a random barcode which is not a hamming neighbour of any
// of the existing barcodes
//



std::string _random_barcode(
    size_t stem_length,
    std::mt19937 &gen,
    size_t max_au,
    size_t max_gc,
    size_t max_gu,
    size_t closing_gc,
    const std::unordered_set<std::string>& existing
);



//
// Insert the barcode if it is not a hamming neighbour of any of the existing barcodes.
// Return false if not inserted.
//



bool _insert_if_not_neighbour(
    const std::string& sequence,
    std::unordered_set<std::string>& barcodes
);



//
// Insert the desired number of barcodes into the input unordered_set
//



void _get_barcodes(
    size_t count,
    size_t stem_length,
    std::mt19937 &gen,
    size_t max_au,
    size_t max_gc,
    size_t max_gu,
    size_t closing_gc,
    std::unordered_set<std::string>& barcodes
);



//
// Get the desired number of barcodes and write them to a text file
//



void _barcodes(
    size_t count,
    const std::string& output,
    bool overwrite,
    size_t stem_length,
    size_t max_stem_au,
    size_t max_stem_gc,
    size_t max_stem_gu,
    size_t closing_gc
);



//
// Arguments to the _barcodes program
//



class BarcodesArgs : public Program {
private:
public:
    Arg<int> count;
    Arg<std::string> output;
    Arg<bool> overwrite;
    Arg<int> stem_length;
    Arg<int> max_au;
    Arg<int> max_gc;
    Arg<int> max_gu;
    Arg<int> closing_gc;
    BarcodesArgs();
};

#endif



