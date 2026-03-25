\page python Python Support

Next Tutorial: \ref security
Previous Tutorial: \ref services

## Overview

In this tutorial, we are going to show the functionalities implemented in Python.
These features are brought up by creating bindings from the C++ implementation 
using pybind11. It is important to note that not all of C++ features are available
yet, on this tutorial we will go over the most relevant features. For more information,
refer to the [__init__.py](https://github.com/gazebosim/gz-transport/blob/gz-transport14/python/src/__init__.py)
file which is a wrapper for all the bindings.

For this tutorial, we will create two nodes that communicate via messages. One node
will be a publisher that generates the information, whereas the other node will be 
the subscriber consuming the information. Our nodes will be running on different 
processes within the same machine.

```{.sh}
mkdir ~/gz_transport_tutorial
cd ~/gz_transport_tutorial
```

## Prerequisites

Before you begin, make sure you have the following installed:

- python3-gz-transport14

To install the required package in Linux, run:

```bash
sudo apt install python3-gz-transport14
```

## Publisher

Download the [publisher.py](https://github.com/gazebosim/gz-transport/blob/gz-transport14/python/examples/publisher.py) file within the `gz_transport_tutorial`
folder and open it with your favorite editor:

\snippet python/examples/publisher.py complete

### Walkthrough

```{.py}
    from gz.msgs11.stringmsg_pb2 import StringMsg
    from gz.msgs11.vector3d_pb2 import Vector3d
    from gz.transport14 import Node
```

The library `gz.transport14` contains all the Gazebo Transport elements that can be
used in Python. The final API we will use is contained inside the class `Node`.

The lines `from gz.msgs11.stringmsg_pb2 import StringMsg` and `from gz.msgs11.vector3d_pb2 import Vector3d`
include the generated protobuf code that we are going to use for our messages.
We are going to publish two types of messages: `StringMsg` and `Vector3d` protobuf
messages.

```{.py}
    node = Node()
    stringmsg_topic = "/example_stringmsg_topic"
    vector3d_topic = "/example_vector3d_topic"
    pub_stringmsg = node.advertise(stringmsg_topic, StringMsg)
    pub_vector3d = node.advertise(vector3d_topic, Vector3d)
```

First of all we declare a *Node* that will offer some of the transport
functionality. In our case, we are interested in publishing topic updates, so
the first step is to announce our topics names and their types. Once a topic name
is advertised, we can start publishing periodic messages using the publishers objects.

```{.py}
    vector3d_msg = Vector3d()
    vector3d_msg.x = 10
    vector3d_msg.y = 15
    vector3d_msg.z = 20
    stringmsg_msg = StringMsg()
    stringmsg_msg.data = "Hello"

    try:
        count = 0
        while True:
          count += 1
          vector3d_msg.x = count
          if not (pub_stringmsg.publish(stringmsg_msg)):
              break
          print("Publishing 'Hello' on topic [{}]".format(stringmsg_topic))
          if not (pub_vector3d.publish(vector3d_msg)):
              break
          print("Publishing a Vector3d on topic [{}]".format(vector3d_topic))
          time.sleep(0.1)

    except KeyboardInterrupt:
        pass
```

In this section of the code we create the protobuf messages and fill them with
content. Next, we iterate in a loop that publishes one message every 100ms to 
each topic. The method *publish()* sends a message to all the subscribers.

## Subscriber

Download the [subscriber.py](https://github.com/gazebosim/gz-transport/blob/gz-transport14/python/examples/subscriber.py)
file into the `gz_transport_tutorial` folder and open it with your favorite editor:

\snippet python/examples/subscriber.py complete

### Walkthrough

```{.py}
    from gz.msgs11.stringmsg_pb2 import StringMsg
    from gz.msgs11.vector3d_pb2 import Vector3d
    from gz.transport14 import Node
```

Just as before, we are importing the `Node` class from the `gz.transport14` library
and the generated code for the `StringMsg` and `Vector3d` protobuf messages.

```{.py}
    def stringmsg_cb(msg: StringMsg):
        print("Received StringMsg: [{}]".format(msg.data))

    def vector3_cb(msg: Vector3d):
        print("Received Vector3: [x: {}, y: {}, z: {}]".format(msg.x, msg.y, msg.z))
```

We need to register a function callback that will execute every time we receive
a new topic update. The signature of the callback is always similar to the ones
shown in this example with the only exception of the protobuf message type.
You should create a function callback with the appropriate protobuf type
depending on the type of the topic advertised. In our case, we know that topic
`/example_stringmsg_topic` will contain a Protobuf `StringMsg` type and topic 
`/example_vector3d_topic` a `Vector3d` type.

```{.py}
    # create a transport node
    node = Node()
    topic_stringmsg = "/example_stringmsg_topic"
    topic_vector3d = "/example_vector3d_topic"
    # subscribe to a topic by registering a callback
    if node.subscribe(StringMsg, topic_stringmsg, stringmsg_cb):
        print("Subscribing to type {} on topic [{}]".format(
            StringMsg, topic_stringmsg))
    else:
        print("Error subscribing to topic [{}]".format(topic_stringmsg))
        return
    # subscribe to a topic by registering a callback
    if node.subscribe(Vector3d, topic_vector3d, vector3_cb):
        print("Subscribing to type {} on topic [{}]".format(
            Vector3d, topic_vector3d))
    else:
        print("Error subscribing to topic [{}]".format(topic_vector3d))
        return
```

After the node creation, the method `subscribe()` allows you to subscribe to a
given topic by specifying the message type, the topic name and a subscription 
callback function.

```{.py}
    # wait for shutdown
    try:
      while True:
        time.sleep(0.001)
    except KeyboardInterrupt:
      pass
```

If you don't have any other tasks to do besides waiting for incoming messages,
we create an infinite loop that checks for messages each 1ms that will block
your current thread until you hit *CTRL-C*.

## Updating PYTHONPATH

If you made the binary installation of Gazebo Transport, you can skip this step
and go directly to the next section. Otherwise, if you built the package from 
source it is needed to update the PYTHONPATH in order for Python to recognize 
the library by doing the following:

1. If you built from source using `colcon`:
```{.sh}
export PYTHONPATH=$PYTHONPATH:<path to ws>/install/lib/python
```
2. If you built from source using `cmake`:
```{.sh}
export PYTHONPATH=$PYTHONPATH:<path_install_prefix>/lib/python
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

Open two new terminals and directly run the Python scripts downloaded previously.

From terminal 1:

```{.sh}
python3 ./publisher.py
```

From terminal 2:

```{.sh}
python3 ./subscriber.py
```

In your publisher terminal, you should expect an output similar to this one,
showing when a message is being published:

```{.sh}
$ ./publisher.py
Publishing 'Hello' on topic [/example_stringmsg_topic]
Publishing a Vector3d on topic [/example_vector3d_topic]
Publishing 'Hello' on topic [/example_stringmsg_topic]
Publishing a Vector3d on topic [/example_vector3d_topic]
```
In your subscriber terminal, you should expect an output similar to this one,
showing that your subscriber is receiving the topic updates:

```{.sh}
$ ./subscriber.py
Received StringMsg: [Hello]
Received Vector3: [x: 2.0, y: 15.0, z: 20.0]
Received StringMsg: [Hello]
Received Vector3: [x: 3.0, y: 15.0, z: 20.0]
Received StringMsg: [Hello]
Received Vector3: [x: 4.0, y: 15.0, z: 20.0]
```
## Threading in Gazebo Transport
The way Gazebo Transport is implemented, it creates several threads each time
a node, publisher, subscriber, etc is created. While this allows us to have a
better parallelization in processes, a downside is possible race conditions that 
might occur if the ownership/access of variables is shared across multiple 
threads. Even though Python has its [GIL](https://wiki.python.org/moin/GlobalInterpreterLock),
all the available features are bindings created for its C++ implementation, in
other words, the downsides mentioned before are still an issue to be careful about. We
recommend to always use threading locks when working with object that are used
in several places (publisher and subscribers). 

We developed a couple of examples that demonstrate this particular issue. Take
a look at a publisher and subscriber (within the same node) that have race
conditions triggered in the [data_race_without_mutex.py](https://github.com/gazebosim/gz-transport/blob/gz-transport14/python/examples/data_race_without_mutex.py) file. The proposed solution to this issue is to use the `threading` library, you can see the same example with a mutex
in the [data_race_with_mutex.py](https://github.com/gazebosim/gz-transport/blob/gz-transport14/python/examples/data_race_with_mutex.py) file.

You can run any of those examples by just doing the following in a terminal:
```{.sh}
python3 ./data_race_without_mutex.py
```

or

```{.sh}
python3 ./data_race_with_mutex.py
```

## Advertise Options

We can specify some options before we publish the messages. One such option is
to specify the number of messages published per topic per second. It is optional
to use but it can be handy in situations where we want to control the rate
of messages published per topic.

We can declare the throttling option using the following code :

```{.py}
    from gz.msgs11.stringmsg_pb2 import StringMsg
    from gz.transport14 import Node, AdvertiseMessageOptions

    # Create a transport node and advertise a topic with throttling enabled.
    node = Node()
    topic_name = "/foo"

    # Setting the throttling option
    opts = AdvertiseMessageOptions()
    opts.msgs_per_sec = 1
    pub = node.advertise(topic_name, StringMsg, opts)
    if not pub:
      print("Error advertising topic" + topic_name)
```

### Walkthrough

```{.py}
    # Setting the throttling option
    opts = AdvertiseMessageOptions()
    opts.msgs_per_sec = 1
```

In this section of code, we declare an *AdvertiseMessageOptions* object and use
it to set the publishing rate (the member `msgs_per_sec`), In our case, the rate
specified is 1 msg/sec.

```{.py}
    pub = node.advertise(topic_name, StringMsg, opts)
```

Next, we advertise the topic with message throttling enabled. To do it, we pass opts
as an argument to the *advertise()* method.

## Subscribe Options

A similar option is also available for the Subscriber node which enables it
to control the rate of incoming messages from a specific topic. While subscribing
to a topic, we can use this option to control the number of messages received per
second from that particular topic.

We can declare the throttling option using the following code :

```{.py}
    from gz.msgs11.stringmsg_pb2 import StringMsg
    from gz.transport14 import Node, SubscribeOptions

    def stringmsg_cb(msg: StringMsg):
        print("Received StringMsg: [{}]".format(msg.data))

    # Create a transport node and subscribe to a topic with throttling enabled.
    node = Node()
    topic_name = "/foo"
    opts = SubscribeOptions()
    opts.msgs_per_sec = 1
    node.subscribe(StringMsg, topic_name, stringmsg_cb, opts)
```

### Walkthrough

```{.py}
    opts = SubscribeOptions()
    opts.msgs_per_sec = 1
    node.subscribe(StringMsg, topic_name, stringmsg_cb, opts)
```

In this section of code, we create a *SubscribeOptions* object and use it to set
message rate (the member `msgs_per_sec`). In our case, the message rate specified
is 1 msg/sec. Then, we subscribe to the topic
using the *subscribe()* method with opts passed as an argument to it.

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

```{.py}
    from gz.transport14 import Node, NodeOptions

    # Create a transport node and remap a topic.
    nodeOpts = NodeOptions()
    nodeOpts.add_topic_remap("/example_stringmsg_topic", "/bar")
    node = Node(nodeOpts)
```

You can modify the publisher example to add this option.

From terminal 1:

```{.sh}
python3 ./publisher.py

```

From terminal 2 (requires Gazebo Tools):

```{.sh}
gz topic --echo -t /bar
```

And you should receive all the messages coming in terminal 2.

The command `gz log playback` also supports the notion of topic remapping. Run
`gz log playback -h` in your terminal for further details (requires Gazebo Tools).

## Service Requester

Download the [requester.py](https://github.com/gazebosim/gz-transport/blob/gz-transport14/python/examples/requester.py)
file into the `gz_transport_tutorial` folder and open it with your favorite editor:

\snippet python/examples/requester.py complete

### Walkthrough

```{.py}
    from gz.msgs11.stringmsg_pb2 import StringMsg
    from gz.transport14 import Node
```

Just as before, we are importing the `Node` class from the `gz.transport14`
library and the generated code for the `StringMsg` protobuf message.

```{.py}
    node = Node()
    service_name = "/echo"
    request = StringMsg()
    request.data = "Hello world"
    response = StringMsg()
    timeout = 5000
```

On these lines we are creating our *Node* object which will be used to create
the service request and defining all the relevant variables in order to create
a request, the service name, the timeout of the request, the request and response
data types.

```{.py}
    result, response = node.request(service_name, request, StringMsg, StringMsg, timeout)
    print("Result:", result, "\nResponse:", response.data)
```

Here we are creating the service request to the `/echo` service, storing the
result and response of the request in some variables and printing them out.

## Service Responser

Unfortunately, this feature is not available on Python at the moment. However,
we can use a service responser created in C++ and make a request to it from a
code in Python. Taking that into account, we will use the [response.cc](https://github.com/gazebosim/gz-transport/blob/gz-transport14/example/responser.cc) file as the service responser.

## Running the examples

Open a new terminal and directly run the Python script downloaded previously.
It is expected that the service responser is running in another terminal for
this, you can refer to the previous tutorial \ref services.

From terminal 1:

```{.sh}
python3 ./requester.py
```

In your terminal, you should expect an output similar to this one,
showing the result and response from the request:

```{.sh}
$ ./requester.py
Result: True 
Response: Hello world
```
