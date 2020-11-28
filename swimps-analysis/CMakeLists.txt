cmake_minimum_required(VERSION 3.16)
project(swimps-analysis VERSION 0.0.1 LANGUAGES CXX)

add_library(swimps-analysis SHARED source/swimps-analysis.cpp)
target_include_directories(swimps-analysis PUBLIC include)
target_link_libraries(swimps-analysis swimps-trace)
