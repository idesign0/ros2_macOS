# no\_gz\_prefix example

## Configuring project name

This package uses the `NO_PROJECT_PREFIX` option in `gz_configure_project`
to allow a cmake package name without the `gz-` prefix.
To confirm, configure this package and
`package_source` and then observe that the tarball,
pkg-config `.pc` file, and cmake config files omit the `gz-` prefix:

~~~
mkdir build
cd build
cmake ..
make package_source
~~~

* `no_gz_prefix-0.1.0.tar.bz2`
* `cmake/no_gz_prefix-config.cmake`
* `cmake/pkgconfig/no_gz_prefix.pc`

## Configuring include directory names

This package uses the `REPLACE_INCLUDE_PATH` option in `gz_configure_project`
to allow a custom include path of `no_gz`, which doesn't start with `gz/`. To confirm, build the package 
and observe that `AlmostEmpty.cc` compiles successfully while 
including `no_gz/Export.hh`:

~~~
mkdir build
cd build
cmake ..
make
~~~
