\page nodestopics Nodes and Topics

Next Tutorial: \ref messages
Previous Tutorial: \ref installation

## Nodes

The communication in Gazebo Transport follows a pure distributed architecture.
There is no central process, broker or similar central structure. Instead, all
the nodes in the network can act as publishers, subscribers, provide services and
request services.

A publisher is a node that produces information and a subscriber is a node that
consumes information. There are two ways to communicate in Gazebo Transport.
First, we could use a publish/subscribe approach, where a
node advertises a topic, and then publishes periodic updates. On the other
side, one or more nodes subscribe to the same topic registering a function that
will be executed each time a new message is received. An alternative
communication paradigm is based on service calls (\ref services),
covered in another tutorial. A service call is a remote service that a node offers
to the rest of the nodes. A node can request a service in a similar way a local
function is executed.

## Topics

A topic is simply a name for grouping a specific set of messages or a particular
service. Imagine that you have a camera and want to periodically publish its
images. Your node could advertise a topic called `/image`, and then publish a
new message on this topic every time a new image is available. Other nodes will
subscribe to the same topic and will receive the messages containing the image.
A node could also offer an echo service in the topic `/echo`. Any node
interested in this service will request a service call on topic `/echo`. The
service call will accept arguments and will return a result. In our echo
service example (\ref services), the result will be similar
to the input parameter passed to the service.

There are some rules to follow when selecting a topic name. It should be any
alphanumeric name followed by zero or more slashes. For example: `/image`,
`head_position`, `/robot1/joints/HeadPitch` are examples of valid topic
names. The next table summarizes the allowed and not allowed topic rules.
The following symbols are not allowed as part of the topic name: `@`, `:=`, `~`.

| Topic name  | Validity | Comment                       |
|:----------: |:--------:|:-----------------------------:|
| /topicA     | Valid    |                               |
| /topicA/    | Valid    | Equivalent to /topicA         |
| topicA      | Valid    |                               |
| /a/b        | Valid    |                               |
|             | Invalid  | Empty string is invalid       |
| my topic    | Invalid  | Contains white space          |
| //image     | Invalid  | Contains two consecutive `//` |
| /           | Invalid  | `/` topic is not allowed      |
| ~myTopic    | Invalid  | Symbol `~` not allowed        |
| `@`myTopic  | Invalid  | Symbol `@` not allowed        |
| myTopic:=   | Invalid  | Symbol `:=` not allowed       |

## Topic scope

A topic can be optionally advertised with a scope. A scope allows you to set the
visibility of a topic. The available scopes are `Process`, `Host`, and
`All`. A `Process` scope means that the advertised topic will only be
visible to the nodes within the same process as the advertiser. A topic with a
`Host` scope restricts the visibility of a topic to nodes located on the same
machine as the advertiser. Finally, by specifying a scope with an `All` value,
you're allowing your topic to be visible by any node.

## Partition and namespaces

When you create your node you can specify some options to customize its
behavior. Among those options is setting a partition name and a namespace.

A partition is used to isolate a set of topics or services within a group of
nodes that share the same partition name, e.g.: Node1 advertises topic `/foo`
and Node2 advertises `/foo` too. If we don't use a partition, a node
subscribed to `/foo` will receive the messages published from Node1 and Node2.
Alternatively, we could specify `p1` as a partition for Node1 and `p2` as a
partition for Node2. When we create the node for our subscriber, if we specify
`p1` as a partition name, we'll receive the messages published only by Node1.
If we use `p2`, we'll only receive the messages published by Node2. If we
don't set a partition name, we won't receive any messages from Node1 or Node2.

A partition name is any alphanumeric string with a few exceptions.
The symbol `/` is allowed as part of a partition name but just `/` is
not allowed. The symbols `@`, `~`, `:=` or white spaces are not allowed as
part of a partition name. Two or more consecutive slashes (`//`) are not
allowed.

The default partition name is created using a combination of your hostname,
followed by `:` and your username, e.g.: `bb8:caguero` . It's also possible
to use the environment variable `GZ_PARTITION` for setting a custom partition
name.

A namespace is considered a prefix that might be potentially applied to some of
the topic/services advertised in a node.

E.g.: Node1 sets a namespace `ns1` and advertises the topics
`t1`, `t2` and `/t3`. `/t3` is considered an absolute topic (starts
with `/`) meaning it won't be affected by a namespace. However, `t1` and
`t2` will be advertised as `/ns1/t1` and `/ns1/t2`.

A namespace is any alphanumeric string with a few exceptions.
The symbol `/` is allowed as part of a namespace but just `/` is not
allowed. The symbols `@`, `~` or white spaces are not allowed as
part of a namespace. Two or more consecutive slashes (`//`) are not allowed.
If topic name or namespace is invalid then the fully qualified topic name is
invalid too.

|Namespace |Topic name | Fully qualified topic | Validity| Comment                                 |
|:-------: |:---------:| :--------------------:|:-------:|:---------------------------------------:|
|ns1       |/topicA    | /topicA               | Valid   |  Absolute topic                         |
|          |/topicA    | /topicA               | Valid   |  Absolute topic                         |
|ns1       |topicA     | /ns1/topicA           | Valid   |                                         |
|          |topicA     | /topicA               | Valid   |                                         |
|ns1       |topic A    |                       | Invalid |  Topic contains white space             |
|          |topic A    |                       | Invalid |   Topic contains white space            |
|my ns     |topicA     |                       | Invalid |  Namespace contains white space         |
|//ns      |topicA     |                       | Invalid |  Namespace contains two consecutive `//`|
|/         |topicA     |                       | Invalid |  `/` namespace is not allowed           |
|~myns     |topicA     |                       | Invalid |  Symbol `~` not allowed                 |
