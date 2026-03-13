#include "padding.hpp"
#include "nuc.hpp"
#include "utils.hpp"
#include "io/csv_format.hpp"
#include <fstream>
#include <iostream>

void _generate_padding(
    const std::string& library_csv,
    size_t pad_to,
    const StemConfig& config,
    const std::string& output_file,
    bool overwrite
) {
    _throw_if_not_exists(library_csv);
    _remove_if_exists(output_file, overwrite);

    // Read library CSV to get design lengths
    std::ifstream in(library_csv);
    std::string header_line;
    std::getline(in, header_line);

    csv::Header header(header_line);
    header.validate();

    std::vector<size_t> design_lengths;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::vector<std::string> fields = _split_by_delimiter(line, ',');
        std::string design = header.get(fields, csv::COL_DESIGN);
        design_lengths.push_back(design.size());
    }
    in.close();

    // Generate padding sequences
    std::mt19937 gen = _init_gen();
    std::ofstream out(output_file);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to open output file: " + output_file);
    }

    for (size_t design_len : design_lengths) {
        if (design_len >= pad_to) {
            out << "\n";
        } else {
            size_t padding_length = pad_to - design_len;
            std::string padding = _get_padding(padding_length, config, gen);
            out << padding << "\n";
        }
    }
    out.close();

    std::cout << "Generated " << design_lengths.size() << " padding sequences.\n";
}
