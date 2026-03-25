# version\_from\_package\_xml example

## Getting a version number from package.xml file

The package.xml file format (defined in
[ROS REP 127](https://ros.org/reps/rep-0127.html),
[ROS REP 140](https://ros.org/reps/rep-0140.html), and
[ROS REP 149](https://ros.org/reps/rep-0149.html))
can be used to encode metadata about a package, including its version number.
This example shows how to use the `gz_get_package_xml_version` helper function
to get the version number from a package.xml file and use that to set the
cmake project version.

The package.xml file in this folder encodes a version number `8.21.65`.
Configuring this example with cmake will parse the package.xml file and
print the cmake variables provided by the helper function.

~~~
mkdir build
cd build
cmake ..
~~~

You should see the following console output.

~~~
-- PACKAGE_XML_VERSION 8.21.65
-- PACKAGE_XML_VERSION_MAJOR 8
-- PACKAGE_XML_VERSION_MINOR 21
-- PACKAGE_XML_VERSION_PATCH 65
-- gz-version_from_package_xml version 8.21.65
~~~
