#include "utils.hpp"
#include "io/writers.hpp"

std::mt19937 _init_gen() {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    return std::mt19937(seed);
}

void _throw_if_not_exists(const std::string& filename) {
    if (!std::filesystem::is_regular_file(filename)) {
        throw std::runtime_error("The file \"" + filename + "\" does not exist.");
    }
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
    _remove_if_exists(output_csv(prefix), overwrite);
    _remove_if_exists(output_fasta(prefix), overwrite);
    _remove_if_exists(output_txt(prefix), overwrite);
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

static inline std::string _remove_char(
    const std::string &original,
    char to_remove
) {
    if (original.find(to_remove) != std::string::npos) {
        std::string tmp = original;
        tmp.erase(std::remove(tmp.begin(),tmp.end(), to_remove),tmp.end());
        return tmp;
    }
    return original;
}

std::string _escape_with_quotes(const std::string& original) {
    return "\"" + _remove_char(original, '\"') + "\"";
}

std::string _quote_csv_field(const std::string& field) {
    bool needs_quoting = field.find(',') != std::string::npos ||
                         field.find('"') != std::string::npos ||
                         field.find('\n') != std::string::npos;
    if (!needs_quoting) {
        return field;
    }
    // Escape internal quotes by doubling them
    std::string escaped;
    escaped.reserve(field.size() + 2);
    escaped += '"';
    for (char c : field) {
        if (c == '"') {
            escaped += "\"\"";
        } else {
            escaped += c;
        }
    }
    escaped += '"';
    return escaped;
}

bool _is_fasta_header(const std::string& line) {
    return !line.empty() && line[0] == '>';
}

std::string _get_fasta_name(const std::string& line) {
    return line.substr(1);
}

std::string _get_fasta_seq(const std::string& line) {
    return line;
}

double _frac(size_t val, size_t total) {
    double _val = static_cast<double>(val);
    double _total = static_cast<double>(total);
    return _val / _total ;
}

double _percent(size_t val, size_t total) {
    return _frac(val, total) * 100;
}

std::vector<double> _load_reads(const std::string& filename, size_t expected_count) {
    std::vector<double> reads;
    std::ifstream in(filename);
    if (!in) {
        throw std::runtime_error("Cannot open reads file: " + filename);
    }

    std::string line;
    size_t line_num = 0;
    while (std::getline(in, line)) {
        line_num++;
        if (line.empty()) continue;
        try {
            reads.push_back(std::stod(line));
        } catch (const std::exception&) {
            throw std::runtime_error(
                "Invalid number in " + filename + " at line " + std::to_string(line_num) +
                ": \"" + line + "\""
            );
        }
    }

    // Pad with zeros if reads file is shorter
    while (reads.size() < expected_count) {
        reads.push_back(0.0);
    }

    return reads;
}

std::vector<std::string> _load_lines(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream in(filename);
    if (!in) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }

    return lines;
}

static inline std::string _remove_from_start(
    const std::string& str,
    char ch
) {
    size_t index = 0;
    while (index < str.length() && str[index] == ch) {
        index++;
    }
    return str.substr(index);
}

template <typename T>
void _init_parser(
    Parser& parser,
    const std::string& name,
    const std::string& help
);

template <typename T>
void _init_parser(
    Parser& parser,
    const std::string& name,
    const std::string& help,
    T default_value
);

template<>
void _init_parser<bool>(
    Parser& parser,
    const std::string& name,
    const std::string& help
) {
    parser.add_argument(name)
        .default_value(false)
        .implicit_value(true)
        .help(help);
}

template<>
void _init_parser<bool>(
    Parser& parser,
    const std::string& name,
    const std::string& help,
    bool default_value
) {
    parser.add_argument(name)
        .default_value(default_value)
        .implicit_value(!default_value)
        .help(help);
}

template<>
void _init_parser<int>(
    Parser& parser,
    const std::string& name,
    const std::string& help
) {
    parser.add_argument(name)
        .required()
        .scan<'d', int>()
        .help(help);
}

template<>
void _init_parser<int>(
    Parser& parser,
    const std::string& name,
    const std::string& help,
    int default_value
) {
    parser.add_argument(name)
        .scan<'d', int>()
        .default_value(default_value)
        .help(help);
}

template<>
void _init_parser<std::string>(
    Parser& parser,
    const std::string& name,
    const std::string& help
) {
    parser.add_argument(name)
        .required()
        .help(help);
}

template<>
void _init_parser<std::string>(
    Parser& parser,
    const std::string& name,
    const std::string& help,
    std::string default_value
) {
    parser.add_argument(name)
        .default_value(default_value)
        .help(help);
}

template<>
void _init_parser<std::vector<std::string>>(
    Parser& parser,
    const std::string& name,
    const std::string& help
) {
    parser.add_argument(name)
        .remaining()
        .required()
        .help(help);
}

template<>
void _init_parser<std::vector<std::string>>(
    Parser& parser,
    const std::string& name,
    const std::string& help,
    std::vector<std::string> default_value
) {
    parser.add_argument(name)
        .remaining()
        .default_value(default_value)
        .help(help);
}

template<>
void _init_parser<std::vector<int>>(
    Parser& parser,
    const std::string& name,
    const std::string& help
) {
    parser.add_argument(name)
        .nargs(argparse::nargs_pattern::at_least_one)
        .scan<'d', int>()
        .required()
        .help(help);
}

template<>
void _init_parser<std::vector<int>>(
    Parser& parser,
    const std::string& name,
    const std::string& help,
    std::vector<int> default_value
) {
    parser.add_argument(name)
        .nargs(argparse::nargs_pattern::at_least_one)
        .scan<'d', int>()
        .default_value(default_value)
        .help(help);
}

template <typename T>
Arg<T>::Arg (
    Parser& parser,
    const std::string& name,
    const std::string& help
) : _parser(parser), _name(name) {
    _init_parser<T>(parser, name, help);
}

template <typename T>
Arg<T>::Arg (
    Parser& parser,
    const std::string& name,
    const std::string& help,
    T default_value
) : _parser(parser), _name(name) {
    _init_parser<T>(parser, name, help, default_value);
}

template <typename T>
T Arg<T>::value() const {
    return _parser.template get<T>(_name);
}

template <typename T>
Arg<T>::operator T() const {
    return value();
}

template class Arg<int>;
template class Arg<bool>;
template class Arg<std::string>;
template class Arg<std::vector<std::string>>;
template class Arg<std::vector<int>>;

Program::Program(std::string name) : _name(name), _parser(name) {};

void Program::parse(int argc, char** argv) {
    _parser.parse_args(argc, argv);
}

bool Program::used(const Parser& parent) const {
    return parent.is_subcommand_used(_name);
}
