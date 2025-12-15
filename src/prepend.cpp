#include "prepend.hpp"
#include "io/fasta_io.hpp"
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
    size_t count = transform_fasta(input_fasta, output_fasta, overwrite,
        [&prefix](const FastaEntry& entry) { return prefix + entry.sequence; });

    std::cout << "Prepended '" << prefix << "' to " << count << " sequences.\n";
    std::cout << "Output: " << output_fasta << "\n";
}
