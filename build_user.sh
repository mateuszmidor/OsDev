#!/bin/bash

mkdir -p build/user
cd build/user
cmake -DBUILD_USER=ON $@ ../..
make -j `nproc` all install