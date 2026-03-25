#!/bin/sh -l

set -x

# Install
make install

# So gz-tools finds fuel-tools
export GZ_CONFIG_PATH=/usr/local/share/gz
export LD_LIBRARY_PATH=/usr/local/lib/:$LD_LIBRARY_PATH

# Compile example
cd ../example
mkdir build
cd build
cmake ..
make
cd ../../build
