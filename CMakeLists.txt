cmake_minimum_required(VERSION 3.16)
project(swimps VERSION 0.0.1 LANGUAGES CXX)

enable_testing()

add_compile_options(-Wall -Wextra -pedantic -Werror)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined -fno-omit-frame-pointer -g3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined -fno-omit-frame-pointer -g3")

option(SWIMPS_ENABLE_COVERAGE "Enable code coverage flags." OFF)
if(SWIMPS_ENABLE_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    link_libraries(gcov)
endif()

include(FetchContent)
FetchContent_Declare(
    libsignalsafe
    GIT_REPOSITORY https://github.com/OMGtechy/libsignalsafe.git
    GIT_TAG c1e1b113cecb4ebd4433834ac4b375ddfdfd9b6e
)

FetchContent_MakeAvailable(libsignalsafe)

add_subdirectory(swimps-test)
add_subdirectory(swimps-analysis)
add_subdirectory(swimps-option)
add_subdirectory(swimps-preload)
add_subdirectory(swimps-log)
add_subdirectory(swimps-profile)
add_subdirectory(swimps-error)
add_subdirectory(swimps-time)
add_subdirectory(swimps-trace)
add_subdirectory(swimps-trace-file)
add_subdirectory(swimps-tui)
add_subdirectory(swimps-dwarf)
add_subdirectory(swimps-assert)

add_executable(swimps source/swimps.cpp)
target_link_libraries(swimps PRIVATE swimps-profile swimps-option swimps-analysis swimps-trace-file swimps-tui)
