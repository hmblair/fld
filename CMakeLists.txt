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
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    argparse
    indicators
)

target_compile_options(${PROJECT_NAME} PRIVATE -O3)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
