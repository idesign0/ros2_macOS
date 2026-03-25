\page messages Messages

Next Tutorial: \ref services
Previous Tutorial: \ref nodestopics

## Overview

In this tutorial, we are going to create two nodes that are going to communicate
via messages. One node will be a publisher that generates the information,
whereas the other node will be the subscriber consuming the information. Our
nodes will be running on different processes within the same machine.

```{.sh}
mkdir ~/gz_transport_tutorial
cd ~/gz_transport_tutorial
```

## Publisher

Download the [publisher.cc](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/publisher.cc) file within the `gz_transport_tutorial`
folder and open it with your favorite editor:

\snippet example/publisher.cc complete

### Walkthrough

```{.cpp}
#include <gz/msgs.hh>
#include <gz/transport.hh>
```

The line `#include <gz/transport.hh>` contains all the Gazebo
Transport headers for using the transport library.

The next line includes the generated protobuf code that we are going to use
for our messages. We are going to publish `StringMsg` type protobuf messages.

```{.cpp}
// Create a transport node and advertise a topic.
gz::transport::Node node;
std::string topic = "/foo";

auto pub = node.Advertise<gz::msgs::StringMsg>(topic);
if (!pub)
{
  std::cerr << "Error advertising topic [" << topic << "]" << std::endl;
  return -1;
}
```

First of all we declare a *Node* that will offer some of the transport
functionality. In our case, we are interested in publishing topic updates, so
the first step is to announce our topic name and its type. Once a topic name is
advertised, we can start publishing periodic messages using the publisher
object.

```{.cpp}
// Prepare the message.
gz::msgs::StringMsg msg;
msg.set_data("HELLO");

// Publish messages at 1Hz.
while (!g_terminatePub)
{
  if (!pub.Publish(msg))
    break;

  std::cout << "Publishing hello on topic [" << topic << "]" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
```

In this section of the code we create a protobuf message and fill it with
content. Next, we iterate in a loop that publishes one message every second.
The method *Publish()* sends a message to all the subscribers.

## Subscriber

Download the [subscriber.cc](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/subscriber.cc)
file into the `gz_transport_tutorial` folder and open it with your favorite editor:

\snippet example/subscriber.cc complete

### Walkthrough

```{.cpp}
//////////////////////////////////////////////////
/// brief Function called each time a topic update is received.
void cb(const gz::msgs::StringMsg &_msg)
{
  std::cout << "Msg: " << _msg.data() << std::endl << std::endl;
}
```

We need to register a function callback that will execute every time we receive
a new topic update. The signature of the callback is always similar to the one
shown in this example with the only exception of the protobuf message type.
You should create a function callback with the appropriate protobuf type
depending on the type of the topic advertised. In our case, we know that topic
`/foo` will contain a Protobuf `StringMsg` type.

```{.cpp}
gz::transport::Node node;
std::string topic = "/foo";

// Subscribe to a topic by registering a callback.
if (!node.Subscribe(topic, cb))
{
  std::cerr << "Error subscribing to topic [" << topic << "]" << std::endl;
  return -1;
}
```

After the node creation, the method `Subscribe()` allows you to subscribe to a
given topic name by specifying your subscription callback function.

```{.cpp}
// Zzzzzz.
gz::transport::waitForShutdown();
```

If you don't have any other tasks to do besides waiting for incoming messages,
you can use the call `waitForShutdown()` that will block your current thread
until you hit *CTRL-C*. Note that this function captures the *SIGINT* and
*SIGTERM* signals.

## Building the code

Download the [CMakeLists.txt](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/CMakeLists.txt) file within the `gz_transport_tutorial` folder.

Once you have all your files, go ahead and create a `build/` directory within
the `gz_transport_tutorial` directory.

```{.sh}
mkdir build
cd build
```
Run `cmake` and build the code.

```{.sh}
# Linux and MacOS
cmake ..
make publisher subscriber

# Windows
cmake --build . --config Release
```

## Running the examples

> **NOTE**
> It is essential to have a valid value of `GZ_PARTITION` environment variable
> and to have it set to the same value in all open terminals. As `GZ_PARTITION`
> is based on hostname and username, especially Windows and Mac users might
> have problems due to spaces in their username, which are not a valid character
> in `GZ_PARTITION`. gz-transport prints error `Invalid partition name` in such
> case. To resolve that, set `GZ_PARTITION` explicitly to a valid value:
> ```bash
> # Linux and Mac
> export GZ_PARTITION=test
> # Windows
> set GZ_PARTITION=test
> ```

Open two new terminals and from your `build/` directory run the executables.

From terminal 1:

```{.sh}
# Linux and MacOS
./publisher

# Windows
.\Release\publisher.exe
```

From terminal 2:

```{.sh}
# Linux and MacOS
./subscriber

# Windows
.\Release\subscriber.exe
```

In your subscriber terminal, you should expect an output similar to this one,
showing that your subscriber is receiving the topic updates:

```{.sh}
$ ./subscriber
Msg: HELLO

Msg: HELLO

Msg: HELLO

Msg: HELLO

Msg: HELLO
```

## Advertise Options

We can specify some options before we publish the messages. One such option is
to specify the number of messages published per topic per second. It is optional
to use but it can be handy in situations like when we want to control the rate
of messages published per topic.

We can declare the throttling option using the following code :

```{.cpp}
  // Create a transport node and advertise a topic with throttling enabled.
  gz::transport::Node node;
  std::string topic = "/foo";

  // Setting the throttling option
  gz::transport::AdvertiseMessageOptions opts;
  opts.SetMsgsPerSec(1u);

  auto pub = node.Advertise<gz::msgs::StringMsg>(topic, opts);
  if (!pub)
  {
    std::cerr << "Error advertising topic [" << topic << "]" << std::endl;
    return -1;
  }
```

### Walkthrough

```{.cpp}
  gz::transport::AdvertiseMessageOptions opts;
  opts.SetMsgsPerSec(1u);
```

In this section of code, we declare an *AdvertiseMessageOptions* object and use it
to pass message rate as an argument to *SetMsgsPerSec()* method. In our case, the object
name is opts and the message rate specified is 1 msg/sec.

```{.cpp}
  auto pub = node.Advertise<gz::msgs::StringMsg>(topic, opts);
```

Next, we advertise the topic with message throttling enabled. To do it, we pass opts
as an argument to the *Advertise()* method.


## Subscribe Options

A similar option is also available for the Subscriber node which enables it
to control the rate of incoming messages from a specific topic. While subscribing
to a topic, we can use this option to control the number of messages received per
second from that particular topic.

We can declare the throttling option using the following code :

```{.cpp}
  // Create a transport node and subscribe to a topic with throttling enabled.
  gz::transport::Node node;
  std::string topic = "/foo";
  gz::transport::SubscribeOptions opts;
  opts.SetMsgsPerSec(1u);
  node.Subscribe(topic, cb, opts);
```

### Walkthrough

```{.cpp}
  gz::transport::SubscribeOptions opts;
  opts.SetMsgsPerSec(1u);
  node.Subscribe(topic, cb, opts);
```

In this section of code, we declare a *SubscribeOptions* object and use it
to pass message rate as an argument to the *SetMsgsPerSec()* method. In our case, the object
name is opts and the message rate specified is 1 msg/sec. Then, we subscribe to the topic
using the *Subscribe()* method with opts passed as an argument to it.

##Generic subscribers

As you have seen in the examples so far, the callbacks used by the
subscribers contain a specific protobuf parameter, such as
`gz::msgs::StringMsg`. As the name of this section suggests, it is also
possible to create a generic subscriber callback that can receive messages of
different types. This use case might be interesting if you are building a bridge
between Gazebo Transport and another protocol or if you want to just print the
content of a generic protobuf message using
`google::protobuf::TextFormat::PrintToString()`, among other use cases.

Download the [subscriber_generic.cc](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/subscriber_generic.cc) file within the `gz_transport_tutorial` folder and open it with your favorite editor:

```{.cpp}
#include <google/protobuf/message.h>
#include <iostream>
#include <string>
#include <gz/transport.hh>

//////////////////////////////////////////////////
/// brief Function called each time a topic update is received.
/// Note that this callback uses the generic signature, hence it may receive
/// messages with different types.
void cb(const google::protobuf::Message &_msg,
        const gz::transport::MessageInfo &_info)
{
  std::cout << "Topic: [" << _info.Topic() << "]" << std::endl;

  if (std::string str; google::protobuf::TextFormat::PrintToString(_msg, &str))
    std::cout << str << std::endl;
  else
    std::cerr << "Error printing message" << std::endl;
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  gz::transport::Node node;
  std::string topic = "/foo";
  // Subscribe to a topic by registering a callback.
  if (!node.Subscribe(topic, cb))
  {
    std::cerr << "Error subscribing to topic [" << topic << "]" << std::endl;
    return -1;
  }

  // Zzzzzz.
  gz::transport::waitForShutdown();

  return 0;
}
```

### Walkthrough

```{.cpp}
//////////////////////////////////////////////////
/// brief Function called each time a topic update is received.
/// Note that this callback uses the generic signature, hence it may receive
/// messages with different types.
void cb(const google::protobuf::Message &_msg,
        const gz::transport::MessageInfo &_info)
{
  std::cout << "Topic: [" << _info.Topic() << "]" << std::endl;

  if (std::string str; google::protobuf::TextFormat::PrintToString(_msg, &str))
    std::cout << str << std::endl;
  else
    std::cerr << "Error printing message" << std::endl;
}
```

Here, we use the generic callback function signature. Note the use of
`google::protobuf::Message` as the message type in the subscription callback
function ``cb()``. It enables us to receive topic updates with different message
types, such as `Int32` or `String` from the subscribed topic.
Furthermore, we don't need to worry about the type of the topic advertised while
specifying the callback function. The parameter
`gz::transport::MessageInfo &_info` provides some information about the
message received (e.g.: the topic name).

```{.cpp}
//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  gz::transport::Node node;
  std::string topic = "/foo";
  // Subscribe to a topic by registering a callback.
  if (!node.Subscribe(topic, cb))
  {
    std::cerr << "Error subscribing to topic [" << topic << "]" << std::endl;
    return -1;
  }

  // Zzzzzz.
  gz::transport::waitForShutdown();
  return 0;
}
```

Similar to the previous examples, we use the `Subscribe()` function to
subscribe to a given topic name by specifying the callback function. In our
example, the topic name subscribed is `/foo`.

Follow the next instructions to compile and run the generic subscriber example:

Run `cmake` and build the example:

```{.sh}
cd build

# Linux and MacOS
cmake ..
make subscriber_generic

# Windows
cmake --build . --config Release
```

From terminal 1:

```{.sh}
# Linux and MacOS
./publisher

# Windows
.\Release\publisher.exe
```

From terminal 2:

```{.sh}
# Linux and MacOS
./subscriber_generic

# Windows
.\Release\subscriber_generic.exe
```

## Using custom Protobuf messages

We use Gazebo Msgs in most of our examples and tests. This decision was
made just for convenience but Gazebo Transport supports the use of Protobuf
messages directly. The most common problem with custom Protobuf messages is
often the integration of the message generation into the build system of your
project. Next, you can find an example of a publisher and subscriber using a
custom Protobuf message integrated with CMake.

Download the [publisher_custom_msg.cc](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/publisher_custom_msg.cc)
and the [subscriber_custom_msg.cc](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/subscriber_custom_msg.cc)
files within the `gz_transport_tutorial`. Then, create a `msgs` folder and
download the [stringmsg.proto](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/msgs/stringmsg.proto)
and the [CMakeLists.txt](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/msgs/CMakeLists.txt)
files within the `msgs` folder. Finally, we'll need the main [CMakeLists.txt](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/CMakeLists.txt)
file. You should have this file from the previous examples. Otherwise,
download and place it within the `gz_transport_tutorial` folder.

### Walkthrough

There's nothing new to show in the `publisher_custom_msg.cc` or
`subscriber_custom_msg.cc` besides the use of your custom Protobuf message
instead of Gazebo Msgs. The only relevant parts are in the `CMakeLists.txt`
files.

```{.cpp}
# Message generation. Only required when using custom Protobuf messages.
find_package(Protobuf REQUIRED)
add_subdirectory(msgs)
set_source_files_properties(${PROTO_SRC} ${PROTO_HEADER}
                           PROPERTIES GENERATED TRUE)
include_directories(${CMAKE_BINARY_DIR})
```

This is how we find the Protobuf CMake config file, to make sure that the
macro for Protobuf message generation is available. We also let CMake know that
there is a new subdirectory to inspect containing our custom messages.

```{.cpp}
if (EXISTS "${CMAKE_SOURCE_DIR}/publisher_custom_msg.cc")
  add_executable(publisher_custom_msg publisher_custom_msg.cc)
  target_link_libraries(publisher_custom_msg
     ${GZ-TRANSPORT_LIBRARIES}
     ${PROTO_SRC}
  )
  add_dependencies(publisher_custom_msg protobuf_compilation)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/subscriber_custom_msg.cc")
  add_executable(subscriber_custom_msg subscriber_custom_msg.cc)
  target_link_libraries(subscriber_custom_msg
    ${GZ-TRANSPORT_LIBRARIES}
    ${PROTO_SRC}
  )
  add_dependencies(subscriber_custom_msg protobuf_compilation)
endif()
```

In the previous snippet we can see how the binaries are generated. The relevant
part is to add a new dependency. We're telling CMake that these two
binaries depend on the `protobuf_compilation` target. This will trigger the
recompilation of the binaries if there is any change in our Protobuf messages.
Also, we have to link our binaries with the generated Protobuf messages. The
list of generated messages are contained in the `${PROTO_SRC}` variable.
Finally, this is the content of the `msgs/CMakeLists.txt` file:

```{.cpp}
PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER
  stringmsg.proto
)

# Variables needed to propagate through modules
# If more than one layer of cmake use CACHE INTERNAL ...
set(PROTOBUF_INCLUDE_DIRS ${PROTOBUF_INCLUDE_DIRS} PARENT_SCOPE)
set(PROTOBUF_LIBRARIES ${PROTOBUF_LIBRARIES} PARENT_SCOPE)
set(PROTO_SRC ${PROTO_SRC} PARENT_SCOPE)
set(PROTO_HEADER ${PROTO_HEADER} PARENT_SCOPE)

add_custom_target(protobuf_compilation DEPENDS ${PROTO_SRC})
```

The macro `PROTOBUF_GENERATE_CPP` will use `protoc` to generate the `.pb.h`
and `.pb.cc` files from your `.proto` files. Follow the next instructions to
compile and run the generic subscriber example:

Run `cmake` and build the example:

```{.sh}
cd build

# Linux and MacOS
cmake ..
make

# Windows
cmake --build . --config Release
```

From terminal 1:

```{.sh}
# Linux and MacOS
./publisher_custom_msg

# Windows
.\Release\publisher_custom_msg.exe
```

From terminal 2:

```{.sh}
# Linux and MacOS
./subscriber_custom_msg

# Windows
.\Release\subscriber_custom_msg.exe
```

## Topic remapping

It's possible to set some global node options that will affect both publishers
and subscribers. One of these options is topic remapping. A topic remap
consists of a pair of topic names. The first name is the original topic name to
be replaced. The second name is the new topic name to use instead. As an example,
imagine that you recorded a collection of messages published over topic `/foo`.
Maybe in the future, you want to play back the log file but remapping the topic
`/foo` to `/bar`. This way, all messages will be published over the `/bar`
topic without having to modify the publisher and create a new log.

We can declare the topic remapping option using the following code:

```{.cpp}
  // Create a transport node and remap a topic.
  gz::transport::NodeOptions nodeOptions;
  nodeOptions.AddTopicRemap("/foo", "/bar");
  gz::transport::Node node(nodeOptions);
  std::string topic = "/foo";
```

You can modify any of the publisher examples to add this option.

From terminal 1:

```{.sh}
# Linux and MacOS
./publisher

# Windows
.\Release\publisher.exe
```

From terminal 2 (requires Gazebo Tools):

```{.sh}
gz topic --echo -t /bar
```

And you should receive all the messages coming in terminal 2.

The command `gz log playback` also supports the notion of topic remapping. Run
`gz log playback -h` in your terminal for further details (requires Gazebo Tools).
