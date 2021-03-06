cmake_minimum_required(VERSION 3.16)
project(swimps-system-test VERSION 0.0.1 LANGUAGES CXX)

add_test(NAME swimps-system-test-no-args
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py non-zero no-traces ${swimps_BINARY_DIR}/swimps)

add_test(NAME swimps-system-test-no-program-args
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py zero traces ${swimps_BINARY_DIR}/swimps --no-tui ${swimps-system-test_BINARY_DIR}/swimps-dummy)

add_test(NAME swimps-system-test-program-args
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py zero traces ${swimps_BINARY_DIR}/swimps --no-tui ${swimps-system-test_BINARY_DIR}/swimps-dummy 5)

add_test(NAME swimps-system-test-good-swimps-args
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py zero traces ${swimps_BINARY_DIR}/swimps --no-tui --log-level debug ${swimps-system-test_BINARY_DIR}/swimps-dummy)

add_test(NAME swimps-system-test-missing-log-level-args
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py non-zero no-traces ${swimps_BINARY_DIR}/swimps --log-level)

add_test(NAME swimps-system-test-bad-log-level-args
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py non-zero no-traces ${swimps_BINARY_DIR}/swimps --log-level this is sparta ${swimps-system-test_BINARY_DIR}/swimps-dummy 0)

add_test(NAME swimps-system-test-missing-sample-rate-args
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py non-zero no-traces ${swimps_BINARY_DIR}/swimps --samples-per-second)

add_test(NAME swimps-system-test-bad-sample-rate-args
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py non-zero no-traces ${swimps_BINARY_DIR}/swimps --samples-per-second -1 ${swimps-system-test_BINARY_DIR}/swimps-dummy 0)

add_test(NAME swimps-system-test-high-sample-rate
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py zero traces ${swimps_BINARY_DIR}/swimps --no-tui --samples-per-second 100 ${swimps-system-test_BINARY_DIR}/swimps-dummy 3)

add_test(NAME swimps-system-test-help
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py zero no-traces ${swimps_BINARY_DIR}/swimps --help)

add_test(NAME swimps-system-test-relative-path
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py zero traces ${swimps_BINARY_DIR}/swimps --no-tui ./swimps-dummy 0)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data/swimps_trace_swimps-dummy_1289386_91242411
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/data)

add_test(NAME swimps-system-test-load-profile
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/source/swimps-system-test.py zero traces ${swimps_BINARY_DIR}/swimps --load --target-trace-file ${CMAKE_CURRENT_BINARY_DIR}/data/swimps_trace_swimps-dummy_1289386_91242411 --no-tui)

add_executable(swimps-dummy source/swimps-dummy.cpp)
