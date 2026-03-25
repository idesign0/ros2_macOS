# Custom ROS Message Types

EmbodiedAgents defines custom ROS2 message and action types for data that does not fit standard ROS messages. This guide covers the existing types, how they connect to the Python type system via `SupportedType`, and how to add new message types.

## Existing Message Types

The following custom messages are defined in the `automatika_embodied_agents` package under `msg/`:

| Message | File | Description |
|---|---|---|
| `Point2D` | `Point2D.msg` | A 2D point with `x` and `y` float fields. Used as a building block in other messages. |
| `Bbox2D` | `Bbox2D.msg` | A 2D bounding box with `top_left_x`, `top_left_y`, `bottom_right_x`, `bottom_right_y`. |
| `Detections2D` | `Detections2D.msg` | Object detection results: arrays of `scores`, `labels`, `boxes` (Bbox2D[]), plus optional `image` and `depth`. |
| `Detections2DMultiSource` | `Detections2DMultiSource.msg` | An array of `Detections2D` for multi-camera setups. |
| `Trackings` | `Trackings.msg` | Tracked object data: `ids`, `labels`, `boxes`, `centroids`, `estimated_velocities`, plus source image. |
| `TrackingsMultiSource` | `TrackingsMultiSource.msg` | An array of `Trackings` for multi-camera tracking. |
| `StreamingString` | `StreamingString.msg` | A string with `stream` (bool) and `done` (bool) flags for token-by-token LLM output. |
| `Video` | `Video.msg` | A sequence of `Image` and/or `CompressedImage` frames bundled as a single message. |
| `PointsOfInterest` | `PointsOfInterest.msg` | A list of `Point2D` coordinates on an image, plus the source image/depth. |

### Action Types

| Action | File | Description |
|---|---|---|
| `VisionLanguageAction` | `VisionLanguageAction.action` | ROS2 action for VLA inference. Defines goal, feedback, and result for vision-language-action loops. |

## The `SupportedType` System

Every ROS message used as a topic type in EmbodiedAgents must have a corresponding Python `SupportedType` wrapper class. This class (from Sugarcoat's `ros_sugar.supported_types`) provides three things:

1. **`_ros_type`**: Class attribute pointing to the actual ROS message class.
2. **`callback`**: A callback class that handles deserialization and buffering of incoming messages.
3. **`convert()` class method**: Converts Python data (dicts, numpy arrays, etc.) into a ROS message for publishing.

Example from the codebase -- the `StreamingString` wrapper:

```python
from ros_sugar.supported_types import SupportedType

class StreamingString(SupportedType):
    callback = StreamingStringCallback
    _ros_type = ROSStreamingString

    @classmethod
    def convert(cls, output: str, stream: bool = False, done: bool = True, **_):
        msg = ROSStreamingString()
        msg.stream = stream
        msg.done = done
        msg.data = output
        return msg
```

### Registration with `add_additional_datatypes()`

After defining wrapper classes, they must be registered so that `Topic(msg_type="StreamingString")` resolves correctly:

```python
from ros_sugar.supported_types import add_additional_datatypes

agent_types = [StreamingString, Video, Detections, ...]
add_additional_datatypes(agent_types)
```

This is done at the bottom of `agents/ros.py` and makes these types available as string identifiers in `Topic` definitions.

## When to Create New Message Types

Create a new custom message when:

- Standard ROS messages (`String`, `Image`, `Odometry`, etc.) do not capture the data structure you need.
- Your component produces structured output that downstream components must parse (e.g., detection results with labels and bounding boxes).
- You need to bundle multiple pieces of data atomically (e.g., image + detections, or video frames).

Do **not** create a new message if a standard type suffices -- use `String`, `Image`, `Audio`, `Odometry`, `OccupancyGrid`, `JointState`, `JointTrajectory`, etc. from the existing supported types.

## Step-by-Step: Adding a New Message Type

### Step 1: Define the `.msg` File

Create a new `.msg` file in the `msg/` directory of the `automatika_embodied_agents` package:

```
# msg/SemanticLabel.msg
string label
float32 confidence
sensor_msgs/Image image
```

### Step 2: Register in CMakeLists.txt

Add the new message file to the `rosidl_generate_interfaces` call in `CMakeLists.txt`:

```cmake
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/SemanticLabel.msg"
  # ... existing messages ...
  DEPENDENCIES std_msgs sensor_msgs
)
```

### Step 3: Build the Package

```bash
cd <workspace_root>
colcon build --packages-select automatika_embodied_agents
source install/setup.bash
```

After building, the ROS message class will be available as `automatika_embodied_agents.msg.SemanticLabel`.

### Step 4: Create a Callback Class

Define a callback class in `agents/callbacks.py` (or a new file) that extends the base callback from Sugarcoat. The callback handles deserializing the ROS message into Python data:

```python
from ros_sugar.io.callbacks import GenericCallback


class SemanticLabelCallback(GenericCallback):
    """Callback for SemanticLabel messages."""

    def _msg_to_output(self, msg) -> dict:
        """Convert a SemanticLabel ROS message to a Python dict."""
        return {
            "label": msg.label,
            "confidence": msg.confidence,
        }
```

### Step 5: Create the `SupportedType` Wrapper

In `agents/ros.py`, define the wrapper class:

```python
from automatika_embodied_agents.msg import SemanticLabel as ROSSemanticLabel
from .callbacks import SemanticLabelCallback


class SemanticLabel(SupportedType):
    """Wraps automatika_embodied_agents/msg/SemanticLabel."""

    _ros_type = ROSSemanticLabel
    callback = SemanticLabelCallback

    @classmethod
    def convert(cls, output: dict, **_) -> ROSSemanticLabel:
        msg = ROSSemanticLabel()
        msg.label = output["label"]
        msg.confidence = float(output["confidence"])
        return msg
```

### Step 6: Register the New Type

Add it to the `agent_types` list at the bottom of `agents/ros.py`:

```python
agent_types = [
    StreamingString,
    Video,
    Detections,
    # ... existing types ...
    SemanticLabel,  # <-- add here
]

add_additional_datatypes(agent_types)
```

### Step 7: Export and Use

Add the new type to `__all__` in `agents/ros.py`:

```python
__all__ = [
    # ... existing exports ...
    "SemanticLabel",
]
```

Now it can be used in component definitions:

```python
from agents.ros import Topic

label_topic = Topic(name="semantic_label", msg_type="SemanticLabel")
```

## Callback Architecture

Each `SupportedType.callback` class inherits from `GenericCallback` (Sugarcoat). The callback:

1. Is instantiated per-topic when the component creates subscribers.
2. Receives raw ROS messages via its subscriber.
3. Converts them to Python objects via `_msg_to_output()`.
4. Buffers the latest output so the component can read it via `get_output()`.

For types like `Image`, the callback may also handle conversion to numpy arrays. For `Detections`, it parses bounding boxes into Python dicts. This conversion layer is what allows `_execution_step()` to work with clean Python data.
