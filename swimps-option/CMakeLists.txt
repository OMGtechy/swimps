cmake_minimum_required(VERSION 3.16)
project(swimps-option VERSION 0.0.1 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11
    GIT_TAG v2.1.2
)

FetchContent_MakeAvailable(cli11)

add_library(swimps-option SHARED source/swimps-option-parser.cpp source/swimps-option-options.cpp)
target_include_directories(swimps-option PUBLIC include)
target_link_libraries(swimps-option CLI11::CLI11 swimps-log swimps-error swimps-assert swimps-time swimps-trace-file)
