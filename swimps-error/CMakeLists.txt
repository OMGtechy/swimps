cmake_minimum_required(VERSION 3.16)
project(swimps-error VERSION 0.0.1 LANGUAGES CXX)

add_library(swimps-error SHARED source/swimps-error.cpp)
target_include_directories(swimps-error PUBLIC include)
