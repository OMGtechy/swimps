cmake_minimum_required(VERSION 3.16)
project(swimps-preload VERSION 0.0.1 LANGUAGES CXX)

add_library(swimps-preload SHARED source/swimps-preload.cpp)
target_include_directories(swimps-preload PRIVATE include)
target_link_libraries(swimps-preload swimps-time swimps-log swimps-trace-file)
