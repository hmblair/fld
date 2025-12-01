#include "csv_format.hpp"

namespace csv {

static const std::vector<std::string> _columns = {
    "name",
    "sublibrary",
    "five_const",
    "five_padding",
    "design",
    "three_padding",
    "barcode",
    "three_const"
};

static std::string _build_header() {
    std::string header;
    for (size_t ix = 0; ix < _columns.size() - 1; ix++) {
        header += _columns[ix];
        header += ",";
    }
    header += _columns[_columns.size() - 1];
    return header;
}

static const std::string _header = _build_header();

const std::vector<std::string>& columns() {
    return _columns;
}

const std::string& header() {
    return _header;
}

bool is_valid_header(const std::string& line) {
    return line == _header;
}

}  // namespace csv
