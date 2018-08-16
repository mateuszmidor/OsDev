#!/bin/bash

mkdir -p build/kernel
cd build/kernel
cmake -DBUILD_KERNEL=ON $@ ../..
make -j `nproc` all iso