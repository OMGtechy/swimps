cmake_minimum_required(VERSION 3.16)
project(swimps-assert VERSION 0.0.1 LANGUAGES CXX)

add_library(swimps-assert SHARED source/swimps-assert.cpp)
target_include_directories(swimps-assert PUBLIC include)
