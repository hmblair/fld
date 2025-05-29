#include "library.hpp"
#include "utils.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <indicators/block_progress_bar.hpp>

using namespace indicators;

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

static inline void _check_header(const std::string& header) {
    if (header != _csv_header()) {
        throw std::runtime_error("The columns in the .csv are not as expected.");
    }
}

StemContent::StemContent(
    size_t max_au,
    size_t max_gc,
    size_t max_gu,
    size_t closing_gc
) : max_au(max_au),
    max_gc(max_gc),
    max_gu(max_gu),
    closing_gc(closing_gc) { };

//
// Barcoding related functions
//

// static inline std::string _random_barcode_with_fold(
//     const std::string& sequence,
//     size_t stem_length,
//     std::mt19937 &gen,
//     size_t max_stem_au,
//     size_t max_stem_gc,
//     size_t max_stem_gu,
//     size_t closing_gc,
//     const std::unordered_set<std::string>& existing,
//     double cutoff,
//     size_t max_attempts
// ) {
//     std::string barcode, best_barcode = _random_barcode(
//         stem_length,
//         gen,
//         max_stem_au,
//         max_stem_gc,
//         max_stem_gu,
//         closing_gc,
//         existing
//     );
//     if (max_attempts <= 0) {
//         return best_barcode;
//     }
//     double score = 0, best_score = _score_hairpin(sequence, barcode, 4);
//     size_t attempts = 0;
//     while (score < cutoff && attempts < max_attempts) {
//         barcode = _random_barcode(
//                 stem_length,
//                 gen,
//                 max_stem_au,
//                 max_stem_gc,
//                 max_stem_gu,
//                 closing_gc,
//                 existing
//         );
//         score = _score_hairpin(sequence, barcode, 4);
//         attempts++;
//         if (score > best_score) {
//             best_score = score;
//             best_barcode = barcode;
//         }
//     }
//     return best_barcode;
// }

//
// Construct
//

static inline Construct _from_record(const std::string& record) {
    std::vector<std::string> columns = _split_by_delimiter(record, ',');
    return Construct(
        columns[0],
        columns[1],
        columns[2],
        columns[3],
        columns[4],
        columns[5],
        columns[6],
        columns[7]
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
    std::string name,
    std::string sublibrary,
    std::string fivep_const,
    std::string fivep_padding,
    std::string design,
    std::string threep_padding,
    std::string barcode,
    std::string threep_const
) : _name(name),
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
    std::mt19937 &gen,
    size_t max_stem_au,
    size_t max_stem_gc,
    size_t max_stem_gu,
    size_t closing_gc,
    std::unordered_set<std::string>& existing
) {
    _barcode = _random_barcode(
        stem_length,
        gen,
        max_stem_au,
        max_stem_gc,
        max_stem_gu,
        closing_gc,
        existing
    );
    existing.insert(_barcode);
}

void Construct::pad(
    size_t padded_size,
    size_t min_stem_length,
    size_t max_stem_length,
    size_t spacer_length,
    size_t max_stem_au,
    size_t max_stem_gc,
    size_t max_stem_gu,
    size_t closing_gc,
    std::mt19937 &gen
) {
    if (padded_size < design_length()) {
        throw std::runtime_error("The design region is larger than the padded size (" + std::to_string(design_length()) + " vs " + std::to_string(padded_size) + ")." );
    }
    _fivep_padding = _get_padding(
        padded_size - design_length(),
        min_stem_length,
        max_stem_length,
        spacer_length,
        max_stem_au,
        max_stem_gc,
        max_stem_gu,
        closing_gc,
        gen
    );
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
    size_t min_stem_length,
    size_t max_stem_length,
    size_t spacer_length,
    size_t max_stem_au,
    size_t max_stem_gc,
    size_t max_stem_gu,
    size_t closing_gc
) {

    using namespace indicators;

    // show_console_cursor(false);

    BlockProgressBar bar{
        option::BarWidth{30},
        option::Start{"["},
        option::End{"]"},
        option::PrefixText{"Padding   "},
        option::ForegroundColor{Color::white}  ,
        option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}
    };

    size_t count = 0;
    for (auto& sequence : _sequences) {
        sequence.pad(
            padded_size,
            min_stem_length,
            max_stem_length,
            spacer_length,
            max_stem_au,
            max_stem_gc,
            max_stem_gu,
            closing_gc,
            _gen
        );
        count++;
        bar.set_progress( (count * 100) / size());
    }

    // show_console_cursor(true);

}

void Library::barcode(
    size_t stem_length,
    size_t max_stem_au,
    size_t max_stem_gc,
    size_t max_stem_gu,
    size_t closing_gc
) {


    BlockProgressBar bar{
        option::BarWidth{30},
        option::Start{"["},
        option::End{"]"},
        option::PrefixText{"Barcoding "},
        option::ForegroundColor{Color::white}  ,
        option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}
    };

    size_t count = 0;
    for (auto& sequence : _sequences) {
        sequence.replace_barcode(
            stem_length,
            _gen,
            max_stem_au,
            max_stem_gc,
            max_stem_gu,
            closing_gc,
            _barcodes
        );
        count++;
        bar.set_progress( (count * 100) / size());
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
    _throw_if_exists(filename);
    std::ofstream file(filename);
    file << _csv_header() << "\n";
    for (const Construct& sequence : _sequences) {
        file << sequence.csv_record() << "\n";
    }
}

void Library::to_txt(
    const std::string& filename
) const {
    _throw_if_exists(filename);
    std::ofstream file(filename);
    for (const Construct& sequence : _sequences) {
        file << sequence.str() << "\n";
    }
}

void Library::to_fasta(
    const std::string& filename
) const {
    _throw_if_exists(filename);
    std::ofstream file(filename);
    for (const Construct& sequence : _sequences) {
        file << ">" << sequence.name() << "\n";
        file << sequence.str() << "\n";
    }
}

void Library::save(const std::string& prefix) const {
    to_csv(_csv_name(prefix));
    to_txt(_txt_name(prefix));
    to_fasta(_fasta_name(prefix));
}

static inline size_t _stem_counts(
    size_t length,
    size_t max_au,
    size_t max_gc,
    size_t max_gu,
    size_t closing_gc
) {
    // Adjust for closing GC base pairs
    max_gc -= closing_gc;

    std::vector<std::vector<size_t>> prev(max_au + 1, std::vector<size_t>(max_gc + 1, 0));
    std::vector<std::vector<size_t>> cur(max_au + 1, std::vector<size_t>(max_gc + 1, 0));

    prev[0][0] = 1;  // Base case: 1 way to form an empty sequence

    for (size_t i = 1; i <= length; ++i) {
        for (size_t a = 0; a <= max_au; ++a) {
            for (size_t b = 0; b <= max_gc; ++b) {
                size_t c = i - a - b;
                if (c > max_gu || c < 0) continue;  // Skip invalid states

                size_t count = 0;
                if (a > 0) count += prev[a - 1][b];  // Add AU pair
                if (b > 0) count += prev[a][b - 1];  // Add GC pair
                count += prev[a][b];  // Add GU pair

                cur[a][b] = count;
            }
        }
        std::swap(prev, cur);  // Move to next iteration, avoid copying
    }

    size_t result = 0;
    for (size_t a = 0; a <= max_au; ++a) {
        for (size_t b = 0; b <= max_gc; ++b) {
            size_t c = length - a - b;
            if (c <= max_gu) result += prev[a][b];  // Sum valid results
        }
    }

    return result * (1ULL << length);
}

static inline constexpr size_t STEM_MIN = 4;
static inline void _check_stem_length(
    size_t min_stem_length,
    size_t barcode_stem_length
) {
    if (min_stem_length < STEM_MIN) {
        throw std::runtime_error("The minimum stem length must be at least " + std::to_string(STEM_MIN) + ".");
    }
    if (barcode_stem_length > 0 && barcode_stem_length < STEM_MIN) {
        throw std::runtime_error("A nonzero barcode stem must be at least " + std::to_string(STEM_MIN) + " base pairs.");
    }
}

static inline void _check_if_enough_bases(
    size_t max_stem_length,
    size_t max_stem_au,
    size_t max_stem_gc,
    size_t max_stem_gu
) {
    size_t _max_possible_stem = max_stem_au + max_stem_gc + max_stem_gu;
    if (_max_possible_stem < max_stem_length) {
        throw std::runtime_error("The combined maximum AU, GC, and GU counts (" + std::to_string(max_stem_au) + ", " + std::to_string(max_stem_gc) + " and " + std::to_string(max_stem_gu) + ") must be greater than the maximum stem length.");
    }
}

static inline void _check_if_enough_barcodes(
    size_t library_size,
    size_t barcode_stem_length,
    size_t max_stem_au,
    size_t max_stem_gc,
    size_t max_stem_gu,
    size_t closing_gc
) {
    size_t _counts = _stem_counts(
        barcode_stem_length,
        max_stem_au,
        max_stem_gc,
        max_stem_gu,
        closing_gc
    );
    if (_counts < library_size) {
        throw std::runtime_error("The barcode length (" + std::to_string(barcode_stem_length) + ") and maximum base-pair counts (" + std::to_string(max_stem_au) + ", " + std::to_string(max_stem_gc) + " and " + std::to_string(max_stem_gu) + ") are only large enough to accomodate " + std::to_string(_counts) + " of the required " + std::to_string(library_size) + " unique barcodes.");
    }
}


void _run_checks(
    const Library& library,
    size_t barcode_stem_length,
    size_t min_stem_length,
    size_t max_stem_length,
    size_t max_stem_au,
    size_t max_stem_gc,
    size_t max_stem_gu,
    size_t closing_gc
) {
    _check_stem_length(
        min_stem_length,
        barcode_stem_length
    );
    _check_if_enough_bases(
        max_stem_length,
        max_stem_au,
        max_stem_gc,
        max_stem_gu
    );
    if (barcode_stem_length > 0) {
        _check_if_enough_barcodes(
            library.size(),
            barcode_stem_length,
            max_stem_au,
            max_stem_gc,
            max_stem_gu,
            closing_gc
        );
    }
}

void _add_library_elements(
    Library& library,
    size_t pad_to_length,
    size_t barcode_stem_length,
    size_t min_stem_length,
    size_t max_stem_length,
    size_t max_stem_au,
    size_t max_stem_gc,
    size_t max_stem_gu,
    size_t closing_gc,
    size_t poly_a_spacer,
    const std::string& five_const,
    const std::string& three_const
) {

    _run_checks(
        library,
        barcode_stem_length,
        min_stem_length,
        max_stem_length,
        max_stem_au,
        max_stem_gc,
        max_stem_gu,
        closing_gc
    );

    // Also converts U to T
    library.replace_polybases();

    std::cout << "\n";
    std::cout << "Processing " << std::to_string(library.size()) << " sequences." << std::endl;
    std::cout << "───────────────────────────────────────────────\n";

    library.pad(
        pad_to_length,
        min_stem_length,
        max_stem_length,
        poly_a_spacer,
        max_stem_au,
        max_stem_gc,
        max_stem_gu,
        closing_gc
    );
    if (barcode_stem_length > 0) {
        std::cout << std::endl;
        library.barcode(
            barcode_stem_length,
            max_stem_au,
            max_stem_gc,
            max_stem_gu,
            closing_gc
        );
    }
    std::cout << "\n";
    std::cout << "\n";
    library.primerize(five_const, three_const);
}

void _design(
    const std::string& file,
    const std::string& output,
    bool overwrite,
    int pad,
    int barcode_length,
    int min_stem_length,
    int max_stem_length,
    int max_stem_au,
    int max_stem_gc,
    int max_stem_gu,
    int closing_gc,
    int spacer,
    const std::string& five_const,
    const std::string& three_const
) {

    // Remove any existing output files
    _remove_if_exists_all(
        output,
        overwrite
    );
    // Load the library from the provided .csv
    Library library = _from_csv(file);
    // Add all desired library elements
    _add_library_elements(
       library,
       pad,
       barcode_length,
       min_stem_length,
       max_stem_length,
       max_stem_au,
       max_stem_gc,
       max_stem_gu,
       closing_gc,
       spacer,
       five_const,
       three_const
    );
    // Save to disk
    library.save(output);

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
static inline int _MAX_AU_DEFAULT = 13;
static inline std::string _MAX_AU_HELP = "The maximum AU content of any stem.";

static inline std::string _MAX_GC_NAME = "--max-gc";
static inline int _MAX_GC_DEFAULT = 13;
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
