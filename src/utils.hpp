#include <string>
#include <filesystem>
#include <chrono>
#include <random>

std::mt19937 _init_gen();

std::string _csv_name(const std::string& prefix);
std::string _fasta_name(const std::string& prefix);
std::string _txt_name(const std::string& prefix);

void _throw_if_exists(const std::string& filename);
void _remove_if_exists(const std::string& filename);
void _remove_if_exists(
    const std::string& filename,
    bool overwrite
);
void _remove_if_exists_all(
    const std::string& prefix,
    bool overwrite
);

std::vector<std::string> _split_by_delimiter(
    const std::string& s,
    char delimiter
);

