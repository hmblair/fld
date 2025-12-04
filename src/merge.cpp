#include "merge.hpp"
#include "library.hpp"
#include "io/csv_format.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

static inline std::string _PARSER_NAME = "merge";

MergeArgs::MergeArgs() : Program(_PARSER_NAME),
    library(_parser, "--library", "Input CSV library file (without barcodes)"),
    library_reads(_parser, "--library-reads", "Text file with library read counts"),
    barcodes(_parser, "--barcodes", "Text file with barcodes (one per line)"),
    barcode_reads(_parser, "--barcode-reads", "Text file with barcode read counts"),
    output(_parser, "-o", "Output prefix"),
    overwrite(_parser, "--overwrite", "Overwrite existing files", false)
{
    _parser.add_description("Merge barcodes into library using read-count balancing.");
}

struct LibraryEntry {
    size_t original_index;
    double reads;
    std::vector<std::string> fields;
};

struct BarcodeEntry {
    size_t original_index;
    double reads;
    std::string barcode;
};

struct MergedEntry {
    std::vector<std::string> fields;
    std::string barcode;
    double design_reads;
    double barcode_reads;
    double total_reads;
};

void _merge(
    const std::string& library_csv,
    const std::string& library_reads_file,
    const std::string& barcodes_file,
    const std::string& barcode_reads_file,
    const std::string& output_prefix,
    bool overwrite
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

    if (!csv::is_valid_header(header_line)) {
        throw std::runtime_error("Invalid CSV header. Expected: " + csv::header());
    }

    std::vector<LibraryEntry> library_entries;
    std::string line;
    size_t idx = 0;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        LibraryEntry entry;
        entry.original_index = idx++;
        entry.fields = _split_by_delimiter(line, ',');
        library_entries.push_back(entry);
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
        entry.fields = library_entries[i].fields;
        entry.barcode = barcode_entries[i].barcode;
        entry.design_reads = library_entries[i].reads;
        entry.barcode_reads = barcode_entries[i].reads;
        entry.total_reads = entry.design_reads + entry.barcode_reads;

        // Replace the barcode field
        if (entry.fields.size() > csv::BARCODE) {
            entry.fields[csv::BARCODE] = entry.barcode;
        }

        merged.push_back(entry);
    }

    // Sort merged by total reads ASCENDING
    std::sort(merged.begin(), merged.end(),
        [](const MergedEntry& a, const MergedEntry& b) {
            return a.total_reads < b.total_reads;
        });

    // Write output CSV with extra columns
    std::string csv_out = output_prefix + ".csv";
    std::ofstream out_csv(csv_out);
    out_csv << header_line << ",design_reads,barcode_reads,total_reads\n";
    for (const auto& entry : merged) {
        for (size_t i = 0; i < entry.fields.size(); i++) {
            out_csv << entry.fields[i];
            if (i < entry.fields.size() - 1) out_csv << ",";
        }
        out_csv << "," << entry.design_reads
                << "," << entry.barcode_reads
                << "," << entry.total_reads << "\n";
    }
    out_csv.close();

    // Write FASTA and TXT
    std::string fasta_out = output_prefix + ".fasta";
    std::string txt_out = output_prefix + ".txt";
    std::ofstream out_fasta(fasta_out);
    std::ofstream out_txt(txt_out);

    for (const auto& entry : merged) {
        if (entry.fields.size() < csv::COUNT) {
            std::cerr << "Warning: skipping malformed CSV row with "
                      << entry.fields.size() << " fields (expected " << csv::COUNT << ")\n";
            continue;
        }

        std::string name = entry.fields[csv::NAME];
        std::string sublibrary = entry.fields[csv::SUBLIBRARY];
        std::string seq = entry.fields[csv::FIVE_CONST] + entry.fields[csv::FIVE_PADDING] +
                          entry.fields[csv::DESIGN] + entry.fields[csv::THREE_PADDING] +
                          entry.fields[csv::BARCODE] + entry.fields[csv::THREE_CONST];

        out_fasta << ">" << name << " (" << sublibrary << ")\n" << seq << "\n";
        out_txt << seq << "\n";
    }

    out_fasta.close();
    out_txt.close();

    std::cout << "Merged " << merged.size() << " library entries with barcodes.\n";
    std::cout << "Output: " << csv_out << ", " << fasta_out << ", " << txt_out << "\n";
}
