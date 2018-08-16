#!/bin/bash

mkdir -p build
cd build
cmake -DBUILD_KERNEL=ON ..
make -j `nproc` all iso