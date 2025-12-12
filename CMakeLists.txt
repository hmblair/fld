cmake_minimum_required(VERSION 3.29)
project(fld CXX)

# Get version from git tags (similar to setuptools_scm)
# Produces: "0.2.0" on tag, "0.2.0.dev3+gabcdef" after tag
execute_process(
    COMMAND git describe --tags --always
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE GIT_DESCRIBE_RESULT
)

if(GIT_DESCRIBE_RESULT EQUAL 0 AND GIT_DESCRIBE MATCHES "^v?([0-9]+)\\.([0-9]+)\\.([0-9]+)(-([0-9]+)-g([a-f0-9]+))?$")
    set(FLD_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(FLD_VERSION_MINOR ${CMAKE_MATCH_2})
    set(FLD_VERSION_PATCH ${CMAKE_MATCH_3})
    if(CMAKE_MATCH_5)
        # Dev version: commits after tag
        set(FLD_VERSION "${FLD_VERSION_MAJOR}.${FLD_VERSION_MINOR}.${FLD_VERSION_PATCH}.dev${CMAKE_MATCH_5}+g${CMAKE_MATCH_6}")
        set(FLD_VERSION_DEV ${CMAKE_MATCH_5})
        set(FLD_VERSION_COMMIT ${CMAKE_MATCH_6})
    else()
        # Release version: exactly on tag
        set(FLD_VERSION "${FLD_VERSION_MAJOR}.${FLD_VERSION_MINOR}.${FLD_VERSION_PATCH}")
        set(FLD_VERSION_DEV "")
        set(FLD_VERSION_COMMIT "")
    endif()
else()
    # Fallback when no tags exist
    set(FLD_VERSION "0.0.0+unknown")
    set(FLD_VERSION_MAJOR 0)
    set(FLD_VERSION_MINOR 0)
    set(FLD_VERSION_PATCH 0)
    set(FLD_VERSION_DEV "")
    set(FLD_VERSION_COMMIT "")
endif()

message(STATUS "FLD Version: ${FLD_VERSION}")

# Generate version header
configure_file(
    "${CMAKE_SOURCE_DIR}/src/version.hpp.in"
    "${CMAKE_BINARY_DIR}/generated/version.hpp"
    @ONLY
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INSTALL_MESSAGE LAZY)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

# For Homebrew LLVM on macOS: link against Homebrew's libc++ to match headers
if (APPLE AND EXISTS "/opt/homebrew/opt/llvm/lib")
    link_directories("/opt/homebrew/opt/llvm/lib" "/opt/homebrew/opt/llvm/lib/c++")
endif()

include(FetchContent)
function(FetchContent_MakeAvailable_If_Not_Already_Present name)
    if (NOT TARGET ${name}::${name})
        FetchContent_MakeAvailable(${name})
    endif()
endfunction()

FetchContent_Declare(
    indicators
    GIT_REPOSITORY https://github.com/p-ranav/indicators.git
)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)

FetchContent_MakeAvailable_If_Not_Already_Present(indicators)
FetchContent_MakeAvailable_If_Not_Already_Present(argparse)

# Collect all source files
file(GLOB_RECURSE FLD_ALL_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(${PROJECT_NAME} ${FLD_ALL_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    argparse
    indicators
)

target_compile_options(${PROJECT_NAME} PRIVATE -O3)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Testing
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()

    # Library sources (all src/*.cpp except main.cpp)
    list(FILTER FLD_ALL_SOURCES EXCLUDE REGEX ".*main\\.cpp$")
    set(FLD_LIB_SOURCES ${FLD_ALL_SOURCES})

    # Test sources
    file(GLOB_RECURSE FLD_TEST_SOURCES CONFIGURE_DEPENDS "tests/*.cpp")

    add_executable(fld_tests
        ${FLD_TEST_SOURCES}
        ${FLD_LIB_SOURCES}
    )

    target_include_directories(fld_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests
        ${CMAKE_BINARY_DIR}/generated
    )

    target_link_libraries(fld_tests PRIVATE
        argparse
        indicators
    )

    target_compile_options(fld_tests PRIVATE -O2)

    add_test(NAME fld_tests COMMAND fld_tests)
endif()
