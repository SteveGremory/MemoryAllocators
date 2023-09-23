cmake_minimum_required(VERSION 3.22)
project(NetworkStack)

# Set C++ options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GTest & GBench
set(GTEST_GIT_TAG v1.13.0)
set(GBENCH_GIT_TAG v1.8.3)
set(BENCHMARK_ENABLE_TESTING OFF)
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG ${GTEST_GIT_TAG}
)
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG ${GBENCH_GIT_TAG}
)
FetchContent_MakeAvailable(googletest googlebenchmark)
message("-- Fetched GBench version: ${GBENCH_GIT_TAG}")
message("-- Fetched GTest version: ${GTEST_GIT_TAG}")

# libmem
file(GLOB_RECURSE libmem_SOURCES
        source/*.cpp
        source/libmem/*.cpp
)
message(${libmem_SOURCES})
add_library(libmem ${libmem_SOURCES})
target_include_directories(libmem PUBLIC 
        source/
)
target_link_libraries(libmem PUBLIC asan)

target_compile_options(libmem PRIVATE
        -Wall -fsanitize=address -Wextra
)

# An exec to quick-test with
add_executable(Main bin/main.cpp)
target_link_libraries(Main PUBLIC libmem)

# Tests
file(GLOB_RECURSE TESTS_SOURCES
        tests/*.cpp
)
add_executable(tests ${TESTS_SOURCES})
target_link_libraries(
    tests PUBLIC
        GTest::gtest_main
        libmem
)
include(GoogleTest)
gtest_discover_tests(tests)

# Benchmarks
file(GLOB_RECURSE BENCHMARKS_SOURCES
    benchmarks/*.cpp
)
add_executable(benchmarks ${BENCHMARKS_SOURCES})
target_link_libraries(
    benchmarks PUBLIC
        benchmark::benchmark
        libmem
)