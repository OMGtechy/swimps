cmake_minimum_required(VERSION 3.16)
project(swimps-assert VERSION 0.0.1 LANGUAGES C)

add_library(swimps-assert SHARED source/swimps-assert.c)
target_include_directories(swimps-assert PUBLIC include)
target_link_libraries(swimps-assert swimps-log)
