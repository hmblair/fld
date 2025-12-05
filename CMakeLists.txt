cmake_minimum_required(VERSION 3.29)
project(fld CXX)

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

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/library.cpp
    src/nuc.cpp
    src/utils.cpp
    src/preprocess.cpp
    src/inspect.cpp
    src/barcodes.cpp
    src/m2.cpp
    src/random.cpp
    src/duplicate.cpp
    src/totxt.cpp
    src/test.cpp
    src/categorize.cpp
    src/sort.cpp
    src/merge.cpp
    src/pipeline.cpp
    src/prepend.cpp
    src/torna.cpp
    src/config/stem_config.cpp
    src/config/barcode_config.cpp
    src/config/design_config.cpp
    src/io/csv_format.cpp
    src/io/progress.cpp
    src/io/writers.cpp
    src/domain/sequence.cpp
    src/domain/hairpin.cpp
    src/domain/barcode.cpp
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

    # Collect source files (excluding main.cpp)
    set(FLD_LIB_SOURCES
        src/library.cpp
        src/nuc.cpp
        src/utils.cpp
        src/preprocess.cpp
        src/inspect.cpp
        src/barcodes.cpp
        src/m2.cpp
        src/random.cpp
        src/duplicate.cpp
        src/totxt.cpp
        src/categorize.cpp
        src/sort.cpp
        src/merge.cpp
        src/pipeline.cpp
        src/prepend.cpp
        src/torna.cpp
        src/config/stem_config.cpp
        src/config/barcode_config.cpp
        src/config/design_config.cpp
        src/io/csv_format.cpp
        src/io/progress.cpp
        src/io/writers.cpp
        src/domain/sequence.cpp
        src/domain/hairpin.cpp
        src/domain/barcode.cpp
    )

    add_executable(fld_tests
        tests/test_main.cpp
        tests/test_design.cpp
        tests/test_preprocess.cpp
        tests/test_sequence.cpp
        tests/test_hairpin.cpp
        tests/test_barcode.cpp
        tests/test_csv_format.cpp
        tests/test_prepend.cpp
        tests/test_torna.cpp
        ${FLD_LIB_SOURCES}
    )

    target_include_directories(fld_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests
    )

    target_link_libraries(fld_tests PRIVATE
        argparse
        indicators
    )

    target_compile_options(fld_tests PRIVATE -O2)

    add_test(NAME fld_tests COMMAND fld_tests)
endif()
