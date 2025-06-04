#include "duplicate.hpp"

void _duplicate(
    const std::string& input,
    const std::string& output,
    bool overwrite,
    int count
) {

    _remove_if_exists(output, overwrite);
    std::ifstream infile(input);
    std::ofstream outfile(output);

    std::string line;
    std::string sequence;
    std::string header;

    while (std::getline(infile, line)) {

        if (_is_fasta_header(line)) {
            header = line;
            if (!sequence.empty()) {
                for (int32_t ix = 0; ix < count; ix++) {
                    outfile << header << "_" << ix << "\n";
                    outfile << sequence << "\n";
                }
                sequence.clear();
            }
        } else {
            sequence += line;
        }

    }

    if (!sequence.empty()) {
        for (int32_t ix = 0; ix < count; ix++) {
            outfile << header << "_" << ix << "\n";
            outfile << sequence << "\n";
        }
    }

}



//
// Argument parsing
//


static inline std::string _PARSER_NAME = "duplicate";

static inline std::string _INPUT_NAME = "input";
static inline std::string _INPUT_HELP = "The input FASTA file.";

static inline std::string _OUTPUT_NAME = "--output";
static inline std::string _OUTPUT_HELP = "The output FASTA file.";

static inline std::string _COUNT_NAME = "--count";
static inline std::string _COUNT_HELP = "The number of times to duplicate each sequence.";

static inline std::string _OVERWRITE_NAME = "--overwrite";
static inline std::string _OVERWRITE_HELP = "Overwrite any existing file.";


DuplicateArgs::DuplicateArgs() :
    Program(_PARSER_NAME),
    input(_parser, _INPUT_NAME, _INPUT_HELP),
    output(_parser, _OUTPUT_NAME, _OUTPUT_HELP),
    overwrite(_parser, _OVERWRITE_NAME, _OVERWRITE_HELP),
    count(_parser, _COUNT_NAME, _COUNT_HELP) {}
