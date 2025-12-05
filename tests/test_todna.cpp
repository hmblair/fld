#include "doctest.hpp"
#include "test_helpers.hpp"
#include "todna.hpp"
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

TEST_CASE("to-dna converts U to T") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    write_fasta(input_path, {
        {"seq1", "ACGU"},
        {"seq2", "UUUU"}
    });

    _to_dna(input_path, output_path, true);

    auto seqs = read_fasta(output_path);
    REQUIRE(seqs.size() == 2);
    CHECK(seqs[0].second == "ACGT");
    CHECK(seqs[1].second == "TTTT");
}

TEST_CASE("to-dna converts lowercase u to t") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    write_fasta(input_path, {{"seq1", "acgu"}});

    _to_dna(input_path, output_path, true);

    auto seqs = read_fasta(output_path);
    REQUIRE(seqs.size() == 1);
    CHECK(seqs[0].second == "acgt");
}

TEST_CASE("to-dna preserves headers") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    write_fasta(input_path, {
        {"gene_with_U", "ACGU"},
        {"sequence_U_name", "UGCA"}
    });

    _to_dna(input_path, output_path, true);

    auto seqs = read_fasta(output_path);
    REQUIRE(seqs.size() == 2);
    // Headers should NOT be converted
    CHECK(seqs[0].first == "gene_with_U");
    CHECK(seqs[1].first == "sequence_U_name");
    // But sequences should be converted
    CHECK(seqs[0].second == "ACGT");
    CHECK(seqs[1].second == "TGCA");
}

TEST_CASE("to-dna handles sequence with no U") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    write_fasta(input_path, {{"seq1", "ACGT"}});

    _to_dna(input_path, output_path, true);

    auto seqs = read_fasta(output_path);
    REQUIRE(seqs.size() == 1);
    CHECK(seqs[0].second == "ACGT");
}
