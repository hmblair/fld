#include "categorize.hpp"
#include "nuc.hpp"
#include <fstream>
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
    std::ifstream in(input);
    std::string line;
    std::string current_header;
    std::string current_seq;

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        if (_is_fasta_header(line)) {
            // Write previous sequence if exists
            if (!current_header.empty() && !current_seq.empty()) {
                size_t bin_idx = _find_bin(current_seq.length(), sorted_bins);
                // Convert U to T
                std::string dna_seq = current_seq;
                for (char& c : dna_seq) {
                    if (c == 'U' || c == 'u') c = 'T';
                }
                out_files[bin_idx] << current_header << "\n" << dna_seq << "\n";
                counts[bin_idx]++;
            }
            current_header = line;
            current_seq.clear();
        } else {
            current_seq += line;
        }
    }

    // Write last sequence
    if (!current_header.empty() && !current_seq.empty()) {
        size_t bin_idx = _find_bin(current_seq.length(), sorted_bins);
        std::string dna_seq = current_seq;
        for (char& c : dna_seq) {
            if (c == 'U' || c == 'u') c = 'T';
        }
        out_files[bin_idx] << current_header << "\n" << dna_seq << "\n";
        counts[bin_idx]++;
    }

    // Close files and report
    for (auto& [idx, file] : out_files) {
        file.close();
    }

    std::cout << "Categorized sequences:\n";
    for (size_t i = 0; i < sorted_bins.size(); i++) {
        std::cout << "  <= " << sorted_bins[i] << "nt: " << counts[i] << " sequences\n";
    }
}
