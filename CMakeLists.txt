cmake_minimum_required(VERSION 3.10)
project(fld CXX)

# Set CXX standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check if using GNU or Clang compiler
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall)
endif()

# find_package(PkgConfig REQUIRED)
# pkg_check_modules(VIENNARNA REQUIRED RNAlib2)

# Get argparse
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

FetchContent_Declare(
    indicators
    GIT_REPOSITORY https://github.com/p-ranav/indicators.git
)
FetchContent_MakeAvailable(indicators)

# Add executable
add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/library.cpp
    src/nuc.cpp
    src/utils.cpp
    # src/fold.cpp
    src/preprocess.cpp
    src/inspect.cpp
    src/barcodes.cpp
    src/m2.cpp
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    argparse
    indicators
    # ${VIENNARNA_LIBRARIES}
)
# target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${VIENNARNA_INCLUDE_DIRS})

# Add additional library directories explicitly
# target_link_directories(${PROJECT_NAME} PRIVATE ${VIENNARNA_LIBRARY_DIRS} /usr/local/lib)

# Compiler options
target_compile_options(${PROJECT_NAME} PRIVATE -O2 -g)

# Installation target
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
