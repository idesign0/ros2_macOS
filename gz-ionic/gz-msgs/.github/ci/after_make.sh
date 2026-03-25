# It's necessary to install the python modules for the test.
make install
# Test gz command
export GZ_CONFIG_PATH=/usr/local/share/gz
export LD_LIBRARY_PATH=/usr/local/lib
gz msg
