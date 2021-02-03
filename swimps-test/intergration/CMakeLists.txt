cmake_minimum_required(VERSION 3.16)
project(swimps-intergration-test VERSION 0.0.1 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    catch
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.12.1
)

FetchContent_MakeAvailable(catch)

add_executable(
    swimps-intergration-test
    source/swimps-intergration-test.cpp
    swimps-dwarf-intergration-test/source/dwarf-info-test.cpp
    swimps-io-intergration-test/source/swimps-io-file.cpp
    swimps-option-intergration-test/source/swimps-option-stringify.cpp
)

target_include_directories(swimps-intergration-test PUBLIC include)
target_link_libraries(swimps-intergration-test swimps-dwarf swimps-option Catch2::Catch2)

add_test(NAME swimps-intergration-test
         COMMAND $<TARGET_FILE:swimps-intergration-test>)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/swimps-dwarf-intergration-test/data/cu-test
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/swimps-dwarf-intergration-test/data/)
