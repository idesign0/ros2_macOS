# c\_static\_child example

This package links against that static library provided by
the `c_no_deps_static` package.

To build, ensure that `c_no_deps_static` has been installed somewhere
in your `CMAKE_PREFIX_PATH` and then run:

~~~
mkdir build
cd build
cmake ..
make
~~~

