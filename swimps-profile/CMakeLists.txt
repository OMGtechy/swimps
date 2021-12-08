cmake_minimum_required(VERSION 3.16)
project(swimps-profile VERSION 0.0.1 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    libcodeinjector
    GIT_REPOSITORY https://github.com/OMGtechy/libcodeinjector.git
    GIT_TAG d2086f1eb48ad7117c8be793a60e8218689b9857
)

FetchContent_MakeAvailable(libcodeinjector)

add_library(swimps-profile SHARED source/swimps-profile.cpp source/swimps-profile-child.cpp source/swimps-profile-parent.cpp)
target_include_directories(swimps-profile PUBLIC include)
target_link_libraries(swimps-profile codeinjector swimps-error swimps-log swimps-option)

# we don't want to link against it, but we depend on
# injecting it into other processes
add_dependencies(swimps-profile swimps-preload)
