# c\_nodep\_static example

This package sets the `BUILD_SHARED_LIBS` cmake variable to `OFF`,
ensuring that a static library will be created.
The package has no dependencies.

To build:

~~~
mkdir build
cd build
cmake ..
make
~~~
