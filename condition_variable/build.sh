#!/bin/bash

SCRIPTPATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CMAKE_DIR=$SCRIPTPATH/build-dir/linux/cmake

mkdir -p $CMAKE_DIR
cd $CMAKE_DIR

# find vcpkg in $PATH
#   if vcpkg is not in $PATH, then the cmake script will download and install vcpkg
VCPKG_ROOT="$( dirname "$( which vcpkg )" )"
echo $VCPKG_ROOT

cmake -G "Unix Makefiles" $SCRIPTPATH \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    && make && ./condition_variable

cd $SCRIPTPATH
