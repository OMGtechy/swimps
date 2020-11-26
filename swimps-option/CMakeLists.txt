cmake_minimum_required(VERSION 3.16)
project(swimps-option VERSION 0.0.1 LANGUAGES CXX)

add_library(swimps-option SHARED source/swimps-option.cpp)
target_include_directories(swimps-option PUBLIC include)
target_link_libraries(swimps-option swimps-log swimps-error swimps-assert swimps-time swimps-trace-file)
