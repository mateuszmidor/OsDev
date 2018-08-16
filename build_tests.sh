#!/bin/bash

mkdir -p build/tests
cd build/tests
cmake -DBUILD_TESTS=ON ../..
make -j `nproc` all
ctest