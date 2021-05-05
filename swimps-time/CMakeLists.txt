cmake_minimum_required(VERSION 3.16)
project(swimps-time VERSION 0.0.1 LANGUAGES CXX)

add_library(swimps-time SHARED source/swimps-time.cpp)
target_include_directories(swimps-time PUBLIC include)
target_link_libraries(swimps-time rt swimps-assert)
