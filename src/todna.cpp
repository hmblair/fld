#include "todna.hpp"
#include "nuc.hpp"
#include "io/fasta_io.hpp"
#include <iostream>

static inline std::string _PARSER_NAME = "to-dna";

ToDnaArgs::ToDnaArgs() : Program(_PARSER_NAME),
    file(_parser, "file", "Input FASTA file"),
    output(_parser, "-o", "Output FASTA file"),
    overwrite(_parser, "--overwrite", "Overwrite existing output file", false)
{
    _parser.add_description("Convert RNA sequences to DNA (U -> T).");
}

void _to_dna(
    const std::string& input_fasta,
    const std::string& output_fasta,
    bool overwrite
) {
    _throw_if_not_exists(input_fasta);
    _remove_if_exists(output_fasta, overwrite);

    FastaOutputStream out(output_fasta);
    size_t count = 0;

    for_each_fasta(input_fasta, [&](const FastaEntry& entry) {
        out.write(entry.name, _to_dna(entry.sequence));
        count++;
    });

    std::cout << "Converted " << count << " sequences to DNA.\n";
    std::cout << "Output: " << output_fasta << "\n";
}
