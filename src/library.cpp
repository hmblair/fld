#include "library.hpp"
#include "fold.hpp"
#include "nuc.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

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

//
// Barcoding related functions
//

static inline std::string _random_barcode(
    size_t stem_length,
    std::mt19937 &gen,
    size_t max_stem_au,
    size_t max_stem_gc,
    size_t max_stem_gu,
    size_t closing_gc,
    const std::unordered_set<std::string>& existing
) {
    std::string barcode;
    do {
        barcode = _random_hairpin(
            stem_length,
            gen,
            max_stem_au,
            max_stem_gc,
            max_stem_gu,
            closing_gc
        );
    }
    while (_is_hamming_neighbour(barcode, existing));
    return barcode;
}

//
// Construct
//

Construct _from_record(const std::string& record) {
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

    std::vector<Construct> sequences;

    while (std::getline(file, line)) {
        sequences.push_back(_from_record(line));
    }

    return Library(sequences);

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
    _threep_const(threep_const) { };

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
    return (
        _name + "," +
        _sublibrary + "," +
        _fivep_const + "," +
        _fivep_padding + "," +
        _design + "," +
        _threep_padding + "," +
        _barcode + "," +
        _threep_const
    );
}

std::string Construct::name() const {
    return _name + " (" + _sublibrary + ")";
}

size_t Construct::length() const {
    return str().length();
}

size_t Construct::design_length() const {
    return _design.length();
}

void Construct::to_rna() {
    _fivep_const = _to_rna(_fivep_const);
    _fivep_padding = _to_rna(_fivep_padding);
    _design = _to_rna(_design);
    _threep_padding = _to_rna(_threep_padding);
    _barcode = _to_rna(_barcode);
    _threep_const = _to_rna(_threep_const);
}

void Construct::to_dna(
    std::mt19937 &gen
) {
    _fivep_const = _to_dna(_fivep_const, gen);
    _fivep_padding = _to_dna(_fivep_padding, gen);
    _design = _to_dna(_design, gen);
    _threep_padding = _to_dna(_threep_padding, gen);
    _barcode = _to_dna(_barcode, gen);
    _threep_const = _to_dna(_threep_const, gen);
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
    // double score = _score_hairpin(
    //    _design,
    //    _barcode,
    //    4
    //);
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
        bool is_neighbour = _is_hamming_neighbour(sequence.barcode(), barcodes);
        if (is_neighbour) {
            sequence.remove_barcode();
        } else {
            barcodes.insert(sequence.barcode());
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
        sequence.to_dna(_gen);
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
    }
}

void Library::barcode(
    size_t stem_length,
    size_t max_stem_au,
    size_t max_stem_gc,
    size_t max_stem_gu,
    size_t closing_gc
) {
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
    size_t max_gu
) {
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
    if (barcode_stem_length < STEM_MIN) {
        throw std::runtime_error("The barcode stem length must be at least " + std::to_string(STEM_MIN) + ".");
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
        throw std::runtime_error("The combined maximum AU, GC, and GU counts (" + std::to_string(max_stem_au) + ", " + std::to_string(max_stem_gc) + " and " + std::to_string(max_stem_gc) + ") must be greater than the maximum stem length.");
    }
}

static inline void _check_if_enough_barcodes(
    size_t library_size,
    size_t barcode_stem_length,
    size_t max_stem_au,
    size_t max_stem_gc,
    size_t max_stem_gu
) {
    size_t _counts = _stem_counts(
        barcode_stem_length,
        max_stem_au,
        max_stem_gc,
        max_stem_gu
    );
    if (_counts < library_size) {
        throw std::runtime_error("The barcode length (" + std::to_string(barcode_stem_length) + ") and maximum base-pair counts (" + std::to_string(max_stem_au) + " and " + std::to_string(max_stem_gc) + ") are only large enough to accomodate " + std::to_string(_counts) + " of the required " + std::to_string(library_size) + " unique barcodes.");
    }
}

class _STEM_CONSTRAINTS {
public:
    size_t min_stem_length;
    size_t max_stem_length;
    size_t max_stem_au;
    size_t max_stem_gc;
    size_t max_stem_gu;
    size_t closing_gc;
    _STEM_CONSTRAINTS (
        size_t min_stem_length,
        size_t max_stem_length,
        size_t max_stem_au,
        size_t max_stem_gc,
        size_t max_stem_gu,
        size_t closing_gc
    ) : min_stem_length(min_stem_length),
        max_stem_length(max_stem_length),
        max_stem_au(max_stem_au),
        max_stem_gc(max_stem_gc),
        max_stem_gu(max_stem_gu),
        closing_gc(closing_gc) { };
};

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
    const std::string& five_const,
    const std::string& three_const
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
    _check_if_enough_barcodes(
        library.size(),
        barcode_stem_length,
        max_stem_au,
        max_stem_gc,
        max_stem_gu
    );

    library.to_dna();

    std::cout << "     Padding..." << std::flush;
    library.pad(
        pad_to_length,
        min_stem_length,
        max_stem_length,
        2,
        max_stem_au,
        max_stem_gc,
        max_stem_gu,
        closing_gc
    );
    std::cout << "        DONE" << std::endl;
    std::cout << "     Barcoding..." << std::flush;
    library.barcode(
        barcode_stem_length,
        max_stem_au,
        max_stem_gc,
        max_stem_gu,
        closing_gc
    );
    std::cout << "      DONE" << std::endl;
    std::cout << "     Primerizing..." << std::flush;
    library.primerize(five_const, three_const);
    std::cout << "    DONE" << std::endl;
    std::cout << "\n";
}


