cmake_minimum_required(VERSION 3.16)
project(swimps-io VERSION 0.0.1 LANGUAGES CXX)

add_library(swimps-io SHARED source/swimps-io.cpp)
target_include_directories(swimps-io PUBLIC include)
target_link_libraries(swimps-io swimps-assert m signalsafe)
