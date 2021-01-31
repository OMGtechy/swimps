cmake_minimum_required(VERSION 3.16)
project(swimps-tui VERSION 0.0.1 LANGUAGES CXX)

add_library(swimps-tui SHARED source/swimps-tui.cpp)
target_include_directories(swimps-tui PUBLIC include)
target_link_libraries(swimps-tui ncurses swimps-analysis swimps-assert swimps-error swimps-trace)
