#include "doctest.hpp"
#include "domain/barcode.hpp"
#include <random>

TEST_CASE("Barcode Hamming ball contains original") {
    Barcode bc("ACGTACGT");
    std::vector<std::string> ball = Barcode::hamming_ball(bc.str());

    // The ball should contain the original sequence
    bool contains_original = false;
    for (const auto& s : ball) {
        if (s == "ACGTACGT") {
            contains_original = true;
            break;
        }
    }
    CHECK(contains_original);
}

TEST_CASE("Barcode Hamming ball size is correct") {
    Barcode bc("ACGTACGT");  // 8 bases
    std::vector<std::string> ball = Barcode::hamming_ball(bc.str());

    // Ball should have length + 1 elements (one mutation per position + original)
    CHECK(ball.size() == 9);
}

TEST_CASE("Barcode has_hamming_neighbor detects neighbors") {
    Barcode bc("ACGTACGT");

    // Create a set with a Hamming neighbor (A->G mutation at position 0)
    std::unordered_set<std::string> existing = {"GCGTACGT"};
    CHECK(bc.has_hamming_neighbor(existing) == true);
}

TEST_CASE("Barcode has_hamming_neighbor detects self") {
    Barcode bc("ACGTACGT");
    std::unordered_set<std::string> existing = {"ACGTACGT"};
    CHECK(bc.has_hamming_neighbor(existing) == true);
}

TEST_CASE("Barcode has_hamming_neighbor rejects non-neighbors") {
    Barcode bc("ACGTACGT");

    // A sequence with multiple mutations should not be a neighbor
    std::unordered_set<std::string> existing = {"TTTTTTTT"};
    CHECK(bc.has_hamming_neighbor(existing) == false);
}

TEST_CASE("Barcode random generates valid barcode") {
    std::mt19937 gen(42);
    StemConfig config;
    config.closing_gc = 1;
    config.max_gc = 10;
    config.max_au = 10;

    std::unordered_set<std::string> existing;

    Barcode bc = Barcode::random(8, config, gen, existing);

    // Should have generated a barcode
    CHECK(!bc.empty());

    // Barcode length should be stem*2 + loop(4)
    CHECK(bc.length() == 20);
}

TEST_CASE("Barcode random avoids existing barcodes") {
    std::mt19937 gen(42);
    StemConfig config;
    config.closing_gc = 1;
    config.max_gc = 10;
    config.max_au = 10;

    std::unordered_set<std::string> existing;

    // Generate several barcodes
    for (int i = 0; i < 5; i++) {
        Barcode bc = Barcode::random(8, config, gen, existing);

        // Should not be a neighbor of any existing
        CHECK(!bc.has_hamming_neighbor(existing));

        existing.insert(bc.str());
    }

    // All barcodes should be unique
    CHECK(existing.size() == 5);
}
