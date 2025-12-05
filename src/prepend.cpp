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
    _throw_if_not_exists(input_fasta);
    _remove_if_exists(output_fasta, overwrite);

    FastaOutputStream out(output_fasta);
    size_t count = 0;

    for_each_fasta(input_fasta, [&](const FastaEntry& entry) {
        out.write(entry.name, prefix + entry.sequence);
        count++;
    });

    std::cout << "Prepended '" << prefix << "' to " << count << " sequences.\n";
    std::cout << "Output: " << output_fasta << "\n";
}
