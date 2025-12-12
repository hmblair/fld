#include "doctest.hpp"
#include "test_helpers.hpp"
#include "library.hpp"
#include "preprocess.hpp"
#include "config/design_config.hpp"

TEST_CASE("design produces sequences of correct padded length") {
    std::mt19937 gen(42);  // Fixed seed for reproducibility

    // 1. Random parameters
    size_t N = random_range(50, 200, gen);
    size_t K = random_range(5, 20, gen);

    // 2. Generate random sequences
    TempDir tmpdir;
    std::string fasta_path = tmpdir.path() + "/input.fasta";
    std::string csv_path = tmpdir.path() + "/preprocessed.csv";
    std::string output_prefix = tmpdir.path() + "/output";

    write_random_fasta(fasta_path, K, N, gen);

    // 3-4. Preprocess
    _preprocess(fasta_path, csv_path, true, "test");

    // 5-6. Design
    DesignConfig config;
    config.input_path = csv_path;
    config.output_prefix = output_prefix;
    config.overwrite = true;
    config.pad_to_length = N;
    config.barcode.stem_length = 0;  // No barcoding for this test
    _design(config);

    // 7. Read output
    Library library = _from_csv(output_prefix + ".csv");

    // 8. Verify lengths
    size_t expected = N + config.five_const.length() + config.three_const.length();

    INFO("N = " << N << ", K = " << K << ", expected length = " << expected);

    CHECK(library.size() == K);

    // Note: Library doesn't expose individual constructs via operator[],
    // so we need to verify via the output files
    // Let's read the output CSV directly and check lengths
    std::ifstream file(output_prefix + ".csv");
    std::string line;
    std::getline(file, line);  // Skip header

    size_t count = 0;
    while (std::getline(file, line)) {
        // Parse the CSV line to extract all sequence parts
        // Columns: index,name,sublibrary,five_const,five_padding,design,three_padding,barcode,three_const
        std::vector<std::string> parts;
        std::string current;
        bool in_quotes = false;

        for (char c : line) {
            if (c == '"') {
                in_quotes = !in_quotes;
            } else if (c == ',' && !in_quotes) {
                parts.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }
        parts.push_back(current);

        if (parts.size() >= 9) {
            // Calculate total length: five_const + five_padding + design + three_padding + barcode + three_const
            // Columns: index,name,sublibrary,five_const,five_padding,design,three_padding,barcode,three_const
            size_t total_length = parts[3].length() + parts[4].length() +
                                  parts[5].length() + parts[6].length() +
                                  parts[7].length() + parts[8].length();

            INFO("Sequence " << count << ": total_length = " << total_length);
            CHECK(total_length == expected);
        }
        count++;
    }

    CHECK(count == K);
}

TEST_CASE("design with barcoding produces unique barcodes") {
    std::mt19937 gen(123);  // Different seed

    size_t N = 100;
    size_t K = 10;

    TempDir tmpdir;
    std::string fasta_path = tmpdir.path() + "/input.fasta";
    std::string csv_path = tmpdir.path() + "/preprocessed.csv";
    std::string output_prefix = tmpdir.path() + "/output";

    write_random_fasta(fasta_path, K, N, gen);

    _preprocess(fasta_path, csv_path, true, "test");

    DesignConfig config;
    config.input_path = csv_path;
    config.output_prefix = output_prefix;
    config.overwrite = true;
    config.pad_to_length = N;
    config.barcode.stem_length = 8;  // Enable barcoding
    config.barcode.stem = config.stem;
    _design(config);

    // Read output and collect barcodes
    std::ifstream file(output_prefix + ".csv");
    std::string line;
    std::getline(file, line);  // Skip header

    std::unordered_set<std::string> barcodes;
    while (std::getline(file, line)) {
        std::vector<std::string> parts;
        std::string current;
        bool in_quotes = false;

        for (char c : line) {
            if (c == '"') {
                in_quotes = !in_quotes;
            } else if (c == ',' && !in_quotes) {
                parts.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }
        parts.push_back(current);

        if (parts.size() >= 8) {
            // Columns: index,name,sublibrary,five_const,five_padding,design,three_padding,barcode,three_const
            std::string barcode = parts[7];
            INFO("Barcode: " << barcode);
            CHECK(!barcode.empty());

            // Check barcode is unique
            CHECK(barcodes.find(barcode) == barcodes.end());
            barcodes.insert(barcode);
        }
    }

    CHECK(barcodes.size() == K);
}
