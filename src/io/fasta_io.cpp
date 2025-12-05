#include "fasta_io.hpp"
#include <stdexcept>

static inline bool is_header(const std::string& line) {
    return !line.empty() && line[0] == '>';
}

static inline std::string extract_name(const std::string& header_line) {
    return header_line.substr(1);
}

std::vector<FastaEntry> read_fasta(const std::string& path) {
    std::vector<FastaEntry> entries;
    for_each_fasta(path, [&entries](const FastaEntry& entry) {
        entries.push_back(entry);
    });
    return entries;
}

void for_each_fasta(const std::string& path, FastaCallback callback) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open FASTA file: " + path);
    }

    std::string line;
    FastaEntry current;
    bool has_entry = false;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        if (is_header(line)) {
            // Process previous entry if exists
            if (has_entry) {
                callback(current);
                current.sequence.clear();
            }
            current.name = extract_name(line);
            has_entry = true;
        } else {
            current.sequence += line;
        }
    }

    // Process final entry
    if (has_entry && !current.sequence.empty()) {
        callback(current);
    }
}

void for_each_fasta_indexed(
    const std::string& path,
    std::function<void(size_t index, const FastaEntry&)> callback
) {
    size_t index = 0;
    for_each_fasta(path, [&index, &callback](const FastaEntry& entry) {
        callback(index++, entry);
    });
}

size_t count_fasta_entries(const std::string& path) {
    size_t count = 0;
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open FASTA file: " + path);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (is_header(line)) {
            count++;
        }
    }
    return count;
}

FastaOutputStream::FastaOutputStream(const std::string& path) {
    _file.open(path);
    if (!_file) {
        throw std::runtime_error("Cannot create FASTA file: " + path);
    }
}

FastaOutputStream::~FastaOutputStream() {
    if (_file.is_open()) {
        _file.close();
    }
}

void FastaOutputStream::write(const std::string& name, const std::string& sequence) {
    _file << ">" << name << "\n" << sequence << "\n";
}

void FastaOutputStream::write(const FastaEntry& entry) {
    write(entry.name, entry.sequence);
}
