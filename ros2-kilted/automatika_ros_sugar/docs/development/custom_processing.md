# Adding Processing Pipelines to I/O

This guide covers how to inject data transformations into the input and output paths of a component without modifying the component itself. Read {doc}`architecture` and {doc}`custom_types` first.

## Overview

Data flows through a component in two directions:

```
ROS message → Callback → post-processors → component._execution_step()
                                                    │
component._execution_step() → pre-processors → Publisher → ROS message
```

You can insert processing functions at two points:

- **Post-processors** on callbacks: transform data *after* the callback deserializes it but *before* the component reads it.
- **Pre-processors** on publishers: transform data *after* the component produces it but *before* it is converted to a ROS message and published.

This lets you add filtering, coordinate transforms, format conversion, or bridging logic from outside the component.

## Callback Post-Processors

### Adding at the Component Level

Use `add_callback_postprocessor()` to attach a function to a specific input topic. The function receives the callback output and must return the same type:

```python
from ros_sugar.io import Topic
from ros_sugar.io.supported_types import Float64

raw = Topic(name="sensor", msg_type=Float64)

component = MyComponent(inputs=[raw], outputs=[...])

def clamp_value(value):
    """Clamp sensor readings to [0, 100]."""
    if value is None:
        return None
    return max(0.0, min(100.0, value))

component.add_callback_postprocessor(raw, clamp_value)
```

The post-processor is called every time `get_output()` is invoked on that callback. Multiple post-processors execute in the order they were added.

### Adding Directly to a Callback

If you have access to the callback object (e.g. inside a component subclass), you can attach post-processors directly:

```python
def custom_on_activate(self):
    cb = self.callbacks["sensor"]
    cb.add_post_processors([clamp_value, unit_convert])
```

### Post-Processor Signature

```python
def post_processor(output: T) -> T:
    """
    Receives the deserialized callback output.
    Must return the same type, or None to signal no data.
    """
    return transformed_output
```

Post-processors can also be `socket` objects for inter-process communication — the output is sent over the socket and the response is used as the transformed value.

## Publisher Pre-Processors

### Adding at the Component Level

Use `add_publisher_preprocessor()` to attach a function to a specific output topic. The function receives the data before it is converted to a ROS message:

```python
from ros_sugar.io import Topic
from ros_sugar.io.supported_types import Float64

output = Topic(name="command", msg_type=Float64)

component = MyComponent(inputs=[...], outputs=[output])

def scale_output(value):
    """Scale command output to motor range."""
    if value is None:
        return None
    return value * 0.01

component.add_publisher_preprocessor(output, scale_output)
```

### Pre-Processor Signature

```python
def pre_processor(output: T) -> T:
    """
    Receives the component's output data before ROS conversion.
    Must return the same type, or None to skip publishing.
    """
    return transformed_output
```

Like post-processors, pre-processors can also be `socket` objects. Multiple pre-processors execute in order.

## Attaching Custom Callbacks

For cases where you need to run logic *every time a message arrives* on a topic (not just when the component reads the data), use `attach_custom_callback()`:

```python
def log_every_message(msg, topic, output):
    """Called on every message arrival, before execution_step."""
    print(f"Got message on {topic.name}: {output}")

component.attach_custom_callback(sensor_topic, log_every_message)
```

### Custom Callback Signature

```python
def custom_callback(
    msg,        # Raw ROS message
    topic,      # Topic object
    output,     # Processed callback output (after post-processors)
) -> None:
    pass
```

This is useful for logging, metrics collection, or triggering side effects independent of the component's main loop.

## Event-Triggered Callbacks

Callbacks support an `on_callback_execute` hook that fires every time a message is received and processed:

```python
def on_new_detection(output):
    """Called every time a new detection arrives."""
    if output is not None:
        log_detection(output)

# Inside a component subclass
def custom_on_activate(self):
    self.callbacks["detections"].on_callback_execute(
        callback=on_new_detection,
        get_processed=True,  # Pass post-processed output (default)
    )
```

Set `get_processed=False` to receive the raw callback output before post-processors.

## Practical Example: Coordinate Transform Pipeline

This example adds a TF-based coordinate transform to a component's input without modifying the component:

```python
import numpy as np
from ros_sugar.io import Topic
from ros_sugar.io.supported_types import PoseStamped
from ros_sugar.launch import Launcher

goal = Topic(name="goal_in_camera_frame", msg_type=PoseStamped)
command = Topic(name="command", msg_type=PoseStamped)

planner = MyPlanner(inputs=[goal], outputs=[command])

# Transform goals from camera frame to base frame
def camera_to_base(pose):
    if pose is None:
        return None
    # Apply a static transform (in practice, use TF)
    transformed = pose.copy()
    transformed["position"]["x"] += 0.15  # camera offset
    return transformed

planner.add_callback_postprocessor(goal, camera_to_base)

# Scale commands for a specific robot's actuator range
def scale_for_robot(cmd):
    if cmd is None:
        return None
    cmd["position"]["x"] *= 0.001  # mm to m
    return cmd

planner.add_publisher_preprocessor(command, scale_for_robot)

launcher = Launcher()
launcher.add_pkg(components=[planner])
launcher.bringup()
```

## When to Use Processors vs. a New Component

| Use case | Approach |
|:---------|:---------|
| Simple data transform (unit conversion, clamping, scaling) | Post/pre-processor |
| Logging, metrics, side effects on message arrival | `attach_custom_callback()` or `on_callback_execute()` |
| Complex multi-input logic, stateful processing, model inference | New component |
| Robot-specific topic adaptation | Robot plugin (see {doc}`custom_robot_plugin`) |

Processors are best for stateless, single-topic transforms. If your transform needs to combine multiple inputs or maintain state across calls, create a dedicated component instead.
