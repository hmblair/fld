#include "doctest.hpp"
#include "test_helpers.hpp"
#include "sort.hpp"
#include "preprocess.hpp"
#include "io/csv_format.hpp"
#include "utils.hpp"
#include <fstream>

// Helper to write a reads file
static void write_reads(const std::string& path, const std::vector<double>& reads) {
    std::ofstream file(path);
    for (double r : reads) {
        file << r << "\n";
    }
}

// Helper to parse index and sublibrary from CSV line
static std::pair<size_t, std::string> parse_index_sublibrary(const std::string& line) {
    auto fields = _split_by_delimiter(line, ',');
    size_t index = std::stoull(fields[csv::INDEX]);
    std::string sublibrary = fields[csv::SUBLIBRARY];
    return {index, sublibrary};
}

TEST_CASE("preprocess assigns 1-based indices per file") {
    TempDir tmpdir;
    std::string fasta1 = tmpdir.path() + "/lib1.fasta";
    std::string fasta2 = tmpdir.path() + "/lib2.fasta";
    std::string csv1 = tmpdir.path() + "/lib1.csv";
    std::string csv2 = tmpdir.path() + "/lib2.csv";

    // Create two FASTA files
    write_fasta(fasta1, {{"seq_a", "ACGT"}, {"seq_b", "TGCA"}, {"seq_c", "AAAA"}});
    write_fasta(fasta2, {{"seq_x", "GGGG"}, {"seq_y", "CCCC"}});

    // Preprocess each
    _preprocess(fasta1, csv1, true, "lib1");
    _preprocess(fasta2, csv2, true, "lib2");

    // Check lib1 indices: should be 1, 2, 3
    {
        std::ifstream file(csv1);
        std::string header, line;
        std::getline(file, header);

        std::getline(file, line);
        auto [idx1, sub1] = parse_index_sublibrary(line);
        CHECK(idx1 == 1);
        CHECK(sub1 == "lib1");

        std::getline(file, line);
        auto [idx2, sub2] = parse_index_sublibrary(line);
        CHECK(idx2 == 2);

        std::getline(file, line);
        auto [idx3, sub3] = parse_index_sublibrary(line);
        CHECK(idx3 == 3);
    }

    // Check lib2 indices: should restart at 1, 2
    {
        std::ifstream file(csv2);
        std::string header, line;
        std::getline(file, header);

        std::getline(file, line);
        auto [idx1, sub1] = parse_index_sublibrary(line);
        CHECK(idx1 == 1);
        CHECK(sub1 == "lib2");

        std::getline(file, line);
        auto [idx2, sub2] = parse_index_sublibrary(line);
        CHECK(idx2 == 2);
    }
}

TEST_CASE("sort without --sort-by-reads preserves input order") {
    TempDir tmpdir;
    std::string fasta1 = tmpdir.path() + "/lib1.fasta";
    std::string fasta2 = tmpdir.path() + "/lib2.fasta";
    std::string csv1 = tmpdir.path() + "/lib1.csv";
    std::string csv2 = tmpdir.path() + "/lib2.csv";
    std::string stacked = tmpdir.path() + "/stacked.csv";
    std::string reads_file = tmpdir.path() + "/reads.txt";
    std::string output = tmpdir.path() + "/sorted";

    // Create and preprocess two FASTA files
    write_fasta(fasta1, {{"seq_a", "ACGT"}, {"seq_b", "TGCA"}, {"seq_c", "AAAA"}});
    write_fasta(fasta2, {{"seq_x", "GGGG"}, {"seq_y", "CCCC"}});
    _preprocess(fasta1, csv1, true, "lib1");
    _preprocess(fasta2, csv2, true, "lib2");

    // Stack CSV files
    {
        std::ofstream out(stacked);
        out << csv::header() << "\n";

        std::ifstream in1(csv1);
        std::string line;
        std::getline(in1, line); // skip header
        while (std::getline(in1, line)) out << line << "\n";

        std::ifstream in2(csv2);
        std::getline(in2, line); // skip header
        while (std::getline(in2, line)) out << line << "\n";
    }

    // Create reads that would shuffle order if sorted by reads
    // lib1: seq_a=500, seq_b=100, seq_c=300
    // lib2: seq_x=200, seq_y=400
    write_reads(reads_file, {500, 100, 300, 200, 400});

    // Sort WITHOUT --sort-by-reads (default: preserve input order)
    _sort(stacked, reads_file, output, true, false, false);

    // Read output and verify order is preserved: lib1(1,2,3), lib2(1,2)
    std::ifstream file(output + ".csv");
    std::string header, line;
    std::getline(file, header);

    std::vector<std::pair<size_t, std::string>> results;
    while (std::getline(file, line)) {
        results.push_back(parse_index_sublibrary(line));
    }

    REQUIRE(results.size() == 5);

    // Should be in original order: lib1(1,2,3), lib2(1,2)
    CHECK(results[0].first == 1);
    CHECK(results[0].second == "lib1");
    CHECK(results[1].first == 2);
    CHECK(results[1].second == "lib1");
    CHECK(results[2].first == 3);
    CHECK(results[2].second == "lib1");
    CHECK(results[3].first == 1);
    CHECK(results[3].second == "lib2");
    CHECK(results[4].first == 2);
    CHECK(results[4].second == "lib2");
}

TEST_CASE("sort with --sort-by-reads sorts by read count") {
    TempDir tmpdir;
    std::string fasta = tmpdir.path() + "/input.fasta";
    std::string csv = tmpdir.path() + "/input.csv";
    std::string reads_file = tmpdir.path() + "/reads.txt";
    std::string output = tmpdir.path() + "/sorted";

    // Create FASTA with 3 sequences
    write_fasta(fasta, {{"seq_a", "ACGT"}, {"seq_b", "TGCA"}, {"seq_c", "AAAA"}});
    _preprocess(fasta, csv, true, "test");

    // Reads: seq_a=300, seq_b=100, seq_c=200
    // Ascending order should be: seq_b(100), seq_c(200), seq_a(300)
    write_reads(reads_file, {300, 100, 200});

    // Sort WITH --sort-by-reads
    _sort(csv, reads_file, output, true, false, true);

    // Read output and verify sorted by reads ascending
    std::ifstream file(output + ".csv");
    std::string header, line;
    std::getline(file, header);

    std::vector<size_t> indices;
    while (std::getline(file, line)) {
        auto [idx, sub] = parse_index_sublibrary(line);
        indices.push_back(idx);
    }

    REQUIRE(indices.size() == 3);

    // Sorted by reads ascending: seq_b(idx=2), seq_c(idx=3), seq_a(idx=1)
    CHECK(indices[0] == 2);  // seq_b has lowest reads (100)
    CHECK(indices[1] == 3);  // seq_c has middle reads (200)
    CHECK(indices[2] == 1);  // seq_a has highest reads (300)
}

TEST_CASE("sort preserves index column through sorting") {
    TempDir tmpdir;
    std::string fasta = tmpdir.path() + "/input.fasta";
    std::string csv = tmpdir.path() + "/input.csv";
    std::string reads_file = tmpdir.path() + "/reads.txt";
    std::string output = tmpdir.path() + "/sorted";

    write_fasta(fasta, {{"gene_A", "ACGT"}, {"gene_B", "TGCA"}});
    _preprocess(fasta, csv, true, "mylib");
    write_reads(reads_file, {200, 100});

    // Sort by reads (gene_B first due to lower reads)
    _sort(csv, reads_file, output, true, false, true);

    std::ifstream file(output + ".csv");
    std::string header, line;
    std::getline(file, header);

    // First line should be gene_B (lower reads) but index should still be 2
    std::getline(file, line);
    auto fields = _split_by_delimiter(line, ',');
    CHECK(fields[csv::INDEX] == "2");
    CHECK(fields[csv::NAME] == "gene_B");

    // Second line should be gene_A with index 1
    std::getline(file, line);
    fields = _split_by_delimiter(line, ',');
    CHECK(fields[csv::INDEX] == "1");
    CHECK(fields[csv::NAME] == "gene_A");
}
