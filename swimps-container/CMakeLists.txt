cmake_minimum_required(VERSION 3.16)
project(swimps-container VERSION 0.0.1 LANGUAGES CXX)

add_library(swimps-container SHARED source/swimps-container.cpp)
target_include_directories(swimps-container PUBLIC include)

target_link_libraries(swimps-container swimps-assert)
