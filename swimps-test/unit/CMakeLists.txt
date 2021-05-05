cmake_minimum_required(VERSION 3.16)
project(swimps-unit-test VERSION 0.0.1 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    catch
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.12.1
)

FetchContent_MakeAvailable(catch)

add_executable(
    swimps-unit-test
    source/swimps-unit-test.cpp
    swimps-container-unit-test/source/span-test.cpp
    swimps-io-unit-test/source/swimps-write-to-buffer-test.cpp
    swimps-io-unit-test/source/swimps-format-string-test.cpp
    swimps-log-unit-test/source/swimps-format-log-message-test.cpp
    swimps-option-unit-test/source/swimps-parse-command-line.cpp
)

target_include_directories(swimps-unit-test PUBLIC include)
target_link_libraries(swimps-unit-test swimps-io swimps-option swimps-log swimps-container swimps-trace-file Catch2::Catch2)

add_test(NAME swimps-unit-test
         COMMAND $<TARGET_FILE:swimps-unit-test>)
