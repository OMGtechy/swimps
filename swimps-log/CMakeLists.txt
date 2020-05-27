cmake_minimum_required(VERSION 3.16)
project(swimps-log VERSION 0.0.1 LANGUAGES C)

add_library(swimps-log SHARED source/swimps-log.c)
target_include_directories(swimps-log PUBLIC include)
target_link_libraries(swimps-log swimps-io)
