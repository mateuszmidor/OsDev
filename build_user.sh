#!/bin/bash

mkdir -p build
cd build
cmake -DBUILD_USER=ON ..
make -j `nproc` all install