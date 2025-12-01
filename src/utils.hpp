#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <filesystem>
#include <chrono>
#include <random>
#include <argparse/argparse.hpp>
#include <variant>

std::mt19937 _init_gen();

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

double _percent(size_t val, size_t total);

typedef argparse::ArgumentParser Parser;
template <typename T>
class Arg {
private:
    Parser& _parser;
    std::string _name;
public:
    Arg(Parser& parser, const std::string& name, const std::string& help);
    Arg(Parser& parser, const std::string& name, const std::string& help, T default_value);
    T value() const;
    operator T() const;
};

class Program {
private:
    std::string _name;
public:
    Parser _parser;
    Program(std::string name);
    void parse(int argc, char** argv);
    bool used(const Parser& parent) const;
};

#endif
