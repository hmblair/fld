#include "doctest.hpp"
#include "test_helpers.hpp"
#include "prepend.hpp"
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

TEST_CASE("prepend adds sequence to beginning of all entries") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    write_fasta(input_path, {
        {"seq1", "ACGT"},
        {"seq2", "TGCA"}
    });

    _prepend(input_path, output_path, "GGGAAA", true);

    auto seqs = read_fasta(output_path);
    REQUIRE(seqs.size() == 2);
    CHECK(seqs[0].first == "seq1");
    CHECK(seqs[0].second == "GGGAAAACGT");
    CHECK(seqs[1].first == "seq2");
    CHECK(seqs[1].second == "GGGAAATGCA");
}

TEST_CASE("prepend preserves header names") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    write_fasta(input_path, {
        {"gene_name with spaces", "ACGT"},
        {"another|header|format", "TGCA"}
    });

    _prepend(input_path, output_path, "AAA", true);

    auto seqs = read_fasta(output_path);
    REQUIRE(seqs.size() == 2);
    CHECK(seqs[0].first == "gene_name with spaces");
    CHECK(seqs[1].first == "another|header|format");
}

TEST_CASE("prepend handles empty prefix") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    write_fasta(input_path, {{"seq1", "ACGT"}});

    _prepend(input_path, output_path, "", true);

    auto seqs = read_fasta(output_path);
    REQUIRE(seqs.size() == 1);
    CHECK(seqs[0].second == "ACGT");
}

TEST_CASE("prepend handles long sequences") {
    TempDir tmpdir;
    std::string input_path = tmpdir.path() + "/input.fasta";
    std::string output_path = tmpdir.path() + "/output.fasta";

    std::string long_seq(500, 'A');
    write_fasta(input_path, {{"long_seq", long_seq}});

    std::string prefix = "GGGCCC";
    _prepend(input_path, output_path, prefix, true);

    auto seqs = read_fasta(output_path);
    REQUIRE(seqs.size() == 1);
    CHECK(seqs[0].second.length() == prefix.length() + long_seq.length());
    CHECK(seqs[0].second.substr(0, prefix.length()) == prefix);
}
