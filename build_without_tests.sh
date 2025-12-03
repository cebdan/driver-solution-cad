#!/bin/bash
# Temporary build script without tests
mkdir -p build
cd build
cmake -G "Unix Makefiles" -DBUILD_TESTS=OFF .. 2>&1 || echo "CMake not found - need to install dependencies"
