#!/bin/sh -l

set -x

BUILD_DIR=`pwd`

# Install
make install

# Compile examples
cd ../examples
mkdir build
cd build
cmake ..
make
./graph_example

# Compile python bindings
cd $BUILD_DIR/../src/python_pybind11
mkdir build;
cd build;
cmake ..;
make;

cd $BUILD_DIR
