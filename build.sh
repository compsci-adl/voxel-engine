#!/bin/sh

set -e

if [ "$1" = "release" ]; then
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
else
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
fi

cmake --build build

./build/voxel-engine
