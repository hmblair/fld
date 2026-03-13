#include "merge_padding.hpp"
#include "io/csv_format.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

struct PaddingLibraryEntry {
    size_t original_index = 0;
    size_t row_index = 0;  // 0-based position in CSV
    std::string sublibrary;
    size_t design_length = 0;
    double reads = 0.0;
    std::vector<std::string> fields;
};

struct PaddingEntry {
    size_t row_index = 0;
    double reads = 0.0;
    std::string padding;
};

void _merge_padding(
    const std::string& library_csv,
    const std::string& library_reads_file,
    const std::string& padding_file,
    const std::string& padding_reads_file,
    const std::string& output_prefix,
    bool overwrite
) {
    _throw_if_not_exists(library_csv);
    _throw_if_not_exists(library_reads_file);
    _throw_if_not_exists(padding_file);
    _throw_if_not_exists(padding_reads_file);
    _remove_if_exists_all(output_prefix, overwrite);

    // Load library CSV
    std::ifstream in(library_csv);
    std::string header_line;
    std::getline(in, header_line);

    csv::Header header(header_line);
    header.validate();

    std::vector<PaddingLibraryEntry> library_entries;
    std::string line;
    size_t row_num = 1;
    size_t row_idx = 0;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        PaddingLibraryEntry entry;
        entry.fields = _split_by_delimiter(line, ',');
        std::string idx_str = header.get(entry.fields, csv::COL_INDEX, std::to_string(row_num));
        entry.original_index = std::stoull(idx_str);
        entry.sublibrary = header.get(entry.fields, csv::COL_SUBLIBRARY, "");
        std::string design = header.get(entry.fields, csv::COL_DESIGN);
        entry.design_length = design.size();
        entry.row_index = row_idx;
        library_entries.push_back(entry);
        row_num++;
        row_idx++;
    }
    in.close();

    // Load reads
    std::vector<double> lib_reads = _load_reads(library_reads_file, library_entries.size());
    for (size_t i = 0; i < library_entries.size(); i++) {
        library_entries[i].reads = lib_reads[i];
    }

    // Load padding sequences and reads
    std::vector<std::string> padding_seqs = _load_lines(padding_file);
    // _load_lines skips empty lines, but we need empty lines for zero-length padding.
    // Re-read manually to preserve empty lines.
    {
        padding_seqs.clear();
        std::ifstream pf(padding_file);
        std::string pline;
        while (std::getline(pf, pline)) {
            padding_seqs.push_back(pline);
        }
    }

    if (padding_seqs.size() < library_entries.size()) {
        throw std::runtime_error("Not enough padding sequences (" +
            std::to_string(padding_seqs.size()) + ") for library entries (" +
            std::to_string(library_entries.size()) + ")");
    }

    std::vector<double> pad_reads = _load_reads(padding_reads_file, padding_seqs.size());

    std::vector<PaddingEntry> padding_entries;
    for (size_t i = 0; i < padding_seqs.size(); i++) {
        PaddingEntry entry;
        entry.row_index = i;
        entry.reads = pad_reads[i];
        entry.padding = padding_seqs[i];
        padding_entries.push_back(entry);
    }

    // Group by design length
    std::unordered_map<size_t, std::vector<size_t>> length_groups;
    for (size_t i = 0; i < library_entries.size(); i++) {
        length_groups[library_entries[i].design_length].push_back(i);
    }

    // Result: maps row_index -> assigned padding
    std::vector<std::string> assigned_padding(library_entries.size());
    std::vector<double> assigned_design_reads(library_entries.size());
    std::vector<double> assigned_padding_reads(library_entries.size());

    for (auto& [design_len, indices] : length_groups) {
        // Collect library entries and padding entries for this group
        std::vector<size_t> lib_idx_sorted = indices;
        std::vector<size_t> pad_idx_sorted = indices;  // same indices — 1:1 correspondence

        // Sort library indices by reads ASC
        std::sort(lib_idx_sorted.begin(), lib_idx_sorted.end(),
            [&](size_t a, size_t b) {
                return library_entries[a].reads < library_entries[b].reads;
            });

        // Sort padding indices by reads DESC
        std::sort(pad_idx_sorted.begin(), pad_idx_sorted.end(),
            [&](size_t a, size_t b) {
                return padding_entries[a].reads > padding_entries[b].reads;
            });

        // Pair: low-read design gets high-read padding
        for (size_t i = 0; i < lib_idx_sorted.size(); i++) {
            size_t lib_i = lib_idx_sorted[i];
            size_t pad_i = pad_idx_sorted[i];
            assigned_padding[lib_i] = padding_entries[pad_i].padding;
            assigned_design_reads[lib_i] = library_entries[lib_i].reads;
            assigned_padding_reads[lib_i] = padding_entries[pad_i].reads;
        }
    }

    // Apply padding to library entries
    int pad_col = header.index_of(csv::COL_FIVE_PADDING);
    for (size_t i = 0; i < library_entries.size(); i++) {
        if (pad_col >= 0 && static_cast<size_t>(pad_col) < library_entries[i].fields.size()) {
            library_entries[i].fields[pad_col] = assigned_padding[i];
        }
    }

    // Sort by (sublibrary, original_index) to restore input order
    std::vector<size_t> output_order(library_entries.size());
    std::iota(output_order.begin(), output_order.end(), 0);
    std::sort(output_order.begin(), output_order.end(),
        [&](size_t a, size_t b) {
            if (library_entries[a].sublibrary != library_entries[b].sublibrary) {
                return library_entries[a].sublibrary < library_entries[b].sublibrary;
            }
            return library_entries[a].original_index < library_entries[b].original_index;
        });

    // Write output CSV
    std::string csv_out = output_prefix + ".csv";
    std::ofstream out_csv(csv_out);
    out_csv << header_line << ",design_reads,padding_reads\n";
    for (size_t idx : output_order) {
        const auto& entry = library_entries[idx];
        for (size_t i = 0; i < entry.fields.size(); i++) {
            out_csv << _quote_csv_field(entry.fields[i]);
            if (i < entry.fields.size() - 1) out_csv << ",";
        }
        out_csv << "," << assigned_design_reads[idx]
                << "," << assigned_padding_reads[idx] << "\n";
    }
    out_csv.close();

    // Write FASTA
    std::string fasta_out = output_prefix + ".fasta";
    std::ofstream out_fasta(fasta_out);
    for (size_t idx : output_order) {
        const auto& entry = library_entries[idx];
        std::string seq = header.get(entry.fields, csv::COL_FIVE_CONST) +
                          header.get(entry.fields, csv::COL_FIVE_PADDING) +
                          header.get(entry.fields, csv::COL_DESIGN) +
                          header.get(entry.fields, csv::COL_THREE_PADDING) +
                          header.get(entry.fields, csv::COL_BARCODE) +
                          header.get(entry.fields, csv::COL_THREE_CONST);

        std::string name = header.get(entry.fields, csv::COL_NAME, "sequence");
        std::string sublibrary = header.get(entry.fields, csv::COL_SUBLIBRARY, "");
        if (sublibrary.empty()) {
            out_fasta << ">" << name << "\n" << seq << "\n";
        } else {
            out_fasta << ">" << name << " (" << sublibrary << ")\n" << seq << "\n";
        }
    }
    out_fasta.close();

    std::cout << "Merged " << library_entries.size() << " library entries with padding.\n";
    std::cout << "Output: " << csv_out << ", " << fasta_out << "\n";
}
