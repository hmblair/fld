#include "test.hpp"
#include <filesystem>
#include <iostream>
#include <cstdlib>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#elif __linux__
#include <unistd.h>
#include <limits.h>
#endif

static inline std::string _PARSER_NAME = "test";

TestArgs::TestArgs() : Program(_PARSER_NAME) {}

// Get the directory containing the current executable
static std::filesystem::path _get_executable_dir() {
#ifdef __APPLE__
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        return std::filesystem::canonical(path).parent_path();
    }
#elif __linux__
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        return std::filesystem::path(path).parent_path();
    }
#endif
    return std::filesystem::current_path();
}

int _run_tests(const char* argv0) {
    std::filesystem::path exe_dir = _get_executable_dir();
    std::filesystem::path tests_path;

    // Try locations in order of likelihood:
    // 1. Same directory as fld (bin/)
    // 2. Sibling build directory (../build/)
    // 3. build/ in current working directory
    // 4. Just "fld_tests" and hope it's in PATH

    std::vector<std::filesystem::path> candidates = {
        exe_dir / "fld_tests",
        exe_dir.parent_path() / "build" / "fld_tests",
        std::filesystem::current_path() / "build" / "fld_tests",
    };

    for (const auto& candidate : candidates) {
        if (std::filesystem::exists(candidate)) {
            tests_path = candidate;
            break;
        }
    }

    if (tests_path.empty()) {
        // Last resort: assume it's in PATH
        tests_path = "fld_tests";
    }

    std::string cmd = tests_path.string();

    std::cout << "Running tests: " << cmd << std::endl;
    std::cout << "─────────────────────────────────────────────────" << std::endl;

    int result = std::system(cmd.c_str());

    return WEXITSTATUS(result);
}
