#ifndef FASTA_IO_H
#define FASTA_IO_H

#include <string>
#include <vector>
#include <fstream>
#include <functional>

// Represents a single FASTA entry (header + sequence)
struct FastaEntry {
    std::string name;
    std::string sequence;
};

// Callback type for processing FASTA entries
using FastaCallback = std::function<void(const FastaEntry&)>;

// Read all entries from a FASTA file
std::vector<FastaEntry> read_fasta(const std::string& path);

// Process each entry in a FASTA file via callback (memory efficient)
void for_each_fasta(const std::string& path, FastaCallback callback);

// Process each entry with index
void for_each_fasta_indexed(
    const std::string& path,
    std::function<void(size_t index, const FastaEntry&)> callback
);

// Count entries in a FASTA file without loading all sequences
size_t count_fasta_entries(const std::string& path);

// FASTA writer helper
class FastaOutputStream {
public:
    explicit FastaOutputStream(const std::string& path);
    ~FastaOutputStream();

    void write(const std::string& name, const std::string& sequence);
    void write(const FastaEntry& entry);

private:
    std::ofstream _file;
};

#endif
