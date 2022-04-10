cmake_minimum_required(VERSION 3.16)
project(swimps-benchmark-test VERSION 0.0.1 LANGUAGES CXX)

include(FetchContent)

set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_INSTALL OFF)
set(BUILD_GMOCK OFF)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.5.2
)

FetchContent_MakeAvailable(benchmark)

add_executable(
    swimps-benchmark-test
    source/swimps-benchmark-test.cpp
    swimps-preload-benchmark-test/source/swimps-sigprof-handler-benchmark.cpp
)

target_include_directories(swimps-benchmark-test PUBLIC include)
target_link_libraries(swimps-benchmark-test benchmark::benchmark swimps-preload)

