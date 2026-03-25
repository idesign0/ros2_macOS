\page topicstatistics Topic Statistics

Previous Tutorial: \ref development

## Overview

Topic statistics provides a mechanism to quantify transport performance
between a publisher and subscriber. When enabled, a subscriber on a topic
can measure:

1. Number of dropped messages,
2. Publication statistics,
3. Reception statistics, and
4. Message age statistics.

Dropped messages are detected by the subscriber when an incoming message has
an ID that is larger than 1 plus the previous message ID. A message can be
dropped for a number of reasons. For example, a message can be dropped when
the publication and/or subscription buffers overflow.

Publication statistics capture information about when messages are sent by
a publisher. The average Hz rate, minimum and maximum times between
publications, and standard deviation of publication times are available.

Similarly, reception statistics capture information about when messages are
received by a subscriber. The average Hz rate, minimum and maximum times between receptions, and standard deviation of reception times are available.

Finally, message age statistics capture information between publication and
reception. The age of a message is the time between publication and
reception. We are ignoring clock discrepancies. The average, minimum, maximum, and standard deviation values of message age are available.

## Usage

The `GZ_TRANSPORT_TOPIC_STATISTICS` environment variable must be set to `1`
for both publishers and subscribers. Setting `GZ_TRANSPORT_TOPIC_STATISTICS` to `1` will change the wire protocol, which will prevent communication with nodes that have not set `GZ_TRANSPORT_TOPIC_STATISTICS` to `1`.

Additionally, a node on the subscriber side of a pub/sub relationship must
call `EnableStats`. For example:

```c++
if (!node.EnableStats(topic, true))
{
  std::cout << "Unable to enable stats\n";
}
```

A complete example can be found in the [subscriber_stats example program](https://github.com/gazebosim/gz-transport/blob/gz-transport14/example/subscriber_stats.cc).

With both `GZ_TRANSPORT_TOPIC_STATISTICS` set to `1` and a node
enabling topic statistics, then you will be able to echo statistic
information from the command line using `gz topic -et /statistics`.

It is possible to change the statistics output topic from `/statistics` to
one of your choosing by specifying a topic name when enabling topic
statistics. For example:

```c++
if (!node.EnableStats(topic, true, "/my_stats"))
{
  std::cout << "Unable to enable stats\n";
}
```

You can also change the statistics publication rate from 1Hz by specifying
the new Hz rate after the statistic's topic name:

```c++
if (!node.EnableStats(topic, true, "/my_stats", 100))
{
  std::cout << "Unable to enable stats\n";
}
```

### Example

For running the example, build the binaries in the example directory:

```
git clone https://github.com/gazebosim/gz-transport -b gz-transport14
cd gz-transport/example
cmake -S . -B build
# For UNIX
cmake --build build --parallel
# For Windows
cmake --build build --config release --parallel
```

#### Executing
> **NOTE**
> It is essential to have a valid value of `GZ_PARTITION` environment variable
> and to have it set to the same value in all open terminals. As `GZ_PARTITION`
> is based on hostname and username, especially Windows and Mac users might
> have problems due to spaces in their username, which are not a valid character
> in `GZ_PARTITION`. gz-transport prints error `Invalid partition name` in such
> case. To resolve that, set `GZ_PARTITION` explicitly to a valid value.

If you have the Gazebo Transport sources with the example programs built,
then you can test topic statistics by following these steps depending on the
platform:

##### Linux or Mac

1. Terminal 1: `GZ_PARTITION=test GZ_TRANSPORT_TOPIC_STATISTICS=1 ./example/build/publisher`
1. Terminal 2: `GZ_PARTITION=test GZ_TRANSPORT_TOPIC_STATISTICS=1 ./example/build/subscriber_stats`
1. Terminal 3: `GZ_PARTITION=test GZ_TRANSPORT_TOPIC_STATISTICS=1 gz topic -et /statistics`

##### Windows

1. Terminal 1: `set "GZ_PARTITION=test" && set "GZ_TRANSPORT_TOPIC_STATISTICS=1" && example\build\release\publisher.exe`
1. Terminal 2: `set "GZ_PARTITION=test" && set "GZ_TRANSPORT_TOPIC_STATISTICS=1" && example\build\subscriber_stats.exe`
1. Terminal 3: `set "GZ_PARTITION=test" && set "GZ_TRANSPORT_TOPIC_STATISTICS=1" && gz topic -et /statistics`
