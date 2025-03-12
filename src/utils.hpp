#include <string>
#include <filesystem>
#include <chrono>
#include <random>

std::mt19937 _init_gen();

std::string _csv_name(const std::string& prefix);
std::string _fasta_name(const std::string& prefix);
std::string _txt_name(const std::string& prefix);

void _throw_if_not_exists(const std::string& filename);
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

std::string _escape_with_quotes(const std::string& original);

bool _is_fasta_header(const std::string& line);
std::string _get_fasta_name(const std::string& line);
std::string _get_fasta_seq(const std::string& line);
