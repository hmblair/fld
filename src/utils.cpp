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
