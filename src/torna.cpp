#include "torna.hpp"
#include "io/fasta_io.hpp"
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

static inline std::string to_rna(std::string seq) {
    std::replace(seq.begin(), seq.end(), 'T', 'U');
    std::replace(seq.begin(), seq.end(), 't', 'u');
    return seq;
}

void _to_rna(
    const std::string& input_fasta,
    const std::string& output_fasta,
    bool overwrite
) {
    _throw_if_not_exists(input_fasta);
    _remove_if_exists(output_fasta, overwrite);

    FastaOutputStream out(output_fasta);
    size_t count = 0;

    for_each_fasta(input_fasta, [&](const FastaEntry& entry) {
        out.write(entry.name, to_rna(entry.sequence));
        count++;
    });

    std::cout << "Converted " << count << " sequences to RNA.\n";
    std::cout << "Output: " << output_fasta << "\n";
}
