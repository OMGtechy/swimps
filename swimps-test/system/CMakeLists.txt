cmake_minimum_required(VERSION 3.16)
project(swimps-system-test VERSION 0.0.1 LANGUAGES CXX)

add_test(NAME swimps-system-test-no-args
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py zero ${swimps_BINARY_DIR}/swimps ${swimps-system-test_BINARY_DIR}/swimps-dummy)

add_test(NAME swimps-system-test-program-args
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py zero ${swimps_BINARY_DIR}/swimps ${swimps-system-test_BINARY_DIR}/swimps-dummy 5)

add_test(NAME swimps-system-test-good-swimps-args
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py zero ${swimps_BINARY_DIR}/swimps --log-level debug ${swimps-system-test_BINARY_DIR}/swimps-dummy)

add_test(NAME swimps-system-test-bad-swimps-args
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py non-zero ${swimps_BINARY_DIR}/swimps --log-level this is sparta ${swimps-system-test_BINARY_DIR}/swimps-dummy 0)

add_test(NAME swimps-system-test-high-sample-rate
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py zero ${swimps_BINARY_DIR}/swimps --samples-per-second 1000 ${swimps-system-test_BINARY_DIR}/swimps-dummy 3)

add_executable(swimps-dummy source/swimps-dummy.cpp)
