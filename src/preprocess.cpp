#include "preprocess.hpp"

static inline const std::vector<std::string> _csv_columns = {
    "name",
    "sublibrary",
    "five_const",
    "five_padding",
    "design",
    "three_padding",
    "barcode",
    "three_const"
};

static inline std::string _csv_header() {
    std::string header;
    for (size_t ix = 0; ix < _csv_columns.size() - 1; ix++) {
        header += _csv_columns[ix];
        header += ",";
    }
    header += _csv_columns[_csv_columns.size() - 1];
    return header;
}

void _preprocess(
    const std::string& fasta,
    const std::string& csv,
    bool overwrite,
    const std::string& sublibrary
) {

    _throw_if_not_exists(fasta);
    _remove_if_exists(csv, overwrite);

    std::ifstream fasta_file(fasta);
    std::ofstream csv_file(csv);

    std::string line;
    std::string name;

    csv_file << _csv_header() << "\n";
    while (std::getline(fasta_file, line)) {
        if (_is_fasta_header(line)) {
            name = _get_fasta_name(line);
            csv_file << _escape_with_quotes(name);
            csv_file << "," << sublibrary << ",,,";
        } else {
            csv_file << _get_fasta_seq(line);
            csv_file << ",,,\n"; 
        }
    }
}
