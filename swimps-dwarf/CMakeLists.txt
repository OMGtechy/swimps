cmake_minimum_required(VERSION 3.16)
project(swimps-dwarf VERSION 0.0.1 LANGUAGES CXX)

add_library(swimps-dwarf SHARED source/swimps-dwarf.cpp)
target_include_directories(swimps-dwarf PUBLIC include)
target_link_libraries(swimps-dwarf dwarf swimps-io)
