#include "m2.hpp"
#include "io/fasta_io.hpp"

const std::vector<char> DNA_BASES = {'A', 'C', 'G', 'T'};
const std::vector<char> RNA_BASES = {'A', 'C', 'G', 'U'};

// _complement() always returns a DNA-alphabet base (A/C/G/T). If the input
// sequence is RNA (contains U), naively using its result mixes U and T in
// the same sequence -- e.g. mutating an "A" writes a "T" into an otherwise
// all-U-form sequence. Detect RNA input once per sequence and translate
// accordingly, so m2's output stays in a single consistent alphabet.
static inline bool _is_rna(const std::string& sequence) {
    return sequence.find('U') != std::string::npos;
}

static inline void _write_single_mutant_all(
    FastaOutputStream& out,
    const std::string& header,
    const std::string& sequence,
    int pos,
    bool rna
) {
    char original_base = sequence[pos];
    const std::vector<char>& bases = rna ? RNA_BASES : DNA_BASES;
    for (const auto& mutant_base : bases) {
        if (mutant_base == original_base) {
            continue;
        }

        std::string mutant = sequence;
        mutant[pos] = mutant_base;

        std::string name = header + "_mm_" + std::to_string(pos) + "_" + original_base + "_" + mutant_base;
        out.write(name, mutant);
    }
}

static inline void _write_single_mutant(
    FastaOutputStream& out,
    const std::string& header,
    const std::string& sequence,
    int pos,
    bool rna
) {
    char original_base = sequence[pos];
    char mutant_base;
    try {
        mutant_base = _complement(original_base);
    } catch (const std::exception& e) {
        return;
    }
    if (rna && mutant_base == 'T') {
        mutant_base = 'U';
    }

    std::string mutant = sequence;
    mutant[pos] = mutant_base;

    std::string name = header + "_mm_" + std::to_string(pos) + "_" + original_base + "_" + mutant_base;
    out.write(name, mutant);
}


static inline void _write_single_mutants(
    FastaOutputStream& out,
    const std::string& header,
    const std::string& sequence,
    bool all
) {
    // Write wild type
    out.write(header + "_wt", sequence);

    bool rna = _is_rna(sequence);

    // Write mutants
    size_t seq_len = sequence.length();
    if (all) {
        for (size_t i = 0; i < seq_len; i++) {
            _write_single_mutant_all(out, header, sequence, i, rna);
        }
    } else {
        for (size_t i = 0; i < seq_len; i++) {
            _write_single_mutant(out, header, sequence, i, rna);
        }
    }
}

void _m2(
    const std::string& input,
    const std::string& output,
    bool all,
    bool overwrite
) {
    _throw_if_not_exists(input);
    _remove_if_exists(output, overwrite);

    FastaOutputStream out(output);

    for_each_fasta(input, [&](const FastaEntry& entry) {
        _write_single_mutants(out, entry.name, entry.sequence, all);
    });
}

static inline std::string _PARSER_NAME = "m2";

static inline std::string _INPUT_NAME = "input";
static inline std::string _INPUT_HELP = "Input FASTA file";

static inline std::string _OUTPUT_NAME = "--output";
static inline std::string _OUTPUT_HELP = "Output file";

static inline std::string _ALL_NAME = "--all";
static inline std::string _ALL_HELP = "Generate all mutants";

static inline std::string _OVERWRITE_NAME = "--overwrite";
static inline std::string _OVERWRITE_HELP = "Overwrite output file if exists";

M2Args::M2Args()
    : Program(_PARSER_NAME),
      input(_parser, _INPUT_NAME, _INPUT_HELP),
      output(_parser, _OUTPUT_NAME, _OUTPUT_HELP),
      all(_parser, _ALL_NAME, _ALL_HELP),
      overwrite(_parser, _OVERWRITE_NAME, _OVERWRITE_HELP) {}
