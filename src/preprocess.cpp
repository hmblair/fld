#include "preprocess.hpp"
#include "io/csv_format.hpp"

static inline std::string _PARSER_NAME = "preprocess";
// File
static inline std::string _FILE_NAME = "file";
static inline std::string _FILE_HELP = "The input .fasta file.";
// Output
static inline std::string _OUTPUT_NAME = "--output";
static inline std::string _OUTPUT_HELP = "The output .csv file.";
// Overwrite 
static inline std::string _OVERWRITE_NAME = "--overwrite";
static inline std::string _OVERWRITE_HELP = "Overwrite any existing file.";
// Sublibrary
static inline std::string _SUBLIB_NAME = "--sublibrary";
static inline std::string _SUBLIB_HELP = "The sublibrary this .fasta belongs to.";
static inline std::string _SUBLIB_DEFAULT = "";

PreprocessArgs::PreprocessArgs() :
    Program(_PARSER_NAME),
    file(_parser, _FILE_NAME, _FILE_HELP),
    output(_parser, _OUTPUT_NAME, _OUTPUT_HELP),
    overwrite(_parser, _OVERWRITE_NAME, _OVERWRITE_HELP),
    sublibrary(_parser, _SUBLIB_NAME, _SUBLIB_HELP, _SUBLIB_DEFAULT) {

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
    std::string sequence;
    bool write = false;

    csv_file << csv::header() << "\n";
    while (std::getline(fasta_file, line)) {
        if (_is_fasta_header(line)) {
            if (write) {
                csv_file << sequence;
                csv_file << ",,,\n";
                sequence = "";
                write = false;
            }
            name = _get_fasta_name(line);
            csv_file << _escape_with_quotes(name);
            csv_file << "," << sublibrary << ",,,";
        } else {
            sequence += _get_fasta_seq(line);
            write = true;
        }
    }
    csv_file << sequence;
    csv_file << ",,,\n";
}
