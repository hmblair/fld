#include "inspect.hpp"
#include "io/fasta_io.hpp"


static inline std::string _PARSER_NAME = "inspect";
static inline std::string _FILES_NAME = "files";
static inline std::string _FILES_HELP = "The input .fasta file or files.";
static inline std::string _SORT_NAME = "--sort";
static inline std::string _SORT_HELP = "Sort the output sequences by count rather than length.";

InspectArgs::InspectArgs() :
    Program(_PARSER_NAME),
    files(_parser, _FILES_NAME, _FILES_HELP),
    sort(_parser, _SORT_NAME, _SORT_HELP) {

}

static inline std::unordered_map<size_t, size_t> _get_length_counts(const std::string& filename) {
    std::unordered_map<size_t, size_t> counts;
    _throw_if_not_exists(filename);
    for_each_fasta(filename, [&counts](const FastaEntry& entry) {
        counts[entry.sequence.length()]++;
    });
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

static inline size_t find_longest(const std::vector<std::pair<size_t, size_t>>& vec) {
    if (vec.empty()) return 0;
    auto max_pair = *std::max_element(vec.begin(), vec.end(),
        [](const std::pair<size_t, size_t>& a, const std::pair<size_t, size_t>& b) {
            return a.first < b.first;
        });
    return max_pair.first;
}

static inline size_t find_most(const std::vector<std::pair<size_t, size_t>>& vec) {
    if (vec.empty()) return 0;
    auto max_pair = *std::max_element(vec.begin(), vec.end(),
        [](const std::pair<size_t, size_t>& a, const std::pair<size_t, size_t>& b) {
            return a.second < b.second;
        });
    return max_pair.second;
}

static inline std::string _pluralize(const std::string& str, size_t count) {
    if (count == 1) {
        return str;
    }
    return str + "s";
}

static inline void _merge_counts(
    std::unordered_map<size_t, size_t>& total,
    const std::unordered_map<size_t, size_t>& other
) {
    for (const auto& [length, count] : other) {
        total[length] += count;
    }
}

static inline void _print_length_counts(
    const std::unordered_map<size_t, size_t>& counts,
    bool sort_by_count,
    const std::string& label = "",
    bool bold = false
) {
    std::vector<std::pair<size_t, size_t>> count_pairs(counts.begin(), counts.end());

    if (sort_by_count) {
        std::sort(count_pairs.begin(), count_pairs.end(), _compare_by_count);
    } else {
        std::sort(count_pairs.begin(), count_pairs.end(), _compare_by_length);
    }

    size_t total = 0;
    for (const auto& pair : count_pairs) {
        total += pair.second;
    }

    size_t longest = find_longest(count_pairs);
    size_t longest_digits = std::to_string(longest).length();
    size_t most = find_most(count_pairs);
    size_t most_digits = std::to_string(most).length();

    std::cout << "\n";
    if (!label.empty()) {
        if (bold) std::cout << "\033[1m";
        std::cout << "  " << label << "\n";
        std::cout << "  " << std::string(label.length(), '-');
        if (bold) std::cout << "\033[0m";
        std::cout << "\n";
    }

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
}

void _inspect(const std::vector<std::string>& files, bool sort) {
    if (files.size() == 1) {
        // Single file - just show its stats
        auto counts = _get_length_counts(files[0]);
        _print_length_counts(counts, sort);
    } else {
        // Multiple files - show per-file stats and total
        std::unordered_map<size_t, size_t> total_counts;

        for (const auto& file : files) {
            auto counts = _get_length_counts(file);
            _print_length_counts(counts, sort, file);
            _merge_counts(total_counts, counts);
        }

        // Print combined total
        _print_length_counts(total_counts, sort, "Total (all files)", true);
    }
}
