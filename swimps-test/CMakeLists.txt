cmake_minimum_required(VERSION 3.16)
project(swimps-test VERSION 0.0.1 LANGUAGES C CXX)

include(FetchContent)
FetchContent_Declare(
    catch
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.12.1
)

FetchContent_MakeAvailable(catch)

add_executable(
    swimps-test
    source/swimps-test.cpp
    swimps-io-test/source/swimps-write-to-buffer-test.cpp
    swimps-io-test/source/swimps-write-to-file-descriptor-test.cpp
)

target_include_directories(swimps-test PUBLIC include)
target_link_libraries(swimps-test swimps-io Catch2::Catch2)
