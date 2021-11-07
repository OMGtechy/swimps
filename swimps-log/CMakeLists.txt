cmake_minimum_required(VERSION 3.16)
project(swimps-log VERSION 0.0.1 LANGUAGES CXX)

add_library(swimps-log SHARED source/swimps-log.cpp)
target_include_directories(swimps-log PUBLIC include)
target_link_libraries(swimps-log swimps-container swimps-io signalsafe)
