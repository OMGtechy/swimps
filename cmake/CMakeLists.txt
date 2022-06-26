project(swimps-cmake-dependencies)
cmake_minimum_required(VERSION 3.16)

include(FetchContent)
FetchContent_Declare(
    libsamplerpreload
    GIT_REPOSITORY https://github.com/OMGtechy/libsamplerpreload.git
    GIT_TAG cc812200dce8f9564db5d6ce9c2b619facb4e4a7
)
FetchContent_MakeAvailable(libsamplerpreload)

FetchContent_Declare(
    libcodeinjector
    GIT_REPOSITORY https://github.com/OMGtechy/libcodeinjector.git
    GIT_TAG 3ac55936df8227992c5b4bd0c84d771c31624cad
)
FetchContent_MakeAvailable(libcodeinjector)

add_custom_target(swimps-cmake-dependencies
    DEPENDS samplerpreload codeinjector)

install(TARGETS samplerpreload samplerpreload-utils codeinjector)
