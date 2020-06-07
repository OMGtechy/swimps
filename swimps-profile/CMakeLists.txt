cmake_minimum_required(VERSION 3.16)
project(swimps-profile VERSION 0.0.1 LANGUAGES C)

add_library(swimps-profile SHARED source/swimps-profile.c source/swimps-profile-child.c)
target_include_directories(swimps-profile PUBLIC include)
target_link_libraries(swimps-profile swimps-error swimps-log swimps-io)

# we don't want to link against it, but we depend on
# injecting it into other processes
add_dependencies(swimps-profile swimps-preload)
