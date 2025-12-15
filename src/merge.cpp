#include "merge.hpp"
#include "library.hpp"
#include "io/csv_format.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

static inline std::string _PARSER_NAME = "merge";

MergeArgs::MergeArgs() : Program(_PARSER_NAME),
    library(_parser, "--library", "Input CSV library file (without barcodes)"),
    library_reads(_parser, "--library-reads", "Text file with predicted read counts (one per line)"),
    barcodes(_parser, "--barcodes", "Text file with barcode sequences (one per line)"),
    barcode_reads(_parser, "--barcode-reads", "Text file with predicted barcode read counts"),
    output(_parser, "-o", "Output prefix"),
    overwrite(_parser, "--overwrite", "Overwrite existing files", false),
    sort_by_reads(_parser, "--sort-by-reads", "Sort output by read counts (default: preserve input order)", false)
{
    _parser.add_description(
        "Merge barcodes into library using read-count balancing.\n\n"
        "Pairs low-read designs with high-read barcodes to balance coverage.\n"
        "Read counts should be predicted using a tool like rn-coverage.\n"
        "Default output order preserves input order (by sublibrary and index).\n"
        "Use --sort-by-reads to sort by read counts instead."
    );
}

struct LibraryEntry {
    size_t original_index = 0;  // 1-based index from CSV
    std::string sublibrary;
    double reads = 0.0;
    std::vector<std::string> fields;
};

struct BarcodeEntry {
    size_t original_index = 0;
    double reads = 0.0;
    std::string barcode;
};

struct MergedEntry {
    size_t original_index = 0;  // 1-based index from CSV
    std::string sublibrary;
    std::vector<std::string> fields;
    std::string barcode;
    double design_reads = 0.0;
    double barcode_reads = 0.0;
};

void _merge(
    const std::string& library_csv,
    const std::string& library_reads_file,
    const std::string& barcodes_file,
    const std::string& barcode_reads_file,
    const std::string& output_prefix,
    bool overwrite,
    bool sort_by_reads
) {
    _throw_if_not_exists(library_csv);
    _throw_if_not_exists(library_reads_file);
    _throw_if_not_exists(barcodes_file);
    _throw_if_not_exists(barcode_reads_file);
    _remove_if_exists_all(output_prefix, overwrite);

    // Load library CSV
    std::ifstream in(library_csv);
    std::string header_line;
    std::getline(in, header_line);

    csv::Header header(header_line);
    header.validate();

    std::vector<LibraryEntry> library_entries;
    std::string line;
    size_t row_num = 1;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        LibraryEntry entry;
        entry.fields = _split_by_delimiter(line, ',');
        // Parse original_index and sublibrary from CSV (with defaults)
        std::string idx_str = header.get(entry.fields, csv::COL_INDEX, std::to_string(row_num));
        entry.original_index = std::stoull(idx_str);
        entry.sublibrary = header.get(entry.fields, csv::COL_SUBLIBRARY, "");
        library_entries.push_back(entry);
        row_num++;
    }
    in.close();

    // Load library reads using shared utility
    std::vector<double> lib_reads = _load_reads(library_reads_file, library_entries.size());
    for (size_t i = 0; i < library_entries.size(); i++) {
        library_entries[i].reads = lib_reads[i];
    }

    // Load barcodes using shared utility
    std::vector<std::string> barcodes = _load_lines(barcodes_file);
    if (barcodes.size() < library_entries.size()) {
        throw std::runtime_error("Not enough barcodes (" + std::to_string(barcodes.size()) +
            ") for library entries (" + std::to_string(library_entries.size()) + ")");
    }

    // Load barcode reads
    std::vector<double> bc_reads = _load_reads(barcode_reads_file, barcodes.size());

    std::vector<BarcodeEntry> barcode_entries;
    for (size_t i = 0; i < barcodes.size(); i++) {
        BarcodeEntry entry;
        entry.original_index = i;
        entry.reads = bc_reads[i];
        entry.barcode = barcodes[i];
        barcode_entries.push_back(entry);
    }

    // Sort library by reads ASCENDING (low reads first)
    // This enables pairing low-read designs with high-read barcodes
    std::sort(library_entries.begin(), library_entries.end(),
        [](const LibraryEntry& a, const LibraryEntry& b) {
            return a.reads < b.reads;
        });

    // Sort barcodes by reads DESCENDING (high reads first)
    std::sort(barcode_entries.begin(), barcode_entries.end(),
        [](const BarcodeEntry& a, const BarcodeEntry& b) {
            return a.reads > b.reads;
        });

    // Merge: pair low-read designs with high-read barcodes to balance coverage
    std::vector<MergedEntry> merged;
    for (size_t i = 0; i < library_entries.size(); i++) {
        MergedEntry entry;
        entry.original_index = library_entries[i].original_index;
        entry.sublibrary = library_entries[i].sublibrary;
        entry.fields = library_entries[i].fields;
        entry.barcode = barcode_entries[i].barcode;
        entry.design_reads = library_entries[i].reads;
        entry.barcode_reads = barcode_entries[i].reads;

        // Replace the barcode field
        int bc_idx = header.index_of(csv::COL_BARCODE);
        if (bc_idx >= 0 && static_cast<size_t>(bc_idx) < entry.fields.size()) {
            entry.fields[bc_idx] = entry.barcode;
        }

        merged.push_back(entry);
    }

    // If not sorting by reads, re-sort by (sublibrary, original_index) for output
    if (!sort_by_reads) {
        std::sort(merged.begin(), merged.end(),
            [](const MergedEntry& a, const MergedEntry& b) {
                if (a.sublibrary != b.sublibrary) {
                    return a.sublibrary < b.sublibrary;
                }
                return a.original_index < b.original_index;
            });
    }

    // Write output CSV with extra columns
    std::string csv_out = output_prefix + ".csv";
    std::ofstream out_csv(csv_out);
    out_csv << header_line << ",design_reads,barcode_reads\n";
    for (const auto& entry : merged) {
        for (size_t i = 0; i < entry.fields.size(); i++) {
            out_csv << _quote_csv_field(entry.fields[i]);
            if (i < entry.fields.size() - 1) out_csv << ",";
        }
        out_csv << "," << entry.design_reads
                << "," << entry.barcode_reads << "\n";
    }
    out_csv.close();

    // Write FASTA
    std::string fasta_out = output_prefix + ".fasta";
    std::ofstream out_fasta(fasta_out);

    for (const auto& entry : merged) {
        // Get sequence columns (required)
        std::string seq = header.get(entry.fields, csv::COL_FIVE_CONST) +
                          header.get(entry.fields, csv::COL_FIVE_PADDING) +
                          header.get(entry.fields, csv::COL_DESIGN) +
                          header.get(entry.fields, csv::COL_THREE_PADDING) +
                          header.get(entry.fields, csv::COL_BARCODE) +
                          header.get(entry.fields, csv::COL_THREE_CONST);

        // Get optional metadata for FASTA header
        std::string name = header.get(entry.fields, csv::COL_NAME, "sequence");
        std::string sublibrary = header.get(entry.fields, csv::COL_SUBLIBRARY, "");

        if (sublibrary.empty()) {
            out_fasta << ">" << name << "\n" << seq << "\n";
        } else {
            out_fasta << ">" << name << " (" << sublibrary << ")\n" << seq << "\n";
        }
    }

    out_fasta.close();

    std::cout << "Merged " << merged.size() << " library entries with barcodes.\n";
    std::cout << "Output: " << csv_out << ", " << fasta_out << "\n";
}
