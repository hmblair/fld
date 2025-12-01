#ifndef WRITERS_H
#define WRITERS_H

#include <string>
#include <fstream>
#include <vector>
#include <functional>

// Helper to generate output filenames from prefix
std::string output_csv(const std::string& prefix);
std::string output_fasta(const std::string& prefix);
std::string output_txt(const std::string& prefix);

// Writer base class for common file operations
class FileWriter {
public:
    explicit FileWriter(const std::string& filename);
    ~FileWriter();

    void write_line(const std::string& line);

protected:
    std::ofstream _file;
};

// CSV writer with header support
class CsvWriter : public FileWriter {
public:
    explicit CsvWriter(const std::string& filename);
};

// FASTA writer with sequence format
class FastaWriter : public FileWriter {
public:
    explicit FastaWriter(const std::string& filename);

    void write_sequence(const std::string& name, const std::string& sequence);
};

// TXT writer for plain sequences
class TxtWriter : public FileWriter {
public:
    explicit TxtWriter(const std::string& filename);
};

#endif
