cmake_minimum_required(VERSION 3.16)
project(swimps-trace-file VERSION 0.0.1 LANGUAGES C)

add_library(swimps-trace-file SHARED source/swimps-trace-file.c)
target_include_directories(swimps-trace-file PUBLIC include)
target_link_libraries(swimps-trace-file swimps-io swimps-time swimps-assert)
