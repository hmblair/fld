#include "m2.hpp"

const std::vector<char> BASES = {'A', 'C', 'G', 'T'};

static inline void _add_single_mutant_all(const std::string &header, const std::string& sequence, std::vector<std::string>& mutants, int pos) {

        char original_base = sequence[pos];
        for (const auto& mutant_base : BASES) {

            if (mutant_base == original_base) {
                continue;
            }

            std::string mutant = sequence;
            mutant[pos] = mutant_base;

            mutants.push_back(">" + header + "_mm_" + std::to_string(pos) + "_" + original_base + "_" + mutant_base);
            mutants.push_back(mutant);

        }


}

static inline void _add_single_mutant(const std::string &header, const std::string& sequence, std::vector<std::string>& mutants, int pos) {

        char original_base, mutant_base;

        original_base = sequence[pos];
        try {
            mutant_base = _complement(original_base);
        } catch (const std::exception& e) {
            return;
        }

        std::string mutant = sequence;
        mutant[pos] = mutant_base;

        mutants.push_back(">" + header + "_mm_" + std::to_string(pos) + "_" + original_base + "_" + mutant_base);
        mutants.push_back(mutant);

}


static inline std::vector<std::string> _generate_single_mutants(const std::string &header, const std::string &sequence, bool all) {

    std::vector<std::string> mutants;
    size_t seq_len = sequence.length();

    mutants.push_back(">" + header + "_wt");
    mutants.push_back(sequence);

    if (all) {
        for (size_t i = 0; i < seq_len; i++) {
            _add_single_mutant_all(header, sequence, mutants, i);
        }
    } else {
        for (size_t i = 0; i < seq_len; i++) {
            _add_single_mutant(header, sequence, mutants, i);
        }
    }


    return mutants;

}

void _m2(
    const std::string& input,
    const std::string& output,
    bool all,
    bool overwrite
) {

    _throw_if_not_exists(input);
    _remove_if_exists(output, overwrite);

    std::ifstream infile(input);
    std::ofstream outfile(output);
    std::string line, current_header, current_sequence;

    while (std::getline(infile, line)) {

        if (_is_fasta_header(line)) {

            if (!current_sequence.empty()) {

                std::vector<std::string> mutants = _generate_single_mutants(current_header, current_sequence, all);

                for (const auto& str : mutants) {
                    outfile << str << "\n";
                }

                current_sequence.clear();

            }

            current_header = _get_fasta_name(line);

        } else {

            current_sequence += line;

        }

    }

    if (!current_sequence.empty()) {
        std::vector<std::string> mutants = _generate_single_mutants(current_header, current_sequence, all);
        for (const auto& str : mutants) {
            outfile << str << "\n";
        }
    }

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

