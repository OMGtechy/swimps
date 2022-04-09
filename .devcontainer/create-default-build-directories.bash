#!/usr/bin/env bash

set -e

mkdir -p build
cd build
mkdir -p gcc-10-debug clang-11-debug gcc-10-release clang-11-release
cd gcc-10-debug
CC=gcc-10 CXX=g++-10 cmake -DCMAKE_BUILD_TYPE=Debug ../../
make -j
make test
cd ../gcc-10-release
CC=gcc-10 CXX=g++-10 cmake -DCMAKE_BUILD_TYPE=Release ../../
make -j
make test
cd ../clang-11-debug
CC=clang-11 CXX=clang++-11 cmake -DCMAKE_BUILD_TYPE=Debug ../../
make -j
make test
cd ../clang-11-release
CC=clang-11 CXX=clang++-11 cmake -DCMAKE_BUILD_TYPE=Release ../../
make -j
make test