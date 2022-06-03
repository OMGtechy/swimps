project(swimps-cmake-dependencies)
cmake_minimum_required(VERSION 3.16)

include(FetchContent)
FetchContent_Declare(
    libsamplerpreload
    GIT_REPOSITORY https://github.com/OMGtechy/libsamplerpreload.git
    GIT_TAG 6c2862a964ed028677be89c999bb8d2e7da1c695
)
FetchContent_MakeAvailable(libsamplerpreload)

add_custom_target(swimps-cmake-dependencies
    DEPENDS samplerpreload)
