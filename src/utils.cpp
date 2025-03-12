#include "utils.hpp"

std::mt19937 _init_gen() {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    return std::mt19937(seed);
}

std::string _csv_name(const std::string& prefix) {
    return prefix + ".csv";
}
std::string _fasta_name(const std::string& prefix) {
    return prefix + ".fasta";
}
std::string _txt_name(const std::string& prefix) {
    return prefix + ".txt";
}

void _throw_if_not_exists(const std::string& filename) {
    if (!std::filesystem::is_regular_file(filename)) {
        throw std::runtime_error("The file \"" + filename + "\" does not exist.");
    }
}

void _throw_if_exists(const std::string& filename) {
    if (std::filesystem::is_regular_file(filename)) {
        throw std::runtime_error("The file \"" + filename + "\" already exists.");
    }
}

void _remove_if_exists(const std::string& filename) {
    if (std::filesystem::is_regular_file(filename)) {
        std::filesystem::remove(filename);
    }
}

void _remove_if_exists(
    const std::string& filename,
    bool overwrite
) {
    if (!overwrite) {
        _throw_if_exists(filename);
    } else {
        _remove_if_exists(filename);
    }
}

void _remove_if_exists_all(
    const std::string& prefix,
    bool overwrite
) {
    _remove_if_exists(_csv_name(prefix), overwrite);
    _remove_if_exists(_fasta_name(prefix), overwrite);
    _remove_if_exists(_txt_name(prefix), overwrite);
}

std::vector<std::string> _split_by_delimiter(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token = "";
    bool insideQuotes = false;
    for (const auto& c : s) {
        if (c == delimiter && !insideQuotes) {
            tokens.push_back(token);
            token = "";
        } else if (c == '"') {
            insideQuotes = !insideQuotes;
        } else {
            token += c;
        }
    }
    tokens.push_back(token);
    return tokens;
}

static inline std::string _remove_char(
    const std::string &original,
    char to_remove
) {
    if (original.find(to_remove) != std::string::npos) {
        std::string tmp = original;
        tmp.erase(std::remove(tmp.begin(),tmp.end(), to_remove),tmp.end());
        return tmp;
    }
    return original;
}

std::string _escape_with_quotes(const std::string& original) {
    std::string tmp = _remove_char(original, '\"');
    return "\"" + _remove_char(tmp, '\"') + "\"";
}

bool _is_fasta_header(const std::string& line) {
    return line[0] == '>';
}

std::string _get_fasta_name(const std::string& line) {
    return line.substr(1);
}

std::string _get_fasta_seq(const std::string& line) {
    return line;
}
