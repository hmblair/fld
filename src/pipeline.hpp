#ifndef PIPELINE_H
#define PIPELINE_H

#include "utils.hpp"
#include "config/stem_config.hpp"
#include <vector>

class PipelineArgs : public Program {
public:
    Arg<std::vector<std::string>> inputs;
    Arg<std::string> output;
    Arg<bool> overwrite;
    // Design options
    Arg<int> pad_to;
    Arg<std::string> five_const;
    Arg<std::string> three_const;
    // Stem options (shared between padding and barcodes)
    Arg<int> min_stem_length;
    Arg<int> max_stem_length;
    Arg<int> max_au;
    Arg<int> max_gc;
    Arg<int> max_gu;
    Arg<int> closing_gc;
    Arg<int> spacer;
    // Barcode options
    Arg<int> barcode_length;
    // Pipeline options
    Arg<bool> no_barcodes;
    Arg<bool> m2;
    // Prediction options
    Arg<bool> predict;
    Arg<int> batch_size;
    PipelineArgs();
};

struct PipelineConfig {
    std::vector<std::string> inputs;
    std::string output_dir;
    bool overwrite;
    int pad_to;
    std::string five_const;
    std::string three_const;
    StemConfig stem;
    int barcode_length;
    bool no_barcodes;
    bool generate_m2;
    // Prediction options
    bool predict;
    int batch_size;
};

void _pipeline(const PipelineConfig& config);

#endif
