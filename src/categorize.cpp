#include "categorize.hpp"
#include "io/fasta_io.hpp"
#include <iostream>
#include <map>
#include <algorithm>
#include <filesystem>

static inline std::string _PARSER_NAME = "categorize";

CategorizeArgs::CategorizeArgs() : Program(_PARSER_NAME),
    file(_parser, "file", "Input FASTA file"),
    output(_parser, "-o", "Output directory", "."),
    overwrite(_parser, "--overwrite", "Overwrite existing files", false),
    bins(_parser, "--bins", "Length bins (sequences go to smallest bin they fit in)", std::vector<int>{130, 240, 500, 2000})
{
    _parser.add_description("Split a FASTA file by sequence length into bins.");
}

static size_t _find_bin(size_t length, const std::vector<int>& bins) {
    for (size_t i = 0; i < bins.size(); i++) {
        if (length <= static_cast<size_t>(bins[i])) {
            return i;
        }
    }
    return bins.size() - 1;  // Put in largest bin if doesn't fit
}

static inline std::string to_dna(const std::string& seq) {
    std::string result = seq;
    for (char& c : result) {
        if (c == 'U') c = 'T';
        if (c == 'u') c = 't';
    }
    return result;
}

void _categorize(
    const std::string& input,
    const std::string& output_dir,
    bool overwrite,
    const std::vector<int>& bins
) {
    _throw_if_not_exists(input);

    // Create output directory
    std::filesystem::create_directories(output_dir);

    // Sort bins for consistent ordering
    std::vector<int> sorted_bins = bins;
    std::sort(sorted_bins.begin(), sorted_bins.end());

    // Open output files for each bin
    std::map<size_t, std::ofstream> out_files;
    std::map<size_t, size_t> counts;

    for (size_t i = 0; i < sorted_bins.size(); i++) {
        std::string filename = output_dir + "/" + std::to_string(sorted_bins[i]) + ".fasta";
        if (!overwrite) {
            _throw_if_exists(filename);
        }
        out_files[i].open(filename);
        counts[i] = 0;
    }

    // Read input FASTA and categorize
    for_each_fasta(input, [&](const FastaEntry& entry) {
        size_t bin_idx = _find_bin(entry.sequence.length(), sorted_bins);
        out_files[bin_idx] << ">" << entry.name << "\n" << to_dna(entry.sequence) << "\n";
        counts[bin_idx]++;
    });

    // Close files and report
    for (auto& [idx, file] : out_files) {
        file.close();
    }

    std::cout << "Categorized sequences:\n";
    for (size_t i = 0; i < sorted_bins.size(); i++) {
        std::cout << "  <= " << sorted_bins[i] << "nt: " << counts[i] << " sequences\n";
    }
}
