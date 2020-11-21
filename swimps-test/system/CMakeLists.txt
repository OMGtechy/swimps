cmake_minimum_required(VERSION 3.16)
project(swimps-system-test VERSION 0.0.1 LANGUAGES CXX)

add_test(NAME swimps-system-test
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py ${swimps_BINARY_DIR}/swimps --target-program ${swimps-system-test_BINARY_DIR}/swimps-dummy --log-level debug)

add_executable(swimps-dummy source/swimps-dummy.cpp)
