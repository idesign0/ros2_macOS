# Extending the Type System

Sugarcoat uses a type system built on `SupportedType` to bridge ROS 2 message types with Python-native data. This document explains how the system works and how to extend it with custom types.

## SupportedType Base Class

Every supported message type is a subclass of `ros_sugar.io.supported_types.SupportedType`. The base class defines three extension points:

```python
class SupportedType:
    # The ROS 2 message class (e.g., std_msgs.msg.String)
    _ros_type: type

    # Callback class for deserializing incoming messages
    callback = callbacks.GenericCallback

    @classmethod
    def convert(cls, *output, **_) -> Any:
        """Convert Python data into a ROS message instance."""
        return output

    @classmethod
    def get_ros_type(cls) -> type:
        """Return the underlying ROS 2 message class."""
        return cls._ros_type
```

### _ros_type

Class attribute that holds the ROS 2 message class. This is used to create subscriptions, validate topic compatibility, and generate UI schemas.

### callback

A callback class (typically a subclass of `GenericCallback`) that handles deserialization of incoming ROS messages. Different types use specialized callbacks -- for example, `ImageCallback` for sensor images, `OdomCallback` for odometry, `StdMsgCallback` for simple `std_msgs` types.

### convert

A classmethod that takes Python-native data and returns a ROS message instance. This is called by `Publisher` when a component publishes output. For example, `Image.convert()` accepts either a `numpy.ndarray` or an existing `ROSImage` and returns a `ROSImage`.

## Built-in Types

Sugarcoat ships with the following built-in types in `ros_sugar.io.supported_types`:

| Type | ROS Message | Callback |
|:-----|:------------|:---------|
| `String` | `std_msgs/String` | `TextCallback` |
| `Bool` | `std_msgs/Bool` | `StdMsgCallback` |
| `Float32` | `std_msgs/Float32` | `StdMsgCallback` |
| `Float32MultiArray` | `std_msgs/Float32MultiArray` | `StdMsgArrayCallback` |
| `Float64` | `std_msgs/Float64` | `StdMsgCallback` |
| `Float64MultiArray` | `std_msgs/Float64MultiArray` | `StdMsgArrayCallback` |
| `Image` | `sensor_msgs/Image` | `ImageCallback` |
| `CompressedImage` | `sensor_msgs/CompressedImage` | `CompressedImageCallback` |
| `Audio` | `std_msgs/ByteMultiArray` | `AudioCallback` |
| `MapMetaData` | `nav_msgs/MapMetaData` | `MapMetaDataCallback` |
| `Odometry` | `nav_msgs/Odometry` | `OdomCallback` |
| `LaserScan` | `sensor_msgs/LaserScan` | `GenericCallback` |
| `Path` | `nav_msgs/Path` | `PathCallback` |
| `OccupancyGrid` | `nav_msgs/OccupancyGrid` | `OccupancyGridCallback` |
| `Point` | `geometry_msgs/Point` | `PointCallback` |
| `PointStamped` | `geometry_msgs/PointStamped` | `PointStampedCallback` |
| `Pose` | `geometry_msgs/Pose` | `PoseCallback` |
| `PoseStamped` | `geometry_msgs/PoseStamped` | `PoseStampedCallback` |
| `ComponentStatus` | `automatika_ros_sugar/ComponentStatus` | `GenericCallback` |
| `Twist` | `geometry_msgs/Twist` | _(default)_ |

## Registering Additional Types

Use `add_additional_datatypes()` to register custom types at runtime:

```python
from ros_sugar.io.supported_types import add_additional_datatypes

add_additional_datatypes([MyCustomType, AnotherType])
```

This function maintains a global `_additional_types` dictionary. When a type with the same `__name__` already exists, the function merges callbacks and conversion functions rather than replacing the existing entry. This allows multiple packages to augment the same type with additional callbacks.

## Step-by-Step: Adding a Custom Type

Suppose you want to add support for `sensor_msgs/Range`:

### 1. Define the type class

```python
from sensor_msgs.msg import Range as ROSRange
from ros_sugar.io.supported_types import SupportedType
from ros_sugar.io.callbacks import GenericCallback


class Range(SupportedType):
    """Range sensor message support."""

    _ros_type = ROSRange
    callback = GenericCallback

    @classmethod
    def convert(cls, range_value: float, **_) -> ROSRange:
        msg = ROSRange()
        msg.range = range_value
        return msg
```

### 2. Register the type

```python
from ros_sugar.io.supported_types import add_additional_datatypes

add_additional_datatypes([Range])
```

### 3. Use it in a Topic

```python
from ros_sugar.io import Topic

range_topic = Topic(name="/front_sonar", msg_type=Range)
```

The topic can now be used as an input or output on any `BaseComponent`.

## Custom Callbacks

If the default `GenericCallback` is insufficient (for example, you need to extract specific fields or perform numpy conversions), create a custom callback class inheriting from `GenericCallback`:

```python
from ros_sugar.io.callbacks import GenericCallback

class RangeCallback(GenericCallback):
    def __call__(self, msg):
        # Extract and store just the range value
        self.output = msg.range
```

Then assign it to your type:

```python
class Range(SupportedType):
    _ros_type = ROSRange
    callback = RangeCallback
    # ...
```

## How Derived Packages Register Types

Packages built on Sugarcoat (such as Kompass or EmbodiedAgents) register their own types by calling `add_additional_datatypes()` at import time. For example, a navigation package might add:

```python
# In my_nav_package/__init__.py
from ros_sugar.io.supported_types import add_additional_datatypes
from .types import CostMap, Waypoint, TrajectoryArray

add_additional_datatypes([CostMap, Waypoint, TrajectoryArray])
```

This ensures that when any component from `my_nav_package` is imported, the types are immediately available for topic wiring and event conditions.

### Merging Behavior

If two packages register a type with the same class name, `add_additional_datatypes()` merges them:

- **callback**: If the existing type has no callback, the new one is used. If both have callbacks, they are combined into a list.
- **_ros_type**: Only overwritten if the existing type has no `_ros_type` set.
- **convert**: Merged using the same list-accumulation logic as callbacks.

This allows, for example, one package to define the `_ros_type` and another to supply a specialized `convert` function for the same message type.
