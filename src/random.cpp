#include "random.hpp"


void _random(
    const std::string& output,
    bool overwrite,
    int count,
    int length,
    bool fasta
) {

    _remove_if_exists(output, overwrite);

    std::mt19937 gen = _init_gen();
    std::ofstream file(output);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open output file: " + output);
    }

    if (fasta) {

        for (int32_t ix = 0; ix < count; ++ix) {
            std::string sequence = _random_sequence(length, gen);
            std::string name = ">random_" + std::to_string(ix);
            file << name << "\n" << sequence << "\n";
        }

    } else {

        for (int32_t ix = 0; ix < count; ++ix) {
            std::string sequence = _random_sequence(length, gen);
            file << sequence << "\n";
        }

    }

}


//
// Argument parsing
//


static inline std::string _PARSER_NAME = "random";

static inline std::string _COUNT_NAME = "--count";
static inline std::string _COUNT_HELP = "The number of sequences to generate.";

static inline std::string _OUTPUT_NAME = "--output";
static inline std::string _OUTPUT_HELP = "The output file.";

static inline std::string _BARCODE_LENGTH_NAME = "--length";
static inline std::string _BARCODE_LENGTH_HELP = "The length of each output sequence.";

static inline std::string _OVERWRITE_NAME = "--overwrite";
static inline std::string _OVERWRITE_HELP = "Overwrite any existing file.";

static inline std::string _FASTA_NAME = "--fasta";
static inline std::string _FASTA_HELP = "Output in FASTA format.";


RandomArgs::RandomArgs() :
    Program(_PARSER_NAME),
    output(_parser, _OUTPUT_NAME, _OUTPUT_HELP),
    overwrite(_parser, _OVERWRITE_NAME, _OVERWRITE_HELP),
    count(_parser, _COUNT_NAME, _COUNT_HELP),
    length(_parser, _BARCODE_LENGTH_NAME, _BARCODE_LENGTH_HELP),
    fasta(_parser, _FASTA_NAME, _FASTA_HELP) {
}
