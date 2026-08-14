#include "doctest.hpp"
#include "test_helpers.hpp"
#include "m2.hpp"
#include <fstream>

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

TEST_CASE("m2 on RNA input does not mix U and T") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    // "A" at position 0 complements to a base that must be written as "U",
    // not "T", to stay consistent with the rest of this RNA-form sequence.
    write_fasta(input_path, {{"seq1", "ACGUACGUAC"}});

    _m2(input_path, output_path, false, true);

    auto seqs = read_fasta(output_path);
    for (const auto& [name, seq] : seqs) {
        CHECK(seq.find('T') == std::string::npos);
    }

    // Position 0 (A) should complement to U, not T.
    bool found = false;
    for (const auto& [name, seq] : seqs) {
        if (name == "seq1_mm_0_A_T" || name == "seq1_mm_0_A_U") {
            found = true;
            CHECK(seq[0] == 'U');
        }
    }
    CHECK(found);
}

TEST_CASE("m2 on DNA input does not introduce U") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    write_fasta(input_path, {{"seq1", "ACGTACGTAC"}});

    _m2(input_path, output_path, false, true);

    auto seqs = read_fasta(output_path);
    for (const auto& [name, seq] : seqs) {
        CHECK(seq.find('U') == std::string::npos);
    }
}

TEST_CASE("m2 default mode produces exactly N+1 records for length-N input") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    write_fasta(input_path, {{"seq1", "ACGUACGUAC"}});

    _m2(input_path, output_path, false, true);

    auto seqs = read_fasta(output_path);
    // 1 wild-type record + 1 mutant per position
    CHECK(seqs.size() == 11);
}

TEST_CASE("m2 --all on RNA input produces 3 real mutants per position, no T") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    write_fasta(input_path, {{"seq1", "ACGUACGUAC"}});

    _m2(input_path, output_path, true, true);

    auto seqs = read_fasta(output_path);
    // 1 wild-type + 3 real mutants per position (not 4 -- "U->T" is not a
    // real mutation, it's the same base in a different alphabet)
    CHECK(seqs.size() == 1 + 10 * 3);
    for (const auto& [name, seq] : seqs) {
        CHECK(seq.find('T') == std::string::npos);
    }
}
