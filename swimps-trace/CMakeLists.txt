cmake_minimum_required(VERSION 3.16)
project(swimps-trace VERSION 0.0.1 LANGUAGES CXX)

add_library(swimps-trace SHARED source/swimps-trace.cpp)
target_include_directories(swimps-trace PUBLIC include)
target_link_libraries(swimps-trace signalsafe signalsampler swimps-dwarf swimps-time)
