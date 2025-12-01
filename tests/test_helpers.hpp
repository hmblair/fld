#pragma once
#include <random>
#include <string>
#include <fstream>
#include <filesystem>

// Temp directory that auto-cleans on destruction
class TempDir {
public:
    TempDir() {
        _path = std::filesystem::temp_directory_path() /
                ("fld_test_" + std::to_string(std::rand()));
        std::filesystem::create_directories(_path);
    }

    ~TempDir() {
        std::filesystem::remove_all(_path);
    }

    std::string path() const { return _path.string(); }

    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

private:
    std::filesystem::path _path;
};

// Random integer in range [low, high]
inline size_t random_range(size_t low, size_t high, std::mt19937& gen) {
    std::uniform_int_distribution<size_t> dist(low, high);
    return dist(gen);
}

// Generate random nucleotide sequence
inline std::string random_sequence(size_t length, std::mt19937& gen) {
    static const char bases[] = "ACGT";
    std::uniform_int_distribution<int> dist(0, 3);
    std::string seq(length, 'N');
    for (size_t i = 0; i < length; i++) {
        seq[i] = bases[dist(gen)];
    }
    return seq;
}

// Write random FASTA file with sequences of length up to max_length
inline void write_random_fasta(
    const std::string& path,
    size_t count,
    size_t max_length,
    std::mt19937& gen
) {
    std::ofstream file(path);
    for (size_t i = 0; i < count; i++) {
        size_t len = random_range(10, max_length, gen);
        file << ">seq" << i << "\n";
        file << random_sequence(len, gen) << "\n";
    }
}

// Write a specific FASTA file with given sequences
inline void write_fasta(
    const std::string& path,
    const std::vector<std::pair<std::string, std::string>>& sequences
) {
    std::ofstream file(path);
    for (const auto& [name, seq] : sequences) {
        file << ">" << name << "\n";
        file << seq << "\n";
    }
}
