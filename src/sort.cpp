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
    descending(_parser, "--descending", "Sort in descending order (highest reads first)", false)
{
    _parser.add_description(
        "Sort a library CSV by predicted read counts.\n\n"
        "Default order is ascending (lowest reads first).\n"
        "A 'reads' column is appended to the output CSV."
    );
}

struct IndexedConstruct {
    size_t index;
    double reads;
    std::string line;
};

void _sort(
    const std::string& csv_file,
    const std::string& reads_file,
    const std::string& output_prefix,
    bool overwrite,
    bool descending
) {
    _throw_if_not_exists(csv_file);
    _throw_if_not_exists(reads_file);
    _remove_if_exists_all(output_prefix, overwrite);

    // Read all lines from CSV
    std::ifstream in(csv_file);
    std::string header_line;
    std::getline(in, header_line);

    if (!csv::is_valid_header(header_line)) {
        throw std::runtime_error("Invalid CSV header. Expected: " + csv::header());
    }

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

    // Create indexed pairs
    std::vector<IndexedConstruct> indexed;
    indexed.reserve(lines.size());
    for (size_t i = 0; i < lines.size(); i++) {
        indexed.push_back({i, reads[i], lines[i]});
    }

    // Sort by reads
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

    // Write sorted CSV
    std::string csv_out = output_prefix + ".csv";
    std::ofstream out_csv(csv_out);
    out_csv << header_line << ",reads\n";
    for (const auto& item : indexed) {
        out_csv << item.line << "," << item.reads << "\n";
    }
    out_csv.close();

    // Write sorted FASTA and TXT
    std::string fasta_out = output_prefix + ".fasta";
    std::string txt_out = output_prefix + ".txt";
    std::ofstream out_fasta(fasta_out);
    std::ofstream out_txt(txt_out);

    for (const auto& item : indexed) {
        auto fields = _split_by_delimiter(item.line, ',');
        if (fields.size() < csv::COUNT) {
            std::cerr << "Warning: skipping malformed CSV row with "
                      << fields.size() << " fields (expected " << csv::COUNT << ")\n";
            continue;
        }

        std::string name = fields[csv::NAME];
        std::string sublibrary = fields[csv::SUBLIBRARY];
        std::string seq = fields[csv::FIVE_CONST] + fields[csv::FIVE_PADDING] +
                          fields[csv::DESIGN] + fields[csv::THREE_PADDING] +
                          fields[csv::BARCODE] + fields[csv::THREE_CONST];

        out_fasta << ">" << name << " (" << sublibrary << ")\n" << seq << "\n";
        out_txt << seq << "\n";
    }

    out_fasta.close();
    out_txt.close();

    std::cout << "Sorted " << lines.size() << " sequences by read count.\n";
    std::cout << "Output: " << csv_out << ", " << fasta_out << ", " << txt_out << "\n";
}
