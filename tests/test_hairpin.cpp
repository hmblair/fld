#include "doctest.hpp"
#include "domain/hairpin.hpp"
#include "domain/sequence.hpp"
#include <random>

TEST_CASE("Hairpin has correct structure") {
    std::mt19937 gen(42);
    StemConfig config;
    config.closing_gc = 1;
    config.max_gc = 10;
    config.max_au = 10;

    Hairpin hp = Hairpin::random(8, config, gen);

    // Hairpin should be: stem (8) + loop (4) + antistem (8) = 20
    CHECK(hp.str().length() == 20);
    CHECK(hp.stem_length() == 8);
}

TEST_CASE("Hairpin stem length varies") {
    std::mt19937 gen(42);
    StemConfig config;
    config.closing_gc = 1;
    config.max_gc = 10;
    config.max_au = 10;

    // Test different stem lengths
    for (size_t stem_len = 5; stem_len <= 12; stem_len++) {
        Hairpin hp = Hairpin::random(stem_len, config, gen);
        size_t expected = stem_len * 2 + 4;  // stem + loop + antistem
        CHECK(hp.str().length() == expected);
        CHECK(hp.stem_length() == stem_len);
    }
}

TEST_CASE("Hairpin contains only valid bases") {
    std::mt19937 gen(42);
    StemConfig config;
    config.closing_gc = 1;
    config.max_gc = 10;
    config.max_au = 10;

    Hairpin hp = Hairpin::random(10, config, gen);
    std::string seq = hp.str();

    for (char c : seq) {
        CHECK(Sequence::is_valid_base(c));
    }
}

TEST_CASE("Hairpin stem forms valid base pairs") {
    std::mt19937 gen(42);
    StemConfig config;
    config.closing_gc = 2;
    config.max_gc = 10;
    config.max_au = 10;
    config.max_gu = 2;

    Hairpin hp = Hairpin::random(8, config, gen);
    std::string seq = hp.str();

    // The stem pairs should be: seq[i] pairs with seq[length-1-i]
    // for i in [0, stem_length)
    size_t stem_len = hp.stem_length();
    size_t total_len = seq.length();

    for (size_t i = 0; i < stem_len; i++) {
        char five_base = seq[i];
        char three_base = seq[total_len - 1 - i];

        // Check if they form a valid base pair (AU, GC, or GU)
        bool is_au = BasePair::is_au(five_base, three_base);
        bool is_gc = BasePair::is_gc(five_base, three_base);
        bool is_gu = BasePair::is_gu(five_base, three_base);

        INFO("Position " << i << ": " << five_base << "-" << three_base);
        CHECK((is_au || is_gc || is_gu));
    }
}
