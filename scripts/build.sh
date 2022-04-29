#!/bin/bash
set -e

echo "Building..."

if [ -z ${BUILD_DIR+x} ]; then
    export BUILD_DIR=build
fi

mkdir -p $BUILD_DIR
cd $BUILD_DIR

CPUS=1
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    CPUS=$(grep -c ^processor /proc/cpuinfo)
elif [[ "$OSTYPE" == "darwin"* ]]; then
    CPUS=$(sysctl -n hw.ncpu)
fi

CMAKE_BUILD_TYPE="Debug"
if [[ "$BUILD_RELEASE" == "1" ]]; then
    CMAKE_BUILD_TYPE="Release"
elif [[ "$BUILD_PROFILING" == "1" ]]; then
    CMAKE_BUILD_TYPE="Profiling"
fi

eval "cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} .."
make -j$CPUS
