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

TEST_CASE("complement in the DNA alphabet") {
    CHECK(complement('A', Alphabet::DNA) == 'T');
    CHECK(complement('T', Alphabet::DNA) == 'A');
    CHECK(complement('G', Alphabet::DNA) == 'C');
    CHECK(complement('C', Alphabet::DNA) == 'G');
    CHECK(complement('U', Alphabet::DNA) == 'A');
}

TEST_CASE("complement in the RNA alphabet") {
    CHECK(complement('A', Alphabet::RNA) == 'U');
    CHECK(complement('U', Alphabet::RNA) == 'A');
    CHECK(complement('G', Alphabet::RNA) == 'C');
    CHECK(complement('C', Alphabet::RNA) == 'G');
    CHECK(complement('T', Alphabet::RNA) == 'A');
}

TEST_CASE("complement throws on an invalid base") {
    CHECK_THROWS(complement('N', Alphabet::DNA));
    CHECK_THROWS(complement('X', Alphabet::RNA));
}

TEST_CASE("detect_alphabet") {
    CHECK(detect_alphabet("ACGU") == Alphabet::RNA);
    CHECK(detect_alphabet("ACGT") == Alphabet::DNA);
    CHECK(detect_alphabet("ACG") == Alphabet::DNA);
    CHECK(detect_alphabet("acgu") == Alphabet::RNA);
}

TEST_CASE("alphabet_bases") {
    CHECK(alphabet_bases(Alphabet::DNA) == std::vector<char>{'A', 'C', 'G', 'T'});
    CHECK(alphabet_bases(Alphabet::RNA) == std::vector<char>{'A', 'C', 'G', 'U'});
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
