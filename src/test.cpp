#include "test.hpp"
#include <filesystem>
#include <iostream>
#include <cstdlib>

static inline std::string _PARSER_NAME = "test";

TestArgs::TestArgs() : Program(_PARSER_NAME) {}

int _run_tests(const char* argv0) {
    // Find the directory containing the fld executable
    std::filesystem::path exe_path(argv0);
    std::filesystem::path exe_dir;

    if (exe_path.is_absolute()) {
        exe_dir = exe_path.parent_path();
    } else {
        // Try to resolve relative path
        exe_dir = std::filesystem::current_path() / exe_path.parent_path();
    }

    // Look for fld_tests in the same directory
    std::filesystem::path tests_path = exe_dir / "fld_tests";

    if (!std::filesystem::exists(tests_path)) {
        // Also check in build directory relative to current working directory
        tests_path = std::filesystem::current_path() / "build" / "fld_tests";
    }

    if (!std::filesystem::exists(tests_path)) {
        // Check if fld_tests is in PATH
        tests_path = "fld_tests";
    }

    std::string cmd = tests_path.string();

    std::cout << "Running tests: " << cmd << std::endl;
    std::cout << "─────────────────────────────────────────────────" << std::endl;

    int result = std::system(cmd.c_str());

    return WEXITSTATUS(result);
}
