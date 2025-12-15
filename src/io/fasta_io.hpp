#ifndef FASTA_IO_H
#define FASTA_IO_H

#include <string>
#include <vector>
#include <fstream>
#include <functional>
#include "../utils.hpp"

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

/**
 * @brief Transform each sequence in a FASTA file and write to output.
 *
 * Common pattern for commands that read a FASTA, transform each sequence,
 * and write the result to a new FASTA file.
 *
 * @tparam TransformFunc Callable that takes (const FastaEntry&) and returns std::string
 * @param input Input FASTA file path
 * @param output Output FASTA file path
 * @param overwrite Whether to overwrite existing output file
 * @param transform Function to transform each entry's sequence
 * @return Number of sequences processed
 */
template <typename TransformFunc>
size_t transform_fasta(
    const std::string& input,
    const std::string& output,
    bool overwrite,
    TransformFunc transform
) {
    _throw_if_not_exists(input);
    _remove_if_exists(output, overwrite);

    FastaOutputStream out(output);
    size_t count = 0;

    for_each_fasta(input, [&](const FastaEntry& entry) {
        out.write(entry.name, transform(entry));
        count++;
    });

    return count;
}

#endif
