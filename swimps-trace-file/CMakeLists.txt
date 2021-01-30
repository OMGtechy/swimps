cmake_minimum_required(VERSION 3.16)
project(swimps-trace-file VERSION 0.0.1 LANGUAGES CXX)

add_library(swimps-trace-file SHARED source/swimps-trace-file.cpp)
target_include_directories(swimps-trace-file PUBLIC include)
target_link_libraries(swimps-trace-file swimps-assert swimps-container swimps-dwarf swimps-error swimps-io swimps-log swimps-time swimps-trace)
