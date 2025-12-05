#include "doctest.hpp"
#include "test_helpers.hpp"
#include "torna.hpp"
#include <fstream>

// Helper to read FASTA sequences back
static std::vector<std::pair<std::string, std::string>> read_fasta(const std::string& path) {
    std::vector<std::pair<std::string, std::string>> result;
    std::ifstream file(path);
    std::string line;
    std::string current_name;
    std::string current_seq;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line[0] == '>') {
            if (!current_name.empty()) {
                result.push_back({current_name, current_seq});
            }
            current_name = line.substr(1);
            current_seq.clear();
        } else {
            current_seq += line;
        }
    }
    if (!current_name.empty()) {
        result.push_back({current_name, current_seq});
    }
    return result;
}

TEST_CASE("to-rna converts T to U") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    write_fasta(input_path, {
        {"seq1", "ACGT"},
        {"seq2", "TTTT"}
    });

    _to_rna(input_path, output_path, true);

    auto seqs = read_fasta(output_path);
    REQUIRE(seqs.size() == 2);
    CHECK(seqs[0].second == "ACGU");
    CHECK(seqs[1].second == "UUUU");
}

TEST_CASE("to-rna converts lowercase t to u") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    write_fasta(input_path, {{"seq1", "acgt"}});

    _to_rna(input_path, output_path, true);

    auto seqs = read_fasta(output_path);
    REQUIRE(seqs.size() == 1);
    CHECK(seqs[0].second == "acgu");
}

TEST_CASE("to-rna handles mixed case") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    write_fasta(input_path, {{"seq1", "AcGtTaCg"}});

    _to_rna(input_path, output_path, true);

    auto seqs = read_fasta(output_path);
    REQUIRE(seqs.size() == 1);
    CHECK(seqs[0].second == "AcGuUaCg");
}

TEST_CASE("to-rna preserves headers") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    write_fasta(input_path, {
        {"gene_TATA_box", "ACGT"},
        {"sequence_with_T", "TGCA"}
    });

    _to_rna(input_path, output_path, true);

    auto seqs = read_fasta(output_path);
    REQUIRE(seqs.size() == 2);
    // Headers should NOT be converted
    CHECK(seqs[0].first == "gene_TATA_box");
    CHECK(seqs[1].first == "sequence_with_T");
    // But sequences should be converted
    CHECK(seqs[0].second == "ACGU");
    CHECK(seqs[1].second == "UGCA");
}

TEST_CASE("to-rna handles sequence with no T") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    write_fasta(input_path, {{"seq1", "ACGACG"}});

    _to_rna(input_path, output_path, true);

    auto seqs = read_fasta(output_path);
    REQUIRE(seqs.size() == 1);
    CHECK(seqs[0].second == "ACGACG");
}

TEST_CASE("to-rna handles already RNA sequence") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    // Input is already RNA (has U instead of T)
    write_fasta(input_path, {{"seq1", "ACGU"}});

    _to_rna(input_path, output_path, true);

    auto seqs = read_fasta(output_path);
    REQUIRE(seqs.size() == 1);
    CHECK(seqs[0].second == "ACGU");
}
