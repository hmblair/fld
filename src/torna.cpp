#include "torna.hpp"
#include "nuc.hpp"
#include "io/fasta_io.hpp"
#include <iostream>

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
    size_t count = transform_fasta(input_fasta, output_fasta, overwrite,
        [](const FastaEntry& entry) { return _to_rna(entry.sequence); });

    std::cout << "Converted " << count << " sequences to RNA.\n";
    std::cout << "Output: " << output_fasta << "\n";
}
