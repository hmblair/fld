#include "doctest.hpp"
#include "test_helpers.hpp"
#include "pipeline.hpp"
#include <filesystem>
#include <cstdlib>

// Check if rn-coverage is available and working
static bool rn_coverage_available() {
    // Check that rn-coverage exists and can import its dependencies
    // We use a simple Python import check since --help returns non-zero
    return std::system("which rn-coverage > /dev/null 2>&1 && "
                       "python3 -c 'import h5py' > /dev/null 2>&1") == 0;
}

// RAII helper to clean up PyTorch Lightning logs created during tests
struct LightningLogsCleaner {
    ~LightningLogsCleaner() {
        std::filesystem::remove_all("lightning_logs");
    }
};

TEST_CASE("pipeline with --predict" * doctest::skip(!rn_coverage_available())) {
    LightningLogsCleaner log_cleaner;  // Clean up lightning_logs when test ends
    TempDir tmpdir;
    std::string input_fasta = tmpdir.path() + "/input.fasta";
    std::string output_dir = tmpdir.path() + "/output";

    // Create a small test FASTA with a few sequences
    write_fasta(input_fasta, {
        {"seq1", "ACGTACGTACGTACGTACGT"},
        {"seq2", "TGCATGCATGCATGCATGCA"},
        {"seq3", "AAAACCCCGGGGTTTTAAAA"}
    });

    // Run pipeline with prediction
    PipelineConfig config;
    config.inputs = {input_fasta};
    config.output_dir = output_dir;
    config.overwrite = true;
    config.pad_to = 130;
    config.five_const = "ACTCGAGTAGAGTCGAAAA";
    config.three_const = "AAAAGAAACAACAACAACAAC";
    config.stem.min_length = 7;
    config.stem.max_length = 13;
    config.stem.max_au = INT_MAX;
    config.stem.max_gc = 5;
    config.stem.max_gu = 0;
    config.stem.closing_gc = 1;
    config.stem.spacer_length = 2;
    config.barcode_length = 10;
    config.no_barcodes = false;
    config.generate_m2 = false;
    config.predict = true;
    config.sort_by_reads = false;

    // This should not throw
    REQUIRE_NOTHROW(_pipeline(config));

    // Check output files exist
    CHECK(std::filesystem::exists(output_dir + "/library.csv"));
    CHECK(std::filesystem::exists(output_dir + "/library.fasta"));
}

TEST_CASE("pipeline without --predict") {
    TempDir tmpdir;
    std::string input_fasta = tmpdir.path() + "/input.fasta";
    std::string output_dir = tmpdir.path() + "/output";

    // Create a small test FASTA
    write_fasta(input_fasta, {
        {"seq1", "ACGTACGTACGTACGTACGT"},
        {"seq2", "TGCATGCATGCATGCATGCA"}
    });

    // Run pipeline without prediction
    PipelineConfig config;
    config.inputs = {input_fasta};
    config.output_dir = output_dir;
    config.overwrite = true;
    config.pad_to = 130;
    config.five_const = "ACTCGAGTAGAGTCGAAAA";
    config.three_const = "AAAAGAAACAACAACAACAAC";
    config.stem.min_length = 7;
    config.stem.max_length = 13;
    config.stem.max_au = INT_MAX;
    config.stem.max_gc = 5;
    config.stem.max_gu = 0;
    config.stem.closing_gc = 1;
    config.stem.spacer_length = 2;
    config.barcode_length = 10;
    config.no_barcodes = false;
    config.generate_m2 = false;
    config.predict = false;
    config.sort_by_reads = false;

    REQUIRE_NOTHROW(_pipeline(config));

    // Check tmp files exist (no final output without --predict and barcodes)
    CHECK(std::filesystem::exists(output_dir + "/tmp"));
}
