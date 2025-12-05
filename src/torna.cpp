#include "torna.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

static inline std::string _PARSER_NAME = "to-rna";

ToRnaArgs::ToRnaArgs() : Program(_PARSER_NAME),
    file(_parser, "file", "Input FASTA file"),
    output(_parser, "-o", "Output FASTA file"),
    overwrite(_parser, "--overwrite", "Overwrite existing output file", false)
{
    _parser.add_description("Convert DNA sequences to RNA (T -> U).");
}

void _to_rna(
    const std::string& input_fasta,
    const std::string& output_fasta,
    bool overwrite
) {
    _throw_if_not_exists(input_fasta);
    _remove_if_exists(output_fasta, overwrite);

    std::ifstream in(input_fasta);
    std::ofstream out(output_fasta);

    std::string line;
    size_t count = 0;

    while (std::getline(in, line)) {
        if (line.empty()) {
            out << "\n";
            continue;
        }

        if (line[0] == '>') {
            // Header line - pass through
            out << line << "\n";
        } else {
            // Sequence line - convert T to U
            std::replace(line.begin(), line.end(), 'T', 'U');
            std::replace(line.begin(), line.end(), 't', 'u');
            out << line << "\n";
            count++;
        }
    }

    std::cout << "Converted " << count << " sequences to RNA.\n";
    std::cout << "Output: " << output_fasta << "\n";
}
