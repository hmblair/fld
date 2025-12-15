#include "csv_format.hpp"
#include <sstream>
#include <algorithm>

namespace csv {

// Required columns (sequence data only)
static const std::vector<std::string> _required_columns = {
    COL_FIVE_CONST,
    COL_FIVE_PADDING,
    COL_DESIGN,
    COL_THREE_PADDING,
    COL_BARCODE,
    COL_THREE_CONST
};

// All standard columns (for output)
static const std::vector<std::string> _columns = {
    COL_INDEX,
    COL_NAME,
    COL_SUBLIBRARY,
    COL_FIVE_CONST,
    COL_FIVE_PADDING,
    COL_DESIGN,
    COL_THREE_PADDING,
    COL_BARCODE,
    COL_THREE_CONST,
    COL_BEGIN,
    COL_END
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

const std::vector<std::string>& required_columns() {
    return _required_columns;
}

const std::vector<std::string>& columns() {
    return _columns;
}

const std::string& header() {
    return _header;
}

// Helper to split string by delimiter
static std::vector<std::string> _split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

// Helper to trim whitespace
static std::string _trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

Header::Header(const std::string& line) : _header(line) {
    _columns = _split(line, ',');
    for (size_t i = 0; i < _columns.size(); i++) {
        std::string col = _trim(_columns[i]);
        _columns[i] = col;
        _col_indices[col] = static_cast<int>(i);
    }
}

bool Header::has(const std::string& col) const {
    return _col_indices.find(col) != _col_indices.end();
}

int Header::index_of(const std::string& col) const {
    auto it = _col_indices.find(col);
    if (it != _col_indices.end()) {
        return it->second;
    }
    return -1;
}

std::string Header::get(const std::vector<std::string>& fields, const std::string& col,
                        const std::string& default_value) const {
    int idx = index_of(col);
    if (idx < 0 || static_cast<size_t>(idx) >= fields.size()) {
        return default_value;
    }
    return fields[idx];
}

void Header::validate() const {
    std::vector<std::string> missing;
    for (const auto& col : _required_columns) {
        if (!has(col)) {
            missing.push_back(col);
        }
    }
    if (!missing.empty()) {
        std::string msg = "Missing required CSV columns: ";
        for (size_t i = 0; i < missing.size(); i++) {
            if (i > 0) msg += ", ";
            msg += missing[i];
        }
        throw std::runtime_error(msg);
    }
}

bool is_valid_header(const std::string& line) {
    try {
        Header h(line);
        h.validate();
        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace csv
