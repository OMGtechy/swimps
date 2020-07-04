cmake_minimum_required(VERSION 3.16)
project(swimps VERSION 0.0.1 LANGUAGES C)

add_compile_options(-Wall -Wextra -pedantic -Werror)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined -fsanitize=address")

add_subdirectory(swimps-test)
add_subdirectory(swimps-preload)
add_subdirectory(swimps-log)
add_subdirectory(swimps-io)
add_subdirectory(swimps-profile)
add_subdirectory(swimps-error)
add_subdirectory(swimps-time)
add_subdirectory(swimps-trace)
add_subdirectory(swimps-trace-file)
add_subdirectory(swimps-assert)

add_executable(swimps source/swimps.c)
target_link_libraries(swimps PRIVATE swimps-profile)
