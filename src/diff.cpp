#include "diff.hpp"
#include "io/fasta_io.hpp"
#include <iostream>

static inline std::string _PARSER_NAME = "diff";

DiffArgs::DiffArgs() : Program(_PARSER_NAME),
    file1(_parser, "file1", "First FASTA file"),
    file2(_parser, "file2", "Second FASTA file")
{
    _parser.add_description("List sequence indices where two FASTA files differ.");
}

bool _diff(
    const std::string& file1,
    const std::string& file2
) {
    _throw_if_not_exists(file1);
    _throw_if_not_exists(file2);

    auto seqs1 = read_fasta(file1);
    auto seqs2 = read_fasta(file2);

    std::vector<size_t> diff_indices;
    size_t max_size = std::max(seqs1.size(), seqs2.size());

    for (size_t i = 0; i < max_size; i++) {
        bool differs = false;
        if (i >= seqs1.size() || i >= seqs2.size()) {
            differs = true;
        } else if (seqs1[i].sequence != seqs2[i].sequence) {
            differs = true;
        }
        if (differs) {
            diff_indices.push_back(i + 1);  // 1-indexed
        }
    }

    if (diff_indices.empty()) {
        std::cout << "Files are identical (" << seqs1.size() << " sequences)\n";
        return true;
    }

    std::cout << diff_indices.size() << " difference(s) at "
              << (diff_indices.size() == 1 ? "index" : "indices") << ":";
    for (size_t idx : diff_indices) {
        std::cout << " " << idx;
    }
    std::cout << "\n";

    if (seqs1.size() != seqs2.size()) {
        std::cout << "  (file1: " << seqs1.size() << " sequences, file2: " << seqs2.size() << " sequences)\n";
    }

    return false;
}
