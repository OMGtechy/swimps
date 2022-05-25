cmake_minimum_required(VERSION 3.16)
project(swimps-profile VERSION 0.0.1 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    libcodeinjector
    GIT_REPOSITORY https://github.com/OMGtechy/libcodeinjector.git
    GIT_TAG a19e6102e0f5ec393f0d09eb89dab17958a36073
)

FetchContent_MakeAvailable(libcodeinjector)

add_library(swimps-profile SHARED source/swimps-profile.cpp source/swimps-profile-child.cpp source/swimps-profile-parent.cpp)
target_include_directories(swimps-profile PUBLIC include)
target_link_libraries(swimps-profile codeinjector samplerpreload-utils swimps-error swimps-log swimps-option)

# we don't want to link against it, but we depend on
# injecting it into other processes
add_dependencies(swimps-profile samplerpreload)
add_custom_command(
    TARGET swimps-profile
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${libsamplerpreload_BINARY_DIR}/libsamplerpreload.so
        ${libsamplerpreload_BINARY_DIR}/libsamplerpreload-utils.so
        ${CMAKE_CURRENT_BINARY_DIR})
