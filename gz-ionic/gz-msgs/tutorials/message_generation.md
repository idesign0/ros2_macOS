\page messagegeneration Message Generation

## Overview

This tutorial describes how Gazebo messages are generated and you can create
custom messages that can be used with the simulator and command line tools.

Gazebo messages use [Protobuf](https://protobuf.dev) to define the structures
that can be easily serialized for use in communication through the Gazebo
software stack.  Protobuf is a language-neutral framework for serializing
structured data, with the advantage of generating native language bindings
so that messages are easily used from your language of choice.

## Message Definitions

### File structure

Gazebo message definitions are stored in the
[proto](https://github.com/gazebosim/gz-msgs/tree/main/proto) folder.

Messages may additionally belong to a `package`, which allows for convenient
namespacing or grouping of common messages.
Currently, all `gz-msgs` definitions reside in the `gz.msgs` package, which
generates the corresponding `gz::msgs` namespace in C++ and `gz․msgs` in Python.

### Proto files

Messages are defined in files with the extension `.proto`.

The definitions use the `proto` language, which has two major version, of which
Gazebo uses the `proto3` version.

The full language guide for the `proto` language is available [here](https://protobuf.dev/programming-guides/proto3/)

A typical message definition looks something like this:

```{proto}
/// First non-comment line must define using the proto3 syntax.
syntax = "proto3";

/// Define the package name we are using
package gz.msgs;

/// Import common message definitions
import "gz/msgs/header.proto";
import "gz/msgs/vector3d.proto";

message FooMessage
{
    /// Message header data (timestamp)
    /// Since this definition is also in gz.msgs, the fully-qualified
    /// name isn't necessary here, but left in for explicitness.
    gz.msgs.Header header = 1;

    /// Define a field using one of the protobuf scalar types
    string name = 2;

    /// Define a field using on the Gazebo message types
    gz.msgs.Vector3d position = 3;
}
```

Note that each field has a corresponding *field number*.
Each field must be given an ID between `1` and `536,870,911`, with the following
restrctions (from the [language guide](https://protobuf.dev/programming-guides/proto3/#assigning)):

* The given number must be unique among all fields for that message.
* Field numbers `19,000` to `19,999` are reserved for the Protocol Buffers implementation.
  The protocol buffer compiler will complain if you use one of these reserved field numbers in your message.
* You cannot use any previously reserved field numbers or any field numbers that have been allocated to extensions.



## The Message Generation Pipeline

Once the proto messages have been defined, there are several steps that take
place to make the messages usable across the Gazebo stack.

To begin with, each message definition is passed through the `gz_msgs_protoc`
function:

\image html files/gz_msgs_protoc.svg

For each file:

* `gz_msgs_protoc` accepts the arguments necessary to find all the supporting
scripts and executables:
 * `gz_msgs_generate.py` - Top-level python script entrypoint for generating messages.
 * `protoc` - The protobuf compiler
 * `gz-msgs-protoc-compiler` - The Gazebo messages extension compiler.
* The protobuf compiler generates `.pb.c` , `.pb.h`, and `_pb2.py` file
  for each message found in the `.proto` file.
* The generation python script moves the generated `.pb.h` file into a
  `details/` directory to hide some of the protobuf implementation from
  downstream users.
* The `gz-msgs-protoc-compiler` generates a new `.pb.h` file with Gazebo-specific definitions
* `gz_msgs_generate.py` additionally generates a `.pbindex` file to be used by later
  steps in the process.


After individual message files have been generated, all of the definitions
are grouped and processed via `gz_msgs_factory`

\image html files/gz_msgs_factory.svg

For the collection, `gz_msgs_factory` generates:

* A `register.cc` file that can be used to statically register all of the
  messages passed as arguments.
* A `MessageTypes.hh` file that enumerates all messages available in the
  collection.
* A `.gz_desc` file that allows the messages to be dynamically loaded by the
  various Gazebo tools.


## Custom Message Generation

Now that we understand the components of the message generation pipeline,
we can use them in our own custom package.

The code for this example can be found in the `gz-msgs` [repository](https://github.com/gazebosim/gz-msgs/tree/main), in the [`examples/generating_custom_msgs`](https://github.com/gazebosim/gz-msgs/tree/main/examples/generating_custom_msgs) folder.



The `cmake` functionality is exported from the `gz-msgs` library, via the `gz-cmake` [`extras` functionality](https://github.com/gazebosim/gz-cmake/pull/345).
To make the functions available, simply `find_package(gz-msgs11)` in your `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(my_custom_package VERSION 0.0.1)
find_package(gz-cmake4 REQUIRED)
find_package(gz-msgs11 REQUIRED)
```


Next, create a directory for your custom message definitions:

```sh
mkdir -p proto/gz/custom_msgs
touch proto/gz/custom_msgs/foo.proto
touch proto/gz/custom_msgs/bar.proto
touch proto/gz/custom_msgs/baz.proto
```

Give the message files some content:

`proto/gz/custom_msgs/foo.proto`:

```proto
syntax = "proto3";
package gz.custom_msgs;

message Foo
{
  double value = 1;
}
```

`proto/gz/custom_msgs/bar.proto`:

```proto
syntax = "proto3";
package gz.custom_msgs;

message Bar
{
  double value = 1;
}
```

`proto/gz/custom_msgs/baz.proto`:

```proto
syntax = "proto3";
package gz.custom_msgs;

import "gz/msgs/header.proto";

import "gz/custom_msgs/foo.proto";
import "gz/custom_msgs/bar.proto";

message Baz
{
  gz.custom_msgs.Foo foo = 1;
  gz.custom_msgs.Bar bar = 2;
}

message BazStamped
{
  gz.msgs.Header header = 1;
  gz.custom_msgs.Baz baz = 2;
}
```


Then, back in the `CMakeLists.txt` file, add following lines to generate the message library:

```cmake
# Define a variable 'MSGS_PROTOS' listing the .proto files
set(MSGS_PROTOS
     ${CMAKE_CURRENT_SOURCE_DIR}/proto/gz/custom_msgs/foo.proto
     ${CMAKE_CURRENT_SOURCE_DIR}/proto/gz/custom_msgs/bar.proto
     ${CMAKE_CURRENT_SOURCE_DIR}/proto/gz/custom_msgs/baz.proto
)

# Call 'gz_msgs_generate_messages()' to process the .proto files
gz_msgs_generate_messages(
  # The cmake target to be generated for libraries/executables to link
  TARGET msgs
  # The protobuf package to generate (Typically based on the path)
  PROTO_PACKAGE "gz.custom_msgs"
  # The path to the base directory of the proto files
  # All import paths should be relative to this (eg gz/custom_msgs/vector3d.proto)
  MSGS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/proto
  # List of proto files to process
  MSGS_PROTOS ${MSGS_PROTOS}
  # Dependency on gz-msgs
  DEPENDENCIES gz-msgs11::gz-msgs11
)
```

In order to reduce the amount of edits needed upon a version change of `gz-msgs`, it is common to:

 - Define a variable `GZ_MSGS_VER`, holding the version number:
   ```cmake
   find_package(gz-msgs11 REQUIRED)
   set(GZ_MSGS_VER ${gz-msgs11_VERSION_MAJOR})
   ```
 - And change the dependency line in above code block to:
   ```cmake
   DEPENDENCIES gz-msgs${GZ_MSGS_VER}::gz-msgs${GZ_MSGS_VER}
   ```

### Using Custom messages

There are two primary ways that Gazebo messages are used as part of an application.
Messages are either known at compile time and linked into the application, or
they are loaded dynamically at runtime.

#### Compile time message use

When messages are known at compile-time, they can be directly used.
This allow for the messages to be used without any sort of reflection/introspection.

Include the headers and interact with the messages using the generated
bindings according to the [language guide](https://protobuf.dev/reference/cpp/cpp-generated/)

```cpp
#include <gz/custom_msgs/baz.pb.h>

int main(int argc, char** argv)
{

  gz::custom_msgs::BazStamped msg;

  // msg has header and baz field
  auto *header = msg.mutable_header();
  auto *baz = msg.mutable_baz();
}
```

The corresponding CMake for this:

```
add_executable(${PROJECT_NAME} main.cc)
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}-msgs)
```

#### Run time message use

Alternatively, there may be cases where all message definitions are not known
at the time that the executable/library of interest is being built.
A common case for this is with command line tools, where only a limited
set of messages are available at the time the tool is built, and more
user-defined messages are generated later.

For example, when custom messages are generated, they aren't initially visible
to the `gz msgs` or `gz topic` command line tools:

```sh
$ gz msg -l | grep "custom_msgs" | wc -l
0

$ gz topic -t /foo -m gz.custom_msgs.Foo -p 'value: 1.0'
Unable to create message of type[gz.custom_msgs.Foo] with data[value: 1.0].
```

To use the new messages, point the `GZ_DESCRIPTOR_PATH` environment variable to
the location of the `build` folder or where you have chosen to install the
`.gz_desc` file:

```sh
$ cd build/

$ export GZ_DESCRIPTOR_PATH=`pwd`

$ gz msg -l | grep "custom_msgs" | wc -l
4

$ gz msg --info gz.custom_msgs.Foo
Name: gz.custom_msgs.Foo
File: gz/custom_msgs/foo.proto

message Foo {
  double value = 1 [json_name = "value"];
}

$ gz topic -t /foo -m gz.custom_msgs.Foo -p 'value: 1.0'
```
