#include "pipeline.hpp"
#include "preprocess.hpp"
#include "library.hpp"
#include "barcodes.hpp"
#include "padding.hpp"
#include "merge.hpp"
#include "merge_padding.hpp"
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
        "With --predict: predict reads for padding, designs, and barcodes;\n"
        "                merge padding and barcodes with read balancing;\n"
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
    std::string header_line;
    std::getline(csv_file, header_line);

    csv::Header header(header_line);
    header.validate();

    // Check if begin/end columns are present
    if (!header.has(csv::COL_BEGIN) || !header.has(csv::COL_END)) {
        std::cout << "Skipping begin/end verification (columns not present in CSV).\n";
        return;
    }

    std::string line;
    size_t row = 0;
    while (std::getline(csv_file, line)) {
        if (line.empty()) continue;

        std::vector<std::string> fields = _split_by_delimiter(line, ',');

        std::string design = header.get(fields, csv::COL_DESIGN);
        std::string begin_str = header.get(fields, csv::COL_BEGIN);
        std::string end_str = header.get(fields, csv::COL_END);

        if (begin_str.empty() || end_str.empty()) {
            throw std::runtime_error("CSV row " + std::to_string(row + 1) +
                " has empty begin/end values");
        }

        size_t begin = std::stoull(begin_str);
        size_t end = std::stoull(end_str);

        // Check length sanity (end is exclusive, so end - begin = design.size())
        size_t expected_len = end - begin;
        if (expected_len != design.size()) {
            throw std::runtime_error(
                "Row " + std::to_string(row + 1) + ": begin/end length mismatch. " +
                "end - begin = " + std::to_string(expected_len) +
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

// Write a .txt file by concatenating specific CSV columns per row.
static void _csv_columns_to_txt(
    const std::string& csv_path,
    const std::string& output_path,
    const std::vector<std::string>& columns
) {
    std::ifstream in(csv_path);
    std::string header_line;
    std::getline(in, header_line);
    csv::Header header(header_line);

    std::ofstream out(output_path);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::vector<std::string> fields = _split_by_delimiter(line, ',');
        std::string seq;
        for (const auto& col : columns) {
            seq += header.get(fields, col);
        }
        out << seq << "\n";
    }
}

// Run tokenize + predict + extract for a single sequence set.
// Returns the path to the extracted reads .txt file.
static std::string _predict_reads(
    const std::string& input_txt,
    const std::string& tokens_path,
    const std::string& pred_dir,
    const std::string& reads_path,
    const std::string& label
) {
    std::cout << "Tokenizing " << label << "...\n";
    if (_run_command("rn-coverage tokenize " + input_txt + " " + tokens_path) != 0) {
        throw std::runtime_error("Failed to tokenize " + label);
    }

    std::cout << "\nPredicting " << label << " read counts...\n";
    if (_run_command("rn-coverage predict " + tokens_path + " -o " + pred_dir) != 0) {
        throw std::runtime_error("Failed to predict " + label + " read counts");
    }

    // The prediction output .h5 has the same basename as the input .h5
    std::string tokens_basename = std::filesystem::path(tokens_path).filename().string();
    std::string pred_h5 = pred_dir + "/" + tokens_basename;

    std::cout << "\nExtracting " << label << " predictions...\n";
    if (_run_command("rn-coverage extract " + pred_h5 + " " + reads_path) != 0) {
        throw std::runtime_error("Failed to extract " + label + " predictions");
    }

    return reads_path;
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

    // Step 4: Design
    std::cout << "\n----- Designing library -----\n\n";
    DesignConfig design_config;
    design_config.input_path = stacked_csv;
    design_config.output_prefix = tmp_dir + "/library";
    design_config.overwrite = true;
    design_config.pad_to_length = config.pad_to;
    design_config.stem = config.stem;
    // When predicting, skip padding and barcodes — they are generated separately
    // for read-count balancing. Also leave constants empty so intermediate
    // predictions only see the components finalized so far.
    if (config.predict) {
        design_config.skip_padding = true;
        design_config.barcode.stem_length = 0;
        design_config.five_const = "";
        design_config.three_const = "";
    } else {
        design_config.five_const = config.five_const;
        design_config.three_const = config.three_const;
        design_config.barcode.stem_length = config.no_barcodes ? 0 : config.barcode_length;
    }
    design_config.barcode.stem = config.stem;

    _design(design_config);

    std::string final_library = tmp_dir + "/library";

    // With --predict: 5-step read-count balancing for padding and barcodes
    if (config.predict && !config.no_barcodes && config.barcode_length > 0) {

        // Check prerequisites
        if (!_command_exists("rn-coverage")) {
            throw std::runtime_error(
                "rn-coverage not found on PATH. Install it and add to PATH, "
                "or run without --predict and follow manual instructions.");
        }

        std::string predict_dir = tmp_dir + "/predictions";
        std::filesystem::create_directories(predict_dir);

        // Generate padding and barcodes separately
        std::cout << "\n----- Generating padding for read-count balancing -----\n\n";
        std::string padding_file = tmp_dir + "/padding.txt";
        _generate_padding(final_library + ".csv", config.pad_to, config.stem, padding_file, true);

        std::cout << "\n----- Generating barcodes for read-count balancing -----\n\n";
        std::string barcodes_file = tmp_dir + "/barcodes.txt";
        _barcodes(seq_count, barcodes_file, true, config.barcode_length, config.stem);

        // Extract design-only sequences for prediction
        std::string designs_txt = tmp_dir + "/designs.txt";
        _csv_columns_to_txt(final_library + ".csv", designs_txt, {csv::COL_DESIGN});

        // ===== PREDICT 1: padding sequences alone =====
        std::cout << "\n----- [1/5] Predicting padding read counts -----\n\n";
        std::string padding_reads = tmp_dir + "/padding_reads.txt";
        _predict_reads(padding_file,
            tmp_dir + "/padding_tokens.h5",
            predict_dir + "/padding",
            padding_reads, "padding");

        // ===== PREDICT 2: design sequences alone =====
        std::cout << "\n----- [2/5] Predicting design read counts -----\n\n";
        std::string design_reads = tmp_dir + "/design_reads.txt";
        _predict_reads(designs_txt,
            tmp_dir + "/design_tokens.h5",
            predict_dir + "/designs",
            design_reads, "designs");

        // ===== PREDICT 3: barcode sequences alone =====
        std::cout << "\n----- [3/5] Predicting barcode read counts -----\n\n";
        std::string barcode_reads = tmp_dir + "/barcode_reads.txt";
        _predict_reads(barcodes_file,
            tmp_dir + "/barcode_tokens.h5",
            predict_dir + "/barcodes",
            barcode_reads, "barcodes");

        // ===== MERGE ROUND 1: attach padding =====
        std::cout << "\n----- Merging padding with read-count balancing -----\n\n";
        std::string padded_prefix = tmp_dir + "/padded";
        _merge_padding(final_library + ".csv", design_reads, padding_file, padding_reads,
            padded_prefix, true);

        // ===== PREDICT 4: padding + design (no constants, no barcodes) =====
        std::cout << "\n----- [4/5] Predicting padded design read counts -----\n\n";
        std::string padded_txt = tmp_dir + "/padded.txt";
        _csv_columns_to_txt(padded_prefix + ".csv", padded_txt,
            {csv::COL_FIVE_PADDING, csv::COL_DESIGN});

        std::string padded_reads = tmp_dir + "/padded_reads.txt";
        _predict_reads(padded_txt,
            tmp_dir + "/padded_tokens.h5",
            predict_dir + "/padded",
            padded_reads, "padded designs");

        // ===== MERGE ROUND 2: attach barcodes =====
        std::cout << "\n----- Merging barcodes with read-count balancing -----\n\n";
        std::string merged_prefix = tmp_dir + "/merged";
        _merge(padded_prefix + ".csv", padded_reads, barcodes_file, barcode_reads,
            merged_prefix, true, config.sort_by_reads);

        // ===== PREDICT 5: final full library (with constants) =====
        // Constants were left empty in the CSV so intermediate predictions
        // excluded them. Write the final .txt with constants prepended/appended.
        std::cout << "\n----- [5/5] Predicting final read counts -----\n\n";
        std::string final_txt = tmp_dir + "/final.txt";
        {
            std::ifstream fin(merged_prefix + ".csv");
            std::string hdr_line;
            std::getline(fin, hdr_line);
            csv::Header hdr(hdr_line);

            std::ofstream fout(final_txt);
            std::string fline;
            while (std::getline(fin, fline)) {
                if (fline.empty()) continue;
                std::vector<std::string> fields = _split_by_delimiter(fline, ',');
                std::string seq = config.five_const +
                                  hdr.get(fields, csv::COL_FIVE_PADDING) +
                                  hdr.get(fields, csv::COL_DESIGN) +
                                  hdr.get(fields, csv::COL_THREE_PADDING) +
                                  hdr.get(fields, csv::COL_BARCODE) +
                                  config.three_const;
                fout << seq << "\n";
            }
        }

        std::string final_reads = tmp_dir + "/final_reads.txt";
        _predict_reads(final_txt,
            tmp_dir + "/final_tokens.h5",
            predict_dir + "/final",
            final_reads, "final library");

        // Fill in constant regions in the merged CSV before final output
        {
            std::ifstream fin(merged_prefix + ".csv");
            std::string hdr_line;
            std::getline(fin, hdr_line);
            csv::Header hdr(hdr_line);

            int five_idx = hdr.index_of(csv::COL_FIVE_CONST);
            int three_idx = hdr.index_of(csv::COL_THREE_CONST);

            std::string primerized_csv = merged_prefix + "_primerized.csv";
            std::ofstream fout(primerized_csv);
            fout << hdr_line << "\n";
            std::string fline;
            while (std::getline(fin, fline)) {
                if (fline.empty()) continue;
                std::vector<std::string> fields = _split_by_delimiter(fline, ',');
                if (five_idx >= 0 && static_cast<size_t>(five_idx) < fields.size()) {
                    fields[five_idx] = config.five_const;
                }
                if (three_idx >= 0 && static_cast<size_t>(three_idx) < fields.size()) {
                    fields[three_idx] = config.three_const;
                }
                for (size_t i = 0; i < fields.size(); i++) {
                    fout << _quote_csv_field(fields[i]);
                    if (i < fields.size() - 1) fout << ",";
                }
                fout << "\n";
            }
            fout.close();

            // Replace merged CSV with primerized version
            std::filesystem::rename(primerized_csv, merged_prefix + ".csv");
        }

        // Sort by final read counts
        std::cout << "\n----- Sorting by final read counts -----\n\n";
        std::string final_output = config.output_dir + "/library";
        _sort(merged_prefix + ".csv", final_reads, final_output, true, false, config.sort_by_reads);

    } else {
        // No prediction - copy designed library (with barcodes if enabled) to output
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
    std::string library_rna_fasta = config.output_dir + "/t7-library.fasta";
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
