#include "sort.hpp"
#include "library.hpp"
#include "io/csv_format.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

static inline std::string _PARSER_NAME = "sort";

SortArgs::SortArgs() : Program(_PARSER_NAME),
    file(_parser, "file", "Input CSV library file"),
    reads(_parser, "--reads", "Text file with read counts (one per line, same order as CSV)"),
    output(_parser, "-o", "Output prefix"),
    overwrite(_parser, "--overwrite", "Overwrite existing files", false),
    descending(_parser, "--descending", "Sort in descending order (highest reads first)", false),
    sort_by_reads(_parser, "--sort-by-reads", "Sort output by read counts (default: preserve input order)", false)
{
    _parser.add_description(
        "Sort a library CSV by predicted read counts.\n\n"
        "Default order preserves input order (by sublibrary and index).\n"
        "Use --sort-by-reads to sort by read counts instead.\n"
        "A 'reads' column is appended to the output CSV."
    );
}

struct IndexedConstruct {
    size_t original_index;  // 1-based index from CSV
    std::string sublibrary;
    double reads;
    std::string line;
};

void _sort(
    const std::string& csv_file,
    const std::string& reads_file,
    const std::string& output_prefix,
    bool overwrite,
    bool descending,
    bool sort_by_reads
) {
    _throw_if_not_exists(csv_file);
    _throw_if_not_exists(reads_file);
    _remove_if_exists_all(output_prefix, overwrite);

    // Read all lines from CSV
    std::ifstream in(csv_file);
    std::string header_line;
    std::getline(in, header_line);

    csv::Header header(header_line);
    header.validate();

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    in.close();

    // Load reads using shared utility
    std::vector<double> reads = _load_reads(reads_file, lines.size());

    // Create indexed pairs - extract original_index and sublibrary from CSV
    std::vector<IndexedConstruct> indexed;
    indexed.reserve(lines.size());
    for (size_t i = 0; i < lines.size(); i++) {
        auto fields = _split_by_delimiter(lines[i], ',');
        std::string idx_str = header.get(fields, csv::COL_INDEX, std::to_string(i + 1));
        size_t orig_idx = std::stoull(idx_str);
        std::string sublib = header.get(fields, csv::COL_SUBLIBRARY, "");
        indexed.push_back({orig_idx, sublib, reads[i], lines[i]});
    }

    // Sort by reads (internal operation always happens)
    if (descending) {
        std::sort(indexed.begin(), indexed.end(),
            [](const IndexedConstruct& a, const IndexedConstruct& b) {
                return a.reads > b.reads;
            });
    } else {
        std::sort(indexed.begin(), indexed.end(),
            [](const IndexedConstruct& a, const IndexedConstruct& b) {
                return a.reads < b.reads;
            });
    }

    // If not sorting by reads, re-sort by (sublibrary, original_index) for output
    if (!sort_by_reads) {
        std::sort(indexed.begin(), indexed.end(),
            [](const IndexedConstruct& a, const IndexedConstruct& b) {
                if (a.sublibrary != b.sublibrary) {
                    return a.sublibrary < b.sublibrary;
                }
                return a.original_index < b.original_index;
            });
    }

    // Write sorted CSV
    std::string csv_out = output_prefix + ".csv";
    std::ofstream out_csv(csv_out);
    out_csv << header_line << ",reads\n";
    for (const auto& item : indexed) {
        out_csv << item.line << "," << item.reads << "\n";
    }
    out_csv.close();

    // Write sorted FASTA
    std::string fasta_out = output_prefix + ".fasta";
    std::ofstream out_fasta(fasta_out);

    for (const auto& item : indexed) {
        auto fields = _split_by_delimiter(item.line, ',');

        // Get sequence columns (required)
        std::string seq = header.get(fields, csv::COL_FIVE_CONST) +
                          header.get(fields, csv::COL_FIVE_PADDING) +
                          header.get(fields, csv::COL_DESIGN) +
                          header.get(fields, csv::COL_THREE_PADDING) +
                          header.get(fields, csv::COL_BARCODE) +
                          header.get(fields, csv::COL_THREE_CONST);

        // Get optional metadata for FASTA header
        std::string name = header.get(fields, csv::COL_NAME, "sequence");
        std::string sublibrary = header.get(fields, csv::COL_SUBLIBRARY, "");

        if (sublibrary.empty()) {
            out_fasta << ">" << name << "\n" << seq << "\n";
        } else {
            out_fasta << ">" << name << " (" << sublibrary << ")\n" << seq << "\n";
        }
    }

    out_fasta.close();

    std::cout << "Sorted " << lines.size() << " sequences by read count.\n";
    std::cout << "Output: " << csv_out << ", " << fasta_out << "\n";
}
