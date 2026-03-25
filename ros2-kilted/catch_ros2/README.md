# catch_ros2
A lightweight wrapper around the [Catch2](https://github.com/catchorg/Catch2/) testing framework for use with ROS 2.

This can be used for unit testing of ROS 2 related functionality or for integration tests to test the functionality of nodes in C++.

## Installation
To install Debian packages of `catch_ros2` simply run the following command (assuming your environment is properly [set up for installing Debian packages](https://docs.ros.org/en/rolling/Installation/Ubuntu-Install-Debians.html)):
```
apt install ros-${ROS_DISTRO}-catch-ros2
```
Debian packages are currently available for Humble, Iron, Jazzy, Kilted, and Rolling ROS 2 distributions.

To build the package from source, clone it into the `src` directory of your workspace root directory (`ws`). Then from `ws` use `colcon build`, like any other ROS 2 package.

## Usage
See the [examples](examples/) section for guides on usage. This README provides an overview, but more details can be found in the links to example source code.

There are two basic ways to use this framework:
1. [Integration testing](#integration-testing)
2. [Unit testing](#unit-testing)

You can also [customize further](#selecting-a-main-function) if you need further control.

### Integration Testing
Integration testing is an important way to ensure different elements of your system work together as desired. In ROS, we generally want to test that our nodes behave correctly, which may be difficult with typical unit testing.

ROS 2 already contains a [framework for writing integration tests in Python](https://github.com/ros2/launch/tree/rolling/launch_testing) which may be more than sufficient for most users. GTest can also be used to test C++ code within an integration test. However, to my knowledge, I have not seen an easy way to create integration tests in C++ with Catch2 - a simple to use but powerful C++ testing framework. The primary motivation of this package is to provide a framework for just that.

#### Components
For a launch integration test in this framework you need 4 components:
1. [Node(s) under test](#nodes-under-test)
2. [Testing node](#testing-node)
3. [Launch file](#launch-file)
4. [CMake setup](#integration-cmake-setup)

##### Node(s) under test
These are the nodes in your package whose functionality you'd like to test. Nothing special has to be done with these nodes to test them with this framework. This example has [`integration_aux_node`](examples/integration_test/integration_aux_node.cpp) as the node under test.

##### Testing node
This node is specifically written with Catch2 test cases and assertions to test the functionality of other nodes. The example has [`integration_test_node`](examples/integration_test/integration_test_node.cpp) as the testing node.

If it is linked with `catch_ros2::catch_ros2_with_node_main` (see [CMake setup](#integration-cmake-setup)) it does not require a `main` function and can be run just like any other ROS 2 node (`ros2 run ${package_name} ${executable_name}`). Tests can be executed manually by just running the node like this (along with other necessary nodes).

If more precise functionality beyond the [default node main](src/node_main.cpp) of this package is desired, see [selecting a main function](#selecting-a-main-function).

##### Launch file
To automate the integration test, a [Python launch file](examples/integration_test/example_integration_test.launch.py) can be used to launch all the necessary nodes for the test, including the test node. An integration test can also be run manually by simply launching this launch file.

This launch file is the same as any other ROS 2 Python launch file, but the `launch_catch_ros2` Python module provides a few useful classes:
- `Catch2LaunchDescription` - a wrapper around the typical [`launch.LaunchDescription`](https://github.com/ros2/launch/blob/rolling/launch/launch/launch_description.py) that includes a "result_file" argument where the results of the test will be output.
- `Catch2IntegrationTestNode` - a wrapper around the typical [`launch_ros.actions.Node`](https://github.com/ros2/launch_ros/blob/rolling/launch_ros/launch_ros/actions/node.py) that passes the "result_file" argument to Catch2 (via the default `catch_ros2` node main function) and shuts down all nodes on exit. This should be used to launch the [testing node](#testing-node). Only one should be used per integration test.

[XML](examples/integration_test/example_integration_test.launch.xml) and [YAML](examples/integration_test/example_integration_test.launch.yaml) launch files can also be used - see the provided links for examples of the above same launch file implemented in those formats.

##### Integration CMake setup
Most of the [CMake required for this integration test](examples/integration_test/CMakeLists.txt) is pretty standard for a ROS 2 package (though the ROS 2 boilerplate CMake is not shown in this example). Nodes are built/installed and the launch file is installed to the package's share directory.

A few items are of note:
```
target_link_libraries(integration_test_node
  catch_ros2::catch_ros2_with_node_main
)
```
This links the integration test node with the version of `catch_ros2` that includes the [default main for running test nodes](src/node_main.cpp). Only one version of the `catch_ros2` library need be linked to a target, depending on the need for a default main. See [selecting a main function](#selecting-a-main-function).

```
catch_ros2_add_integration_test(ExampleIntegration_Test
  LAUNCH_FILE example_integration_test.launch.py
)
```
This custom CMake function is only necessary if the test should be run automatically whenever `colcon test` is run. See [here](cmake/catch_ros2_add_integration_test.cmake) for documentation on the function.

### Unit Testing
Unit testing is a good way to ensure specific elements of your package work well on their own. It's important to ensure functionality is preserved in an ever-evolving codebase.

This package can also be used for unit testing with Catch2. Using Catch2 for unit testing of ROS 2 components is much more straightforward, and you may not need `catch_ros2`. However this package provides a few utilities and a newer version of Catch2 than Ubuntu does.

#### Components
Only 2 components are needed to create a unit test with this framework:
1. [Test source file](#test-source-file)
2. [CMake setup](#unit-cmake-setup)

##### Test source file
The [source file for unit tests](examples/unit_test/example_unit_test.cpp) can be written like any other [Catch2](https://github.com/catchorg/Catch2) unit test.

##### Unit CMake setup
The [CMake setup for unit testing](examples/unit_test/CMakeLists.txt) is pretty standard for adding a Catch2 test. The only thing of note is linking the source file with `catch_ros2::catch_ros2_with_main`, which gives access to `catch_ros2` utilities and uses Catch2's default main function. See [selecting a main function](#selecting-a-main-function).
```
target_link_libraries(example_unit_test
  catch_ros2::catch_ros2_with_main
)
```

#### Utilities
`catch_ros2` provides some utility functions for unit testing in ROS 2.

- [Arguments](include/catch_ros2/arguments.hpp) - utilities for handling command line arguments.
  - `SimulateArgs` - allows you to pass in a string or vector of strings representing command line arguments to create simulated `argc` and `argv` values to pass into functions or `rclcpp::init()`.
    - See [here](examples/unit_test/example_unit_test.cpp) and [here](examples/custom_main/example_test_with_custom_main.cpp) for examples.
  - `SplitROSArgs` - removes ROS arguments from an input `argc` and `argv`, so these arguments won't cause problems for other functions (like Catch2 sessions).
    - See [here](src/node_main.cpp) for an example.
  - `tokenize` - splits a command line string into a vector of strings, ignoring whitespace in quotation marks.
    - See [here](examples/custom_main/example_test_with_custom_main.cpp) for an example.

### Selecting a main function
There are three options for a main function when using `catch_ros2`. Which option is selected depends on which library you link your target to - you should only link to one.

#### 1. Default Node Main
This test node main function (defined [here](src/node_main.cpp)) is good for writing ROS 2 test nodes that use Catch2 (ex: for integration tests). It initializes the ROS context with the input arguments, removes the ROS arguments from the input list, runs the Catch session, then shuts down ROS.

You can use this main by linking to `catch_ros2::catch_ros2_with_node_main`:
```
target_link_libraries(${target_name}
  catch_ros2::catch_ros2_with_node_main
)
```

#### 2. Default Catch2 Main
This main function is Catch2's [default main function](src/default_main.cpp), which is good for typical unit testing scenarios.

You can use this main by linking to `catch_ros2::catch_ros2_with_main`:
```
target_link_libraries(${target_name}
  catch_ros2::catch_ros2_with_main
)
```

#### 3. Defining a custom main
If neither of the above two options satisfy your needs, you can define your own custom main function. See [Catch2's guidelines on this](https://github.com/catchorg/Catch2/blob/devel/docs/own-main.md). See [here](examples/custom_main/example_test_with_custom_main.cpp) for an example.

If you want your test to behave like a node, you can use [the `catch_ros2` node main](src/node_main.cpp) as a starting point.

To define your own main but still use this library, link to `catch_ros2::catch_ros2`:
```
target_link_libraries(${target_name}
  catch_ros2::catch_ros2
)
```

## Catch2 Version
Currently this repository uses the [amalgamated version of Catch2](https://github.com/catchorg/Catch2/blob/devel/docs/migrate-v2-to-v3.md#how-to-migrate-projects-from-v2-to-v3) as a simple way of vendoring v3.4.0 of Catch2. Ubuntu 22.04 currently only provides Catch2 v2, which ROS 2 depends on, so we've made the decision to vendor v3 to be able to use the newest features. In the future, this approach may be abandoned for vendoring Catch2 v3 in its multiple header form.

In order to update the repository for a future release of Catch2 v3, simply copy the amalgamated [header](https://github.com/catchorg/Catch2/blob/devel/extras/catch_amalgamated.hpp) and [source](https://github.com/catchorg/Catch2/blob/devel/extras/catch_amalgamated.cpp) files from the Catch2 repository and replace those [header](include/catch_amalgamated.hpp) and [source](src/catch_amalgamated.cpp) files in this repository. The [default main](src/default_main.cpp) may also need to be updated.

However, since Ubuntu 24.04 provides Catch2 v3.4.0, we have standardized on this version for all ROS distros until a new version is provided by future Ubuntu distros. With some rework of this library the need for vendoring Catch2 v3 could be avoided. Catch2 v3 would still be vendored for ROS distros on 22.04. We may pursue this in the future!

## License
This package is released under the Apache-2.0 License. Catch2 is licensed under the BSL-1.0 License (see their repository for more details).