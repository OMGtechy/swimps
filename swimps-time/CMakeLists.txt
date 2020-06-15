cmake_minimum_required(VERSION 3.16)
project(swimps-time VERSION 0.0.1 LANGUAGES C)

add_library(swimps-time SHARED source/swimps-time.c)
target_include_directories(swimps-time PUBLIC include)
