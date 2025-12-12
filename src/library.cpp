#include "library.hpp"
#include "utils.hpp"
#include "io/csv_format.hpp"
#include "io/progress.hpp"
#include "io/writers.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

static inline void _check_header(const std::string& header) {
    if (!csv::is_valid_header(header)) {
        throw std::runtime_error("The columns in the .csv are not as expected.");
    }
}

//
// Construct
//

static inline Construct _from_record(const std::string& record) {
    std::vector<std::string> columns = _split_by_delimiter(record, ',');
    if (columns.size() < csv::COUNT) {
        throw std::runtime_error("CSV record has " + std::to_string(columns.size()) +
            " columns, expected " + std::to_string(csv::COUNT));
    }
    size_t index = std::stoull(columns[csv::INDEX]);
    return Construct(
        index,
        columns[csv::NAME],
        columns[csv::SUBLIBRARY],
        columns[csv::FIVE_CONST],
        columns[csv::FIVE_PADDING],
        columns[csv::DESIGN],
        columns[csv::THREE_PADDING],
        columns[csv::BARCODE],
        columns[csv::THREE_CONST]
    );
}

Library _from_csv(const std::string& filename) {

    _throw_if_not_exists(filename);

    std::ifstream file(filename);
    std::string line;

    std::getline(file, line);
    _check_header(line);

    std::vector<Construct> constructs;

    while (std::getline(file, line)) {
        constructs.push_back(_from_record(line));
    }

    return Library(constructs);

}

Construct::Construct(
    size_t index,
    std::string name,
    std::string sublibrary,
    std::string fivep_const,
    std::string fivep_padding,
    std::string design,
    std::string threep_padding,
    std::string barcode,
    std::string threep_const
) : _index(index),
    _name(name),
    _sublibrary(sublibrary),
    _fivep_const(fivep_const),
    _fivep_padding(fivep_padding),
    _design(design),
    _threep_padding(threep_padding),
    _barcode(barcode),
    _threep_const(threep_const) {};

std::string Construct::str() const {
    return (
        _fivep_const +
        _fivep_padding +
        _design +
        _threep_padding +
        _barcode +
        _threep_const
    );
}

std::string Construct::csv_record() const {
    return(
        std::to_string(_index) + "," +
        _escape_with_quotes(_name) + "," +
        _escape_with_quotes(_sublibrary) + "," +
        _fivep_const + "," +
        _fivep_padding + "," +
        _design + "," +
        _threep_padding + "," +
        _barcode + "," +
        _threep_const
    );
}

size_t Construct::index() const {
    return _index;
}

std::string Construct::name() const {
    if (_sublibrary.empty()) {
        return _name;
    }
    return _name + " (" + _sublibrary + ")";
}

size_t Construct::length() const {
    return str().length();
}

size_t Construct::design_length() const {
    return _design.length();
}

void Construct::replace_polybases(std::mt19937 &gen) {
    _fivep_const = _replace_polybases(_fivep_const, gen);
    _fivep_padding = _replace_polybases(_fivep_padding, gen);
    _design = _replace_polybases(_design, gen);
    _threep_padding = _replace_polybases(_threep_padding, gen);
    _barcode = _replace_polybases(_barcode, gen);
    _threep_const = _replace_polybases(_threep_const, gen);
}

void Construct::to_rna() {
    _fivep_const = _to_rna(_fivep_const);
    _fivep_padding = _to_rna(_fivep_padding);
    _design = _to_rna(_design);
    _threep_padding = _to_rna(_threep_padding);
    _barcode = _to_rna(_barcode);
    _threep_const = _to_rna(_threep_const);
}

void Construct::to_dna() {
    _fivep_const = _to_dna(_fivep_const);
    _fivep_padding = _to_dna(_fivep_padding);
    _design = _to_dna(_design);
    _threep_padding = _to_dna(_threep_padding);
    _barcode = _to_dna(_barcode);
    _threep_const = _to_dna(_threep_const);
}

void Construct::replace_barcode(
    size_t stem_length,
    const StemConfig& config,
    std::mt19937 &gen,
    std::unordered_set<std::string>& existing
) {
    _barcode = _random_barcode(stem_length, config, gen, existing);
    existing.insert(_barcode);
}

void Construct::pad(
    size_t padded_size,
    const StemConfig& config,
    std::mt19937 &gen
) {
    if (padded_size < design_length()) {
        throw std::runtime_error("The design region is larger than the padded size (" + std::to_string(design_length()) + " vs " + std::to_string(padded_size) + ")." );
    }
    _fivep_padding = _get_padding(padded_size - design_length(), config, gen);
}

bool Construct::has_barcode() const {
    return _barcode.length() > 0;
}

std::string Construct::barcode() const {
    return _barcode;
}

void Construct::remove_barcode() {
    _barcode = "";
}

void Construct::primerize(
    const std::string& five,
    const std::string& three
) {
    _fivep_const = five;
    _threep_const = three;
}

void Construct::remove_padding() {
    _fivep_padding = "";
    _threep_padding = "";
}

//
// The main library class
//

static inline void _insert_or_remove(
    Construct& sequence,
    std::unordered_set<std::string>& barcodes
) {
    if (sequence.has_barcode()) {
        bool inserted =  _insert_if_not_neighbour(sequence.barcode(), barcodes);
        if (!inserted) {
            sequence.remove_barcode();
        }
    }
}

Library::Library(
    std::vector<Construct>& sequences
) : _sequences(sequences) {
    _gen = _init_gen();
    for (auto& sequence: _sequences) {
        _insert_or_remove(sequence, _barcodes);
    }
}

size_t Library::size() const {
    return _sequences.size();
}

size_t Library::barcodes() const {
    return _barcodes.size();
}

void Library::to_rna() {
    for (auto& sequence : _sequences) {
        sequence.to_rna();
    }
}

void Library::to_dna() {
    for (auto& sequence : _sequences) {
        sequence.to_dna();
    }
}

void Library::replace_polybases() {
    for (auto& sequence : _sequences) {
        sequence.replace_polybases(_gen);
    }
}

void Library::pad(
    size_t padded_size,
    const StemConfig& config
) {
    ProgressBar bar("Padding   ");
    size_t count = 0;
    for (auto& sequence : _sequences) {
        sequence.pad(padded_size, config, _gen);
        count++;
        bar.update(count, size());
    }
}

void Library::barcode(
    size_t stem_length,
    const StemConfig& config
) {
    ProgressBar bar("Barcoding ");
    size_t count = 0;
    for (auto& sequence : _sequences) {
        sequence.replace_barcode(stem_length, config, _gen, _barcodes);
        count++;
        bar.update(count, size());
    }
}

void Library::primerize(
    const std::string& five,
    const std::string& three
) {
    for (auto& sequence : _sequences) {
        sequence.primerize(five, three);
    }
}

void Library::to_csv(
    const std::string& filename
) const {
    CsvWriter writer(filename);
    for (const Construct& sequence : _sequences) {
        writer.write_line(sequence.csv_record());
    }
}

void Library::to_txt(
    const std::string& filename
) const {
    TxtWriter writer(filename);
    for (const Construct& sequence : _sequences) {
        writer.write_line(sequence.str());
    }
}

void Library::to_fasta(
    const std::string& filename
) const {
    FastaWriter writer(filename);
    for (const Construct& sequence : _sequences) {
        writer.write_sequence(sequence.name(), sequence.str());
    }
}

void Library::save(const std::string& prefix) const {
    to_csv(output_csv(prefix));
    to_txt(output_txt(prefix));
    to_fasta(output_fasta(prefix));
}

static inline void _add_library_elements(
    Library& library,
    const DesignConfig& config
) {
    config.validate_with_library_size(library.size());

    // Also converts U to T
    library.replace_polybases();

    std::cout << "\n";
    std::cout << "Processing " << std::to_string(library.size()) << " sequences." << std::endl;
    std::cout << "───────────────────────────────────────────────\n";

    library.pad(config.pad_to_length, config.stem);
    if (config.barcode.is_enabled()) {
        std::cout << std::endl;
        library.barcode(config.barcode.stem_length, config.barcode.stem);
    }
    std::cout << "\n";
    std::cout << "\n";
    library.primerize(config.five_const, config.three_const);
}

void _design(const DesignConfig& config) {
    // Remove any existing output files
    _remove_if_exists_all(config.output_prefix, config.overwrite);

    // Load the library from the provided .csv
    Library library = _from_csv(config.input_path);

    // Add all desired library elements
    _add_library_elements(library, config);

    // Save to disk
    library.save(config.output_prefix);
}

//
// Argument parsing
//

// Parser name
static inline std::string _PARSER_NAME = "design";

// File argument
static inline std::string _FILE_NAME = "file";
static inline std::string _FILE_HELP = "The input .csv file.";

// Output argument
static inline std::string _OUTPUT_NAME = "-o";
static inline std::string _OUTPUT_LONG_NAME = "--output";
static inline std::string _OUTPUT_HELP = "The output prefix.";

// Required arguments
static inline std::string _PAD_TO_NAME = "--pad-to";
static inline std::string _PAD_TO_HELP = "Pad the design region of all sequences to this length.";
static inline std::string _BARCODE_LENGTH_NAME = "--barcode-length";
static inline int _BARCODE_LENGTH_DEFAULT = 0;
static inline std::string _BARCODE_LENGTH_HELP = "The length of the stem of each barcode. A value of 0 disables barcoding.";

// Optional arguments with defaults
static inline std::string _OVERWRITE_NAME = "--overwrite";
static inline std::string _OVERWRITE_HELP = "Overwrite any existing file.";

static inline std::string _FIVE_CONST_NAME = "--five-const";
static inline std::string _FIVE_CONST_DEFAULT = "ACTCGAGTAGAGTCGAAAA";
static inline std::string _FIVE_CONST_HELP = "The 5' constant sequence.";

static inline std::string _THREE_CONST_NAME = "--three-const";
static inline std::string _THREE_CONST_DEFAULT = "AAAAGAAACAACAACAACAAC";
static inline std::string _THREE_CONST_HELP = "The 3' constant sequence.";

static inline std::string _MIN_STEM_LENGTH_NAME = "--min-stem-length";
static inline int _MIN_STEM_LENGTH_DEFAULT = 7;
static inline std::string _MIN_STEM_LENGTH_HELP = "The minimum length of the stem of a hairpin.";

static inline std::string _MAX_STEM_LENGTH_NAME = "--max-stem-length";
static inline int _MAX_STEM_LENGTH_DEFAULT = 9;
static inline std::string _MAX_STEM_LENGTH_HELP = "The maximum length of the stem of a hairpin.";

static inline std::string _MAX_AU_NAME = "--max-au";
static inline int _MAX_AU_DEFAULT = INT_MAX;
static inline std::string _MAX_AU_HELP = "The maximum AU content of any stem.";

static inline std::string _MAX_GC_NAME = "--max-gc";
static inline int _MAX_GC_DEFAULT = 5;
static inline std::string _MAX_GC_HELP = "The maximum GC content of any stem.";

static inline std::string _MAX_GU_NAME = "--max-gu";
static inline int _MAX_GU_DEFAULT = 0;
static inline std::string _MAX_GU_HELP = "The maximum GU content of any stem.";

static inline std::string _CLOSING_GC_NAME = "--closing-gc";
static inline int _CLOSING_GC_DEFAULT = 1;
static inline std::string _CLOSING_GC_HELP = "The number of GC pairs to close each stem with.";

static inline std::string _SPACER_NAME = "--spacer";
static inline int _SPACER_DEFAULT = 2;
static inline std::string _SPACER_HELP = "The length of the polyA spacer used in between consecutive padding stems.";

DesignArgs::DesignArgs() :
    Program(_PARSER_NAME),
    file(_parser, _FILE_NAME, _FILE_HELP),
    output(_parser, _OUTPUT_NAME, _OUTPUT_HELP),
    pad_to(_parser, _PAD_TO_NAME, _PAD_TO_HELP),
    barcode_length(_parser, _BARCODE_LENGTH_NAME, _BARCODE_LENGTH_HELP, _BARCODE_LENGTH_DEFAULT),
    overwrite(_parser, _OVERWRITE_NAME, _OVERWRITE_HELP),
    five_const(_parser, _FIVE_CONST_NAME, _FIVE_CONST_HELP, _FIVE_CONST_DEFAULT),
    three_const(_parser, _THREE_CONST_NAME, _THREE_CONST_HELP, _THREE_CONST_DEFAULT),
    min_stem_length(_parser, _MIN_STEM_LENGTH_NAME, _MIN_STEM_LENGTH_HELP, _MIN_STEM_LENGTH_DEFAULT),
    max_stem_length(_parser, _MAX_STEM_LENGTH_NAME, _MAX_STEM_LENGTH_HELP, _MAX_STEM_LENGTH_DEFAULT),
    max_au(_parser, _MAX_AU_NAME, _MAX_AU_HELP, _MAX_AU_DEFAULT),
    max_gc(_parser, _MAX_GC_NAME, _MAX_GC_HELP, _MAX_GC_DEFAULT),
    max_gu(_parser, _MAX_GU_NAME, _MAX_GU_HELP, _MAX_GU_DEFAULT),
    closing_gc(_parser, _CLOSING_GC_NAME, _CLOSING_GC_HELP, _CLOSING_GC_DEFAULT),
    spacer(_parser, _SPACER_NAME, _SPACER_HELP, _SPACER_DEFAULT) {

}
