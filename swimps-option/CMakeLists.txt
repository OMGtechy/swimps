cmake_minimum_required(VERSION 3.16)
project(swimps-option VERSION 0.0.1 LANGUAGES CXX)

set(CXXOPTS_BUILD_EXAMPLES OFF)
set(CXXOPTS_BUILD_TESTS OFF)
set(CXXOPTS_ENABLE_INSTALL OFF)

include(FetchContent)
FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG v2.2.1
)

FetchContent_MakeAvailable(cxxopts)

add_library(swimps-option SHARED source/swimps-option.cpp)
target_include_directories(swimps-option PUBLIC include cxxopts)
target_link_libraries(swimps-option swimps-log swimps-error cxxopts)
