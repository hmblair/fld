#include "pipeline.hpp"
#include "preprocess.hpp"
#include "library.hpp"
#include "barcodes.hpp"
#include "merge.hpp"
#include "sort.hpp"
#include "totxt.hpp"
#include "prepend.hpp"
#include "m2.hpp"
#include "config/design_config.hpp"
#include "io/csv_format.hpp"
#include "io/fasta_io.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cstdlib>
#include <csignal>
#include <sys/wait.h>
#include <array>
#include <memory>

static inline std::string _PARSER_NAME = "pipeline";

// Default constant sequences
static const std::string DEFAULT_FIVE_CONST = "ACTCGAGTAGAGTCGAAAA";
static const std::string DEFAULT_THREE_CONST = "AAAAGAAACAACAACAACAAC";

PipelineArgs::PipelineArgs() : Program(_PARSER_NAME),
    inputs(_parser, "inputs", "Input FASTA files"),
    output(_parser, "-o", "Output directory"),
    overwrite(_parser, "--overwrite", "Overwrite existing output directory", false),
    pad_to(_parser, "--pad-to", "Pad sequences to this length", 130),
    five_const(_parser, "--five-const", "5' constant sequence", DEFAULT_FIVE_CONST),
    three_const(_parser, "--three-const", "3' constant sequence", DEFAULT_THREE_CONST),
    min_stem_length(_parser, "--min-stem-length", "Minimum stem length", 7),
    max_stem_length(_parser, "--max-stem-length", "Maximum stem length", 13),
    max_au(_parser, "--max-au", "Maximum AU pairs in stem", INT_MAX),
    max_gc(_parser, "--max-gc", "Maximum GC pairs in stem", 5),
    max_gu(_parser, "--max-gu", "Maximum GU pairs in stem", 0),
    closing_gc(_parser, "--closing-gc", "Number of closing GC pairs", 1),
    spacer(_parser, "--spacer", "Spacer length between stems", 2),
    barcode_length(_parser, "--barcode-length", "Barcode stem length (0 to disable)", 10),
    no_barcodes(_parser, "--no-barcodes", "Skip barcode generation", false),
    m2(_parser, "--m2", "Generate M2-seq complement sequences", false),
    predict(_parser, "--predict", "Predict reads with rn-coverage, merge barcodes, and sort by final reads", false),
    sort_by_reads(_parser, "--sort-by-reads", "Sort output by predicted read counts (default: preserve input order)", false)
{
    _parser.add_description(
        "Run the complete library design pipeline.\n\n"
        "Basic mode: preprocess, pad, and generate barcodes.\n"
        "With --predict: also predict reads, merge barcodes with read balancing,\n"
        "                predict final reads, and optionally sort output.\n\n"
        "Default output order preserves input order (by sublibrary and index).\n"
        "Use --sort-by-reads to sort by predicted read counts instead.\n\n"
        "Requires rn-coverage on PATH when using --predict."
    );
}

// Run a shell command and return the exit code
// Throws if the command was interrupted by a signal (e.g., Ctrl+C)
static int _run_command(const std::string& cmd) {
    std::cout << "  $ " << cmd << "\n";
    int result = std::system(cmd.c_str());
    if (WIFSIGNALED(result)) {
        int sig = WTERMSIG(result);
        throw std::runtime_error("Command interrupted by signal " + std::to_string(sig));
    }
    return WEXITSTATUS(result);
}

// Check if a command is available on PATH
static bool _command_exists(const std::string& cmd) {
    std::string check = "which " + cmd + " > /dev/null 2>&1";
    return std::system(check.c_str()) == 0;
}

// Sanity check: verify that begin/end columns correctly locate design in FASTA
static void _check_design_against_fasta(
    const std::string& csv_path,
    const std::string& fasta_path
) {
    // Read FASTA sequences (order matches CSV rows)
    std::vector<FastaEntry> fasta_entries = read_fasta(fasta_path);

    // Read CSV and check each row
    std::ifstream csv_file(csv_path);
    std::string line;
    std::getline(csv_file, line);  // Skip header

    size_t row = 0;
    while (std::getline(csv_file, line)) {
        if (line.empty()) continue;

        std::vector<std::string> fields = _split_by_delimiter(line, ',');
        if (fields.size() < csv::COUNT) {
            throw std::runtime_error("CSV row " + std::to_string(row + 1) +
                " has insufficient columns");
        }

        std::string design = fields[csv::DESIGN];
        size_t begin = std::stoull(fields[csv::BEGIN]);
        size_t end = std::stoull(fields[csv::END]);

        // Check length sanity
        size_t expected_len = end - begin + 1;
        if (expected_len != design.size()) {
            throw std::runtime_error(
                "Row " + std::to_string(row + 1) + ": begin/end length mismatch. " +
                "end - begin + 1 = " + std::to_string(expected_len) +
                ", but design length = " + std::to_string(design.size()));
        }

        // Check against FASTA sequence
        if (row >= fasta_entries.size()) {
            throw std::runtime_error(
                "Row " + std::to_string(row + 1) + ": no corresponding FASTA entry");
        }

        const std::string& fasta_seq = fasta_entries[row].sequence;
        if (end > fasta_seq.size()) {
            throw std::runtime_error(
                "Row " + std::to_string(row + 1) + ": end position " +
                std::to_string(end) + " exceeds FASTA sequence length " +
                std::to_string(fasta_seq.size()));
        }

        // Extract substring (convert 1-based to 0-based)
        std::string extracted = fasta_seq.substr(begin - 1, expected_len);
        if (extracted != design) {
            throw std::runtime_error(
                "Row " + std::to_string(row + 1) + ": design mismatch.\n" +
                "  CSV design:  " + design + "\n" +
                "  FASTA[" + std::to_string(begin) + ":" + std::to_string(end) + "]: " + extracted);
        }

        row++;
    }

    std::cout << "Verified " << row << " sequences: begin/end columns match design in FASTA.\n";
}

static void _stack_csv_files(
    const std::vector<std::string>& csv_files,
    const std::string& output_file
) {
    std::ofstream out(output_file);
    bool header_written = false;

    for (const auto& csv_file : csv_files) {
        std::ifstream in(csv_file);
        std::string line;
        bool first_line = true;

        while (std::getline(in, line)) {
            if (first_line) {
                first_line = false;
                if (!header_written) {
                    out << line << "\n";
                    header_written = true;
                }
                continue;
            }
            if (!line.empty()) {
                out << line << "\n";
            }
        }
    }
}

void _pipeline(const PipelineConfig& config) {
    // Check/create output directory
    if (std::filesystem::exists(config.output_dir)) {
        if (!config.overwrite) {
            throw std::runtime_error("Output directory already exists: " + config.output_dir +
                "\nUse --overwrite to replace it.");
        }
        std::filesystem::remove_all(config.output_dir);
    }
    std::filesystem::create_directories(config.output_dir);

    std::string tmp_dir = config.output_dir + "/tmp";
    std::filesystem::create_directories(tmp_dir);

    // Validate input files
    for (const auto& input : config.inputs) {
        if (!std::filesystem::exists(input)) {
            throw std::runtime_error("Input file does not exist: " + input);
        }
    }

    std::vector<std::string> fasta_files = config.inputs;
    std::sort(fasta_files.begin(), fasta_files.end());

    std::cout << "\n===== Pipeline: Found " << fasta_files.size() << " FASTA file(s) =====\n\n";

    // Step 1: Optionally generate M2-seq complements
    std::vector<std::string> all_fasta_files = fasta_files;
    if (config.generate_m2) {
        std::cout << "----- Generating M2-seq complements -----\n\n";
        for (const auto& fasta : fasta_files) {
            std::string basename = std::filesystem::path(fasta).stem().string();
            std::string m2_output = tmp_dir + "/" + basename + "_m2.fasta";
            _m2(fasta, m2_output, false, true);  // complements only
            all_fasta_files.push_back(m2_output);
        }
    }

    // Step 2: Preprocess all FASTA files
    std::cout << "----- Preprocessing -----\n\n";
    std::vector<std::string> csv_files;

    for (const auto& fasta : all_fasta_files) {
        std::string basename = std::filesystem::path(fasta).stem().string();
        std::string csv_output = tmp_dir + "/" + basename + ".csv";

        std::cout << "  " << basename << "...\n";
        _preprocess(fasta, csv_output, true, basename);
        csv_files.push_back(csv_output);
    }

    // Step 3: Stack CSV files
    std::cout << "\n----- Stacking CSV files -----\n\n";
    std::string stacked_csv = tmp_dir + "/preprocessed.csv";
    _stack_csv_files(csv_files, stacked_csv);

    // Count sequences
    size_t seq_count = 0;
    {
        std::ifstream in(stacked_csv);
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty()) seq_count++;
        }
        seq_count--;  // Subtract header
    }
    std::cout << "  Total sequences: " << seq_count << "\n";

    // Step 4: Design (add padding)
    std::cout << "\n----- Designing library -----\n\n";
    DesignConfig design_config;
    design_config.input_path = stacked_csv;
    design_config.output_prefix = tmp_dir + "/library";
    design_config.overwrite = true;
    design_config.pad_to_length = config.pad_to;
    design_config.five_const = config.five_const;
    design_config.three_const = config.three_const;
    design_config.stem = config.stem;
    design_config.barcode.stem_length = 0;  // No barcodes in design step
    design_config.barcode.stem = config.stem;

    _design(design_config);

    // Step 5: Generate barcodes (if enabled)
    std::string final_library = tmp_dir + "/library";

    if (!config.no_barcodes && config.barcode_length > 0) {
        std::cout << "\n----- Generating barcodes -----\n\n";
        std::string barcodes_file = tmp_dir + "/barcodes.txt";
        _barcodes(seq_count, barcodes_file, true, config.barcode_length, config.stem);

        if (config.predict) {
            // Step 6: Run rn-coverage prediction
            std::cout << "\n----- Running read count prediction -----\n\n";

            // Check prerequisites
            if (!_command_exists("rn-coverage")) {
                throw std::runtime_error(
                    "rn-coverage not found on PATH. Install it and add to PATH, "
                    "or run without --predict and follow manual instructions.");
            }

            std::string predict_dir = tmp_dir + "/predictions";
            std::filesystem::create_directories(predict_dir);

            // Generate .txt for rn-coverage tokenization
            _to_txt(final_library + ".csv", final_library, true);

            // Tokenize library
            std::string library_tokens = tmp_dir + "/library_tokens.h5";
            std::cout << "Tokenizing library...\n";
            if (_run_command("rn-coverage tokenize " + final_library + ".txt " + library_tokens) != 0) {
                throw std::runtime_error("Failed to tokenize library sequences");
            }

            // Tokenize barcodes
            std::string barcode_tokens = tmp_dir + "/barcode_tokens.h5";
            std::cout << "\nTokenizing barcodes...\n";
            if (_run_command("rn-coverage tokenize " + barcodes_file + " " + barcode_tokens) != 0) {
                throw std::runtime_error("Failed to tokenize barcode sequences");
            }

            // Set up prediction output directories
            std::string library_pred_dir = predict_dir + "/library";
            std::string barcode_pred_dir = predict_dir + "/barcodes";

            // Run predictions
            std::cout << "\nPredicting library read counts...\n";
            if (_run_command("rn-coverage predict " + library_tokens + " -o " + library_pred_dir) != 0) {
                throw std::runtime_error("Failed to predict library read counts");
            }

            std::cout << "\nPredicting barcode read counts...\n";
            if (_run_command("rn-coverage predict " + barcode_tokens + " -o " + barcode_pred_dir) != 0) {
                throw std::runtime_error("Failed to predict barcode read counts");
            }

            // Extract predictions to text files
            std::string library_reads = tmp_dir + "/library_reads.txt";
            std::string barcode_reads = tmp_dir + "/barcode_reads.txt";

            // The prediction output file has the same name as the input .h5 file
            std::string library_pred_h5 = library_pred_dir + "/library_tokens.h5";
            std::string barcode_pred_h5 = barcode_pred_dir + "/barcode_tokens.h5";

            std::cout << "\nExtracting predictions...\n";
            if (_run_command("rn-coverage extract " + library_pred_h5 + " " + library_reads) != 0) {
                throw std::runtime_error("Failed to extract library predictions");
            }
            if (_run_command("rn-coverage extract " + barcode_pred_h5 + " " + barcode_reads) != 0) {
                throw std::runtime_error("Failed to extract barcode predictions");
            }

            // Step 7: Merge barcodes with read-count balancing
            std::cout << "\n----- Merging barcodes with read-count balancing -----\n\n";
            std::string merged_prefix = tmp_dir + "/merged";
            _merge(final_library + ".csv", library_reads, barcodes_file, barcode_reads, merged_prefix, true, config.sort_by_reads);

            // Step 8: Predict reads for merged sequences
            std::cout << "\n----- Predicting final read counts -----\n\n";

            // Generate .txt for rn-coverage tokenization
            _to_txt(merged_prefix + ".csv", merged_prefix, true);

            std::string merged_tokens = tmp_dir + "/merged_tokens.h5";
            std::cout << "Tokenizing merged sequences...\n";
            if (_run_command("rn-coverage tokenize " + merged_prefix + ".txt " + merged_tokens) != 0) {
                throw std::runtime_error("Failed to tokenize merged sequences");
            }

            std::string merged_pred_dir = predict_dir + "/merged";

            std::cout << "\nPredicting merged read counts...\n";
            if (_run_command("rn-coverage predict " + merged_tokens + " -o " + merged_pred_dir) != 0) {
                throw std::runtime_error("Failed to predict merged read counts");
            }

            std::string merged_pred_h5 = merged_pred_dir + "/merged_tokens.h5";
            std::string merged_reads = tmp_dir + "/merged_reads.txt";

            std::cout << "\nExtracting predictions...\n";
            if (_run_command("rn-coverage extract " + merged_pred_h5 + " " + merged_reads) != 0) {
                throw std::runtime_error("Failed to extract merged predictions");
            }

            // Step 9: Sort by final read counts
            std::cout << "\n----- Sorting by final read counts -----\n\n";
            std::string final_output = config.output_dir + "/library";
            _sort(merged_prefix + ".csv", merged_reads, final_output, true, false, config.sort_by_reads);

        } else {
            // No prediction - print manual instructions
            std::cout << "\n----- Barcodes generated -----\n";
            std::cout << "  Library: " << final_library << ".csv\n";
            std::cout << "  Barcodes: " << barcodes_file << "\n\n";
            std::cout << "To complete the pipeline with read-count balancing:\n";
            std::cout << "  1. Generate plain text sequences:\n";
            std::cout << "     fld txt --output " << final_library << " " << final_library << ".csv\n";
            std::cout << "  2. Run your read prediction tool on:\n";
            std::cout << "     - " << final_library << ".txt\n";
            std::cout << "     - " << barcodes_file << "\n";
            std::cout << "  3. Save read counts to text files (one count per line)\n";
            std::cout << "  4. Run: fld merge --library " << final_library << ".csv \\\n";
            std::cout << "          --library-reads <library_reads.txt> \\\n";
            std::cout << "          --barcodes " << barcodes_file << " \\\n";
            std::cout << "          --barcode-reads <barcode_reads.txt> \\\n";
            std::cout << "          -o " << config.output_dir << "/library\n";
        }
    } else {
        // No barcodes - copy final library to output directory
        std::filesystem::copy(final_library + ".csv",
            config.output_dir + "/library.csv",
            std::filesystem::copy_options::overwrite_existing);
        std::filesystem::copy(final_library + ".fasta",
            config.output_dir + "/library.fasta",
            std::filesystem::copy_options::overwrite_existing);
    }

    // Generate RNA version with T7 promoter prefix
    std::string library_csv = config.output_dir + "/library.csv";
    std::string library_fasta = config.output_dir + "/library.fasta";
    std::string library_rna_fasta = config.output_dir + "/library-rna.fasta";
    if (std::filesystem::exists(library_fasta)) {
        std::cout << "\n----- Generating RNA library with T7 prefix -----\n\n";
        _prepend(library_fasta, library_rna_fasta, "GGGAACG", true);
    }

    // Sanity check: verify begin/end columns match design in FASTA
    if (std::filesystem::exists(library_csv) && std::filesystem::exists(library_fasta)) {
        std::cout << "\n----- Verifying begin/end columns -----\n\n";
        _check_design_against_fasta(library_csv, library_fasta);
    }

    std::cout << "\n===== Pipeline complete =====\n";
}
