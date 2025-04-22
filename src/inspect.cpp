#include "inspect.hpp"


static inline std::string _PARSER_NAME = "inspect";
// File
static inline std::string _FILE_NAME = "file";
static inline std::string _FILE_HELP = "The input .fasta file.";
// Sort
static inline std::string _SORT_NAME = "--sort";
static inline std::string _SORT_HELP = "Sort the output sequences by count rather than length.";

InspectArgs::InspectArgs() :
    Program(_PARSER_NAME),
    file(_parser, _FILE_NAME, _FILE_HELP),
    sort(_parser, _SORT_NAME, _SORT_HELP) {

}

static inline std::unordered_map<size_t, size_t> _get_length_counts(const std::string& filename) {
    _throw_if_not_exists(filename);
    std::ifstream file(filename);
    std::string line;

    std::unordered_map<size_t, size_t> counts;
    size_t curr_length = 0;

    while (std::getline(file, line)) {
        if (_is_fasta_header(line)) {
            if (curr_length > 0) {
                counts[curr_length]++;
                curr_length = 0;
            }
        } else {
            curr_length += line.length();
        }
    }
    if (curr_length > 0) {
        counts[curr_length]++;
    }

    return counts;
}

static inline bool _compare_by_length(
    const std::pair<size_t, size_t>& a,
    const std::pair<size_t, size_t>& b
) {
    return a.first < b.first;
}

static inline bool _compare_by_count(
    const std::pair<size_t, size_t>& a,
    const std::pair<size_t, size_t>& b
) {
    return a.second < b.second;
}

size_t find_longest(const std::vector<std::pair<size_t, size_t>>& vec) {
    auto max_pair = *std::max_element(vec.begin(), vec.end(),
        [](const std::pair<size_t, size_t>& a, const std::pair<size_t, size_t>& b) {
            return a.first < b.first;
        });
    return max_pair.first;
}

size_t find_most(const std::vector<std::pair<size_t, size_t>>& vec) {
    auto max_pair = *std::max_element(vec.begin(), vec.end(),
        [](const std::pair<size_t, size_t>& a, const std::pair<size_t, size_t>& b) {
            return a.second < b.second;
        });
    return max_pair.second;
}

// Helper function to pluralize the word "sequence"
static inline std::string _pluralize(const std::string& str, size_t count) {
    if (count == 1) {
        return str;
    }
    return str + "s";
}

static inline void _print_length_counts(const std::unordered_map<size_t, size_t>& counts, bool sort) {
    std::vector<std::pair<size_t, size_t>> count_pairs(counts.begin(), counts.end());

    if (sort) {
        std::sort(
            count_pairs.begin(),
            count_pairs.end(),
            _compare_by_count
        );
    } else {
        std::sort(
            count_pairs.begin(),
            count_pairs.end(),
            _compare_by_length
        );
    }

    // Calculate the total number of sequences
    size_t total = 0;
    for (const auto& pair : count_pairs) {
        total += pair.second;
    }
    // Get the longest sequence
    size_t longest = find_longest(count_pairs);
    size_t longest_digits = std::to_string(longest).length();
    // Get the highest sequence count
    size_t most = find_most(count_pairs);
    size_t most_digits = std::to_string(most).length();

    // Print the counts and percentages of sequences of each length
    std::cout << "\n";
    for (const auto& pair : count_pairs) {
        size_t length = pair.first;
        size_t count = pair.second;
        double percentage = _percent(count, total);
        std::cout << "  Length " << std::left << std::setw(longest_digits + 2) << std::to_string(length) + ": ";
        std::cout << std::left << std::setw(most_digits + 10) << std::to_string(count) + _pluralize(" sequence", count);
        std::cout << " (" << std::fixed << std::setprecision(2) << percentage << "%)\n";
    }
    std::cout << "\n";
    std::cout << "  Total: " << total << _pluralize(" sequence", total) << "\n";
    std::cout << "\n";
}

void _inspect(const std::string& filename, bool sort) {
    std::unordered_map<size_t, size_t> counts = _get_length_counts(filename);
    _print_length_counts(counts, sort);
}
