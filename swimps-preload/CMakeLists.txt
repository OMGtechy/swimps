cmake_minimum_required(VERSION 3.16)
project(swimps-preload VERSION 0.0.1 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    libsignalsampler
    GIT_REPOSITORY https://github.com/OMGtechy/libsignalsampler.git
    GIT_TAG 771e01b8a8b7ee934cb822767c60850eb3145d43
)

FetchContent_MakeAvailable(libsignalsampler)

add_library(swimps-preload SHARED source/swimps-preload.cpp source/swimps-preload-procmaps.cpp)
target_include_directories(swimps-preload SYSTEM BEFORE PUBLIC ${LIBUNWIND_INCLUDE})
target_link_directories(swimps-preload BEFORE PUBLIC ${LIBUNWIND_LIB})

target_include_directories(swimps-preload PUBLIC include)
target_link_libraries(swimps-preload signalsampler swimps-time swimps-log swimps-trace-file swimps-option unwind)
