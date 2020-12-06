cmake_minimum_required(VERSION 3.16)
project(swimps-preload VERSION 0.0.1 LANGUAGES CXX)

add_library(swimps-preload SHARED source/swimps-preload.cpp source/swimps-preload-backtrace.cpp)
target_include_directories(swimps-preload SYSTEM BEFORE PUBLIC ${LIBUNWIND_INCLUDE})
target_link_directories(swimps-preload BEFORE PUBLIC ${LIBUNWIND_LIB})

target_include_directories(swimps-preload PRIVATE include)
target_link_libraries(swimps-preload swimps-time swimps-log swimps-trace-file swimps-option unwind)
