# classic_bags
Simplified ROS 2 interface in the style of ROS 1 for reading and writing bag files

## Reading Python
### ROS 1 Code

```python
import rosbag
bag = rosbag.Bag('test.bag')
for topic, msg, t in bag.read_messages(topics=['chatter', 'numbers']):
    print(msg)
bag.close()
```

### ROS 2 Code
```python
import classic_bags
bag = classic_bags.Bag('test.bag')
for topic, msg, t in bag.read_messages(topics=['chatter', 'numbers']):
    print(msg)
# bag.close() Closing bag is not necessary
```

## Writing Python
### ROS 1 Code

```python
import rosbag
from std_msgs.msg import Int32, String

bag = rosbag.Bag('test.bag', 'w')

try:
    s = String()
    s.data = 'foo'

    i = Int32()
    i.data = 42

    bag.write('chatter', s)
    bag.write('numbers', i)
finally:
    bag.close()
```

### ROS 2 Code

```python
import classic_bags
from std_msgs.msg import Int32, String

bag = classic_bags.Bag('test.bag', 'w')

try:
    s = String()
    s.data = 'foo'

    i = Int32()
    i.data = 42

    bag.write('chatter', s)
    bag.write('numbers', i)
finally:
    bag.close()  # Closing is not strictly necessary
```

### Alternate ROS 2 Code

```python
import classic_bags
from std_msgs.msg import String
import rclpy.time
import builtin_interfaces.msg


with classic_bags.Bag('test.bag', 'w') as bag:
    s = String()
    s.data = 'foo'

    # The third argument of the write method is the timestamp

    # Floats are interpreted as seconds since epoch
    bag.write('chatter', s, 456606000.5)

    # Ints are interpreted as nanoseconds since epoch
    bag.write('chatter', s, 456606000600000000)

    # You can also use rclpy.time.Time
    pytime = rclpy.time.Time(seconds=456606000, nanoseconds=700000000)
    bag.write('chatter', s, pytime)

    # Or a timestamp from a message
    msgtime = builtin_interfaces.msg.Time()
    msgtime.sec = 456606000
    msgtime.nanosec = 800000000
    bag.write('chatter', s, msgtime)

    # If none is specified, the system time will be used
    bag.write('chatter', s)
```

## C++ Reading
### ROS 1 Code
```c++
#include <rosbag/bag.h>
#include <rosbag/view.h>
#include <std_msgs/Int32.h>
#include <std_msgs/String.h>

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

rosbag::Bag bag;
bag.open("test.bag", rosbag::bagmode::Read);

std::vector<std::string> topics;
topics.push_back(std::string("chatter"));
topics.push_back(std::string("numbers"));

rosbag::View view(bag, rosbag::TopicQuery(topics));

foreach(rosbag::MessageInstance const m, view)
{
    std_msgs::String::ConstPtr s = m.instantiate<std_msgs::String>();
    if (s != NULL)
        std::cout << s->data << std::endl;

    std_msgs::Int32::ConstPtr i = m.instantiate<std_msgs::Int32>();
    if (i != NULL)
        std::cout << i->data << std::endl;
}

bag.close();
```

### ROS 2 Code
```c++
#include <classic_bags/bag.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/int32.hpp>

classic_bags::Bag bag
bag.open("test.bag", classic_bags::Read);

// Note: Topic filtering and iterator support not available yet
while (bag.hasNext())
{
    auto m = bag.next();
    std_msgs::msg::String::SharedPtr s = m->instantiate<std_msgs::msg::String>();
    if (s != NULL)
        std::cout << s->data << std::endl;

    std_msgs::msg::Int32::SharedPtr i = m->instantiate<std_msgs::msg::Int32>();
    if (i != NULL)
        std::cout << i->data << std::endl;
}

bag.close();
```

## C++ Writing
### ROS 1 Code
```c++
#include <rosbag/bag.h>
#include <std_msgs/Int32.h>
#include <std_msgs/String.h>

rosbag::Bag bag;
bag.open("test.bag", rosbag::bagmode::Write);

std_msgs::String str;
str.data = std::string("foo");

std_msgs::Int32 i;
i.data = 42;

bag.write("chatter", ros::Time::now(), str);
bag.write("numbers", ros::Time::now(), i);

bag.close();
```

### ROS 2 Code
```c++
#include <classic_bags/bag.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/int32.hpp>

classic_bags::Bag bag;
bag.open("test.bag", classic_bags::Write);

std_msgs::msg::String str;
str.data = std::string("foo");

std_msgs::msg::Int32 i;
i.data = 42;

// If not specified, timestamp is assumed to be system time
bag.write("chatter", str);

// Otherwise, timestamp must be rclcpp::Time
bag.write("numbers", rclcpp::Time(456606000600000000), i);

bag.close();
```
