# Creating custom gz-msgs entries

The example in this folder demonstrates how to create your own message definitions. 

Most of the setup is the same as the `using_gz_msgs` example, so if you are not familiar, view that first.

## Creating message definition

It is best to create a `proto/` folder in your project to hold your message definitions.
Inside of the `proto/` folder, you should construct a file tree that matches your package name.

For example, for the package `gz.msgs`, we would create `mkdir -p ./gz/msgs`
For example, for the package `gz.custom_msgs`, we would create `mkdir -p ./gz/custom_msgs`
For example, for the package `foo.bar.baz.bing`, we would create `mkdir -p ./foo/bar/baz/bing`

Inside the last folder is where you will create your message definitions in the `proto` format.
For more information on the format, consult the [language guide](https://protobuf.dev/programming-guides/proto3/)

Inside of your `.proto` file, you should also specify the package via the package keyword:

```
package gz.msgs;

package gz.custom_msgs;

package foo.bar.baz.bing;
```

Message definitions may also depend on already-specified `gz-msgs` types:

```
import "gz/msgs/header.proto"

message Foo {
  gz.msgs.Header header = 1;
}
```

## Generating files for custom messages

Once the message files are specified, they can be generated via the same CMake command as before:

```
# Example of custom messages that depend on gz.msgs
set(MSGS_PROTOS
   ${CMAKE_CURRENT_SOURCE_DIR}/proto/gz/custom_msgs/vector3d.proto)
gz_msgs_generate_messages(
  # The cmake target to be generated for libraries/executables to link
  TARGET custom_msgs_gen
  # The protobuf package to generate (Typically based on the path)
  PROTO_PACKAGE "gz.custom_msgs"
  # The path to the base directory of the proto files
  # All import paths should be relative to this (eg gz/custom_msgs/vector3d.proto)
  MSGS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/proto
  # List of proto files to generate
  MSGS_PROTOS ${MSGS_PROTOS}
  # List of message targets this library imports from
  DEPENDENCIES gz_msgs_gen)
```
