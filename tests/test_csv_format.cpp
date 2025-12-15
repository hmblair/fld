#include "doctest.hpp"
#include "io/csv_format.hpp"

TEST_CASE("csv::header returns correct format") {
    std::string header = csv::header();
    CHECK(header == "index,name,sublibrary,five_const,five_padding,design,three_padding,barcode,three_const,begin,end");
}

TEST_CASE("csv::required_columns contains sequence columns") {
    const auto& cols = csv::required_columns();
    CHECK(cols.size() == 6);
    CHECK(std::find(cols.begin(), cols.end(), csv::COL_FIVE_CONST) != cols.end());
    CHECK(std::find(cols.begin(), cols.end(), csv::COL_FIVE_PADDING) != cols.end());
    CHECK(std::find(cols.begin(), cols.end(), csv::COL_DESIGN) != cols.end());
    CHECK(std::find(cols.begin(), cols.end(), csv::COL_THREE_PADDING) != cols.end());
    CHECK(std::find(cols.begin(), cols.end(), csv::COL_BARCODE) != cols.end());
    CHECK(std::find(cols.begin(), cols.end(), csv::COL_THREE_CONST) != cols.end());
}

TEST_CASE("csv::is_valid_header accepts full header") {
    CHECK(csv::is_valid_header("index,name,sublibrary,five_const,five_padding,design,three_padding,barcode,three_const,begin,end"));
    // Also accept headers with extra columns
    CHECK(csv::is_valid_header("index,name,sublibrary,five_const,five_padding,design,three_padding,barcode,three_const,begin,end,reads"));
}

TEST_CASE("csv::is_valid_header accepts minimal header") {
    // Only required columns
    CHECK(csv::is_valid_header("five_const,five_padding,design,three_padding,barcode,three_const"));
    // Different order
    CHECK(csv::is_valid_header("design,five_const,five_padding,three_padding,barcode,three_const"));
}

TEST_CASE("csv::is_valid_header rejects missing required columns") {
    CHECK_FALSE(csv::is_valid_header("wrong,header"));
    CHECK_FALSE(csv::is_valid_header(""));
    CHECK_FALSE(csv::is_valid_header("index,name,sublibrary"));  // No sequence columns
    CHECK_FALSE(csv::is_valid_header("five_const,five_padding,design"));  // Missing 3' columns
}

TEST_CASE("csv::columns returns correct count") {
    const auto& cols = csv::columns();
    CHECK(cols.size() == 11);
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
    CHECK(cols[9] == "begin");
    CHECK(cols[10] == "end");
}

TEST_CASE("csv::Header parses columns correctly") {
    csv::Header h("five_const,five_padding,design,three_padding,barcode,three_const");
    CHECK(h.size() == 6);
    CHECK(h.has(csv::COL_FIVE_CONST));
    CHECK(h.has(csv::COL_DESIGN));
    CHECK_FALSE(h.has(csv::COL_INDEX));
    CHECK_FALSE(h.has(csv::COL_NAME));
}

TEST_CASE("csv::Header index_of returns correct indices") {
    csv::Header h("design,five_const,barcode,three_const,five_padding,three_padding");
    CHECK(h.index_of(csv::COL_DESIGN) == 0);
    CHECK(h.index_of(csv::COL_FIVE_CONST) == 1);
    CHECK(h.index_of(csv::COL_BARCODE) == 2);
    CHECK(h.index_of(csv::COL_INDEX) == -1);
}

TEST_CASE("csv::Header get retrieves correct values") {
    csv::Header h("design,five_const,name");
    std::vector<std::string> fields = {"ACGT", "TGCA", "seq1"};

    CHECK(h.get(fields, csv::COL_DESIGN) == "ACGT");
    CHECK(h.get(fields, csv::COL_FIVE_CONST) == "TGCA");
    CHECK(h.get(fields, csv::COL_NAME) == "seq1");
    CHECK(h.get(fields, csv::COL_INDEX, "default") == "default");
}

TEST_CASE("csv::Header validate throws for missing columns") {
    csv::Header h1("five_const,five_padding,design");
    CHECK_THROWS_WITH(h1.validate(), doctest::Contains("Missing required CSV columns"));

    csv::Header h2("five_const,five_padding,design,three_padding,barcode,three_const");
    CHECK_NOTHROW(h2.validate());
}

TEST_CASE("csv::Header handles whitespace in column names") {
    csv::Header h("design , five_const, barcode ");
    CHECK(h.has("design"));
    CHECK(h.has("five_const"));
    CHECK(h.has("barcode"));
}

TEST_CASE("csv::Header get handles out-of-bounds gracefully") {
    csv::Header h("design,five_const");
    std::vector<std::string> fields = {"ACGT"};  // Only 1 field, but 2 columns

    CHECK(h.get(fields, csv::COL_DESIGN) == "ACGT");
    CHECK(h.get(fields, csv::COL_FIVE_CONST) == "");  // Out of bounds returns empty
    CHECK(h.get(fields, csv::COL_FIVE_CONST, "default") == "default");  // Or provided default
}
