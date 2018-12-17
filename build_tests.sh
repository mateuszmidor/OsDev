#!/bin/bash

mkdir -p build/tests
cd build/tests
cmake -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=DEBUG ../..
make -j `nproc` all
[[ $? -eq 0 ]] || exit 1
ctest
