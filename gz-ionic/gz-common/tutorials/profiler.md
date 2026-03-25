\page profiler Profiler

Next Tutorial: \ref hw-encoding

## Overview

This tutorial describes how to get started using the Gazebo Common profiler
to measure and visualize run-time performance of your software.

The `gz::common::Profiler` provides a common interface that can allow for
multiple underlying profiler implementations. Currently, the only available
implementation is [Remotery](https://github.com/Celtoys/Remotery).

The goal of the profiler is to provide introspection and analysis when enabled
at compile time, but to introduce no overhead when it is disabled at compile-time.

To control if the profiler is enabled, set the `GZ_PROFILER_ENABLE` flag using
cmake on the targets or sources that you are interested in (described below).

## Enabling the Profiler

### On custom example

In order to use the profiler, inspection points must be added to the source code,
and the application or library must be linked to the `gz-common::profiler`
component.

To start, download the [profiler.cc](https://github.com/gazebosim/gz-common/raw/main/examples/profiler.cc) example.

The relevant corresponding C++ would be as follows:

```{.cpp}
// Add the profiler header
#include <gz/common/Profiler.hh>

...
void thread(const char *_thread_name)
{
  // Sets the name of the thread to appear in the UI
  GZ_PROFILE_THREAD_NAME(_thread_name);
  while (running)
  {
    // Add a profiling point to this scope.
    GZ_PROFILE("Loop");
    // Execute some arbitrary tasks
    for (size_t ii = 0; ii < 10; ++ii)
    {
      task1();
    }
    task2();
    task3();
  }
}
```

Update your CMakeLists.txt to the following. Note that the profiler must be
enabled at compile time in order to function.

```{.cpp}
cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

# Find the gz-common library
find_package(gz-common6 QUIET REQUIRED COMPONENTS profiler)

add_executable(profiler_example profiler.cc)
target_link_libraries(profiler_example gz-common6::profiler)
# Enable the profiler for the example
target_compile_definitions(profiler_example PUBLIC "GZ_PROFILER_ENABLE=1")
```

Run `cmake` and build the example

```{.sh}
cd build
cmake ..
make profiler_example
```

Then execute the example and the profiler visualization:

From terminal 1:

```{.sh}
./profiler_example
```

From terminal 2, open the visualizer using one of the following commands

```{.sh}
# Find the launcher script and use it (Linux and macOS)
find /usr | grep gz_remotery_vis
...

/usr/<path_to>/gz_remotery_vis

# Use the source path (Linux)
# Substitute the path to your gz-common source checkout
xdg-open $SOURCE_DIR/gz-common/profiler/src/Remotery/vis/index.html

# Use the installation path (Linux)
# This may vary depending on where you have choosen to install
xdg-open /usr/share/gz/gz-common6/profiler_vis/index.html

# Use the installation path (macOS)
open /usr/share/gz/gz-common6/profiler_vis/index.html

# Inside a Docker container with port 8000 exposed
# 1. Find your container's IP with 'ifconfig'
# 2. Start a basic web server:
python3 -m http.server $SOURCE_DIR/gz-common/profiler/src/Remotery/vis/index.html
# 3. Open URL "http://<container IP>:8000/" with a browser on the host.
```

### On Gazebo library

If you want to use profiler on any other Gazebo library, enable the profiler at compile time with ``ENABLE_PROFILER`` cmake argument.

When compiling with ``CMake``:
```{.sh}
cmake .. -DENABLE_PROFILER=1
```
When compiling with ``colcon``:
```{.sh}
colcon build --cmake-args -DENABLE_PROFILER=1
```

Run your Gazebo library then go to your Gazebo installation path and open the profiler browser using:
```{.sh}
<workspace>/install/libexec/gz/gz-common<N>/gz_remotery_vis
```

If the profiler is run successfully, you should see output in a browser. Similar to this

<img src="https://raw.githubusercontent.com/gazebosim/gz-common/main/tutorials/imgs/profiler_tutorial_example.png">

### Troubleshoot the web viewer

If you see ``connection error``, there are a couple of things to double check
1. Was the profiler enabled when the project you're trying to run was compiled? Note that this isn't the case if you installed Gazebo libraries from binaries, for example. You need to compile the project from source with the `ENABLE_PROFILER` variable set.
2. Are you using the correct port number in the upper left corner ``Connection Addresss: ws://127.0.0.1:1500/rmt``? Running ``gz sim -v 4`` will show the port number in use near the top of the outputted text. The port number will be printed out if the profiler is enabled.
  ```{.sh}
  [Dbg] [RemoteryProfilerImpl.cc:187] Starting gz-common profiler impl: Remotery (port: 1500)
  ```
3. Are you running the program in a separate terminal? The profiler only establishes connection if there is a program running and being actively profiled.

4. If you want to use a different port, configure the environment variable `RMT_PORT` by running the following in terminal, and update the web viewer port in your browser accordingly (see 2 above)
  ```{.sh}
  export RMT_PORT=1500
  ```


## Using the Profiler

The profiler is used through a series of macros.

The two primary ways of profiling a section of code are to either use
a matched pair of `GZ_PROFILE_BEGIN` and `GZ_PROFILE_END` macros, or to use
a single RAII-style macro `GZ_PROFILE`. The RAII style will stop measuring
once the scope that the macro was invoked in is left.

Using begin/end:

```{.cpp}
  // An example of using start/stop profiling.
  GZ_PROFILE_BEGIN("a");
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
  GZ_PROFILE_END();
```

Using RAII-style:

```{.cpp}
  {
    // An example of using scope-based profiling.
    GZ_PROFILE("a");
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
```

Additionally, each thread can be given a name for easy reference in the UI:

```{.cpp}
  GZ_PROFILE_THREAD_NAME("main");
  GZ_PROFILE_THREAD_NAME("physics");
  GZ_PROFILE_THREAD_NAME("gui");
```

## Configuring the Profiler

Specific profiler implementations may have further configuration options available.

### Configuring Remotery

Remotery can additionally be configured via environment variables. Most users
should not need to change these for their applications.

 * `RMT_PORT`: Port to listen for incoming connections on.
 * `RMT_QUEUE_SIZE`: Size of the internal message queues
 * `RMT_MSGS_PER_UPDATE`: Upper limit on messages consumed per loop
 * `RMT_SLEEP_BETWEEN_UPDATES`: Controls profile server update rate.

These directly set the corresponding parameters in the `rmtSettings` structure.
For more information, consult the [Remotery source](https://github.com/Celtoys/Remotery/blob/8c3923a04493cd1cb3d21cfdb8ad6fb21b394b96/lib/Remotery.h#L354)
