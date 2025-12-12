#include "doctest.hpp"
#include "io/csv_format.hpp"

TEST_CASE("csv::header returns correct format") {
    std::string header = csv::header();
    CHECK(header == "index,name,sublibrary,five_const,five_padding,design,three_padding,barcode,three_const");
}

TEST_CASE("csv::is_valid_header accepts correct header") {
    CHECK(csv::is_valid_header("index,name,sublibrary,five_const,five_padding,design,three_padding,barcode,three_const"));
}

TEST_CASE("csv::is_valid_header rejects incorrect header") {
    CHECK_FALSE(csv::is_valid_header("wrong,header"));
    CHECK_FALSE(csv::is_valid_header(""));
    CHECK_FALSE(csv::is_valid_header("index,name,sublibrary"));
    CHECK_FALSE(csv::is_valid_header("INDEX,NAME,SUBLIBRARY,FIVE_CONST,FIVE_PADDING,DESIGN,THREE_PADDING,BARCODE,THREE_CONST"));
}

TEST_CASE("csv::columns returns correct count") {
    const auto& cols = csv::columns();
    CHECK(cols.size() == 9);
}

TEST_CASE("csv::columns contains expected names") {
    const auto& cols = csv::columns();
    CHECK(cols[0] == "index");
    CHECK(cols[1] == "name");
    CHECK(cols[2] == "sublibrary");
    CHECK(cols[3] == "five_const");
    CHECK(cols[4] == "five_padding");
    CHECK(cols[5] == "design");
    CHECK(cols[6] == "three_padding");
    CHECK(cols[7] == "barcode");
    CHECK(cols[8] == "three_const");
}
