#include "prepend.hpp"
#include <fstream>
#include <iostream>

static inline std::string _PARSER_NAME = "prepend";

PrependArgs::PrependArgs() : Program(_PARSER_NAME),
    file(_parser, "file", "Input FASTA file"),
    output(_parser, "-o", "Output FASTA file"),
    sequence(_parser, "--sequence", "Sequence to prepend to each entry"),
    overwrite(_parser, "--overwrite", "Overwrite existing output file", false)
{
    _parser.add_description("Prepend a sequence to all entries in a FASTA file.");
}

void _prepend(
    const std::string& input_fasta,
    const std::string& output_fasta,
    const std::string& prefix,
    bool overwrite
) {
    _throw_if_not_exists(input_fasta);
    _remove_if_exists(output_fasta, overwrite);

    std::ifstream in(input_fasta);
    std::ofstream out(output_fasta);

    std::string line;
    size_t count = 0;

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        if (line[0] == '>') {
            // Header line - pass through
            out << line << "\n";
        } else {
            // Sequence line - prepend
            out << prefix << line << "\n";
            count++;
        }
    }

    std::cout << "Prepended '" << prefix << "' to " << count << " sequences.\n";
    std::cout << "Output: " << output_fasta << "\n";
}
