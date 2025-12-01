#include "doctest.hpp"
#include "domain/sequence.hpp"

TEST_CASE("Sequence to_dna converts U to T") {
    Sequence s("ACGU");
    CHECK(s.to_dna().str() == "ACGT");
}

TEST_CASE("Sequence to_rna converts T to U") {
    Sequence s("ACGT");
    CHECK(s.to_rna().str() == "ACGU");
}

TEST_CASE("Sequence complement") {
    CHECK(Sequence::complement('A') == 'T');
    CHECK(Sequence::complement('T') == 'A');
    CHECK(Sequence::complement('G') == 'C');
    CHECK(Sequence::complement('C') == 'G');
    CHECK(Sequence::complement('U') == 'A');
}

TEST_CASE("Sequence length and empty") {
    Sequence empty("");
    CHECK(empty.length() == 0);
    CHECK(empty.empty() == true);

    Sequence seq("ACGT");
    CHECK(seq.length() == 4);
    CHECK(seq.empty() == false);
}

TEST_CASE("Sequence concatenation") {
    Sequence a("ACGT");
    Sequence b("TGCA");
    Sequence c = a + b;
    CHECK(c.str() == "ACGTTGCA");
}

TEST_CASE("Sequence is_valid_base") {
    CHECK(Sequence::is_valid_base('A') == true);
    CHECK(Sequence::is_valid_base('C') == true);
    CHECK(Sequence::is_valid_base('G') == true);
    CHECK(Sequence::is_valid_base('T') == true);
    CHECK(Sequence::is_valid_base('U') == true);
    CHECK(Sequence::is_valid_base('N') == false);
    CHECK(Sequence::is_valid_base('X') == false);
}
