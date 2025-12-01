#include "writers.hpp"
#include "csv_format.hpp"
#include "../utils.hpp"

std::string output_csv(const std::string& prefix) {
    return prefix + ".csv";
}

std::string output_fasta(const std::string& prefix) {
    return prefix + ".fasta";
}

std::string output_txt(const std::string& prefix) {
    return prefix + ".txt";
}

FileWriter::FileWriter(const std::string& filename) {
    _throw_if_exists(filename);
    _file.open(filename);
}

FileWriter::~FileWriter() {
    if (_file.is_open()) {
        _file.close();
    }
}

void FileWriter::write_line(const std::string& line) {
    _file << line << "\n";
}

CsvWriter::CsvWriter(const std::string& filename) : FileWriter(filename) {
    _file << csv::header() << "\n";
}

FastaWriter::FastaWriter(const std::string& filename) : FileWriter(filename) {}

void FastaWriter::write_sequence(const std::string& name, const std::string& sequence) {
    _file << ">" << name << "\n";
    _file << sequence << "\n";
}

TxtWriter::TxtWriter(const std::string& filename) : FileWriter(filename) {}
