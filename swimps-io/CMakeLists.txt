cmake_minimum_required(VERSION 3.16)
project(swimps-io VERSION 0.0.1 LANGUAGES C)

add_library(swimps-io SHARED source/swimps-io.c)
target_include_directories(swimps-io PUBLIC include)
target_include_directories(swimps-io PRIVATE ../swimps-math/include)
