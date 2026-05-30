cmake_minimum_required(VERSION 3.21)
project(fix-engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for safety-critical systems
add_compile_options(
    -Wall -Wextra -Wpedantic
    -Werror
    -Wconversion -Wsign-conversion
    -Wshadow
    -Wold-style-cast
    -Wnon-virtual-dtor
    -Woverloaded-virtual
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
    -Wimplicit-fallthrough
)

# Library targets
add_library(fix_core STATIC
    src/fix_message.cpp
    src/fix_parser.cpp
    src/fix_session.cpp
)

target_include_directories(fix_core PUBLIC include)

# Sanitizer configuration for CI
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

if(ENABLE_ASAN)
    target_compile_options(fix_core PUBLIC -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(fix_core PUBLIC -fsanitize=address)
endif()

if(ENABLE_UBSAN)
    target_compile_options(fix_core PUBLIC -fsanitize=undefined)
    target_link_options(fix_core PUBLIC -fsanitize=undefined)
endif()

# Tests
enable_testing()
add_executable(fix_tests
    tests/test_fix_parser.cpp
    tests/test_fix_session.cpp
    tests/test_property_based.cpp
)
target_link_libraries(fix_tests PRIVATE fix_core)

# Fuzzing harness
add_executable(fuzz_parser
    fuzz/fuzz_parser.cpp
)
target_link_libraries(fuzz_parser PRIVATE fix_core)

# Benchmark
add_executable(fix_bench
    bench/parser_bench.cpp
)
target_link_libraries(fix_bench PRIVATE fix_core)
