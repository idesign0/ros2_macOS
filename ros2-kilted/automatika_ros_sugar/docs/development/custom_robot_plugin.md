# Creating a Robot Plugin

This guide walks through building a robot plugin that adapts any Sugarcoat-based stack to a specific robot's hardware interfaces. A plugin maps generic topic types (e.g. `Twist`, `Odometry`) to robot-specific ROS 2 topics or service clients — without modifying any component code.

## When to Use a Plugin

Use a robot plugin when:

- Your robot uses custom ROS message types instead of standard ones (e.g. a custom odometry message instead of `nav_msgs/Odometry`).
- Your robot exposes motion commands as ROS 2 services instead of topic subscriptions.
- You want the same Sugarcoat recipe to run on different robots by swapping a single config parameter.

## Plugin Structure

A robot plugin is a standard Python package that exports two dictionaries:

```
myrobot_plugin/
├── __init__.py       # Exports robot_feedback and robot_action dicts
├── types.py          # SupportedType wrappers for custom messages
└── clients.py        # Service clients for action interfaces (optional)
```

### The Two Dictionaries

```python
# __init__.py
from typing import Dict, Union, Type
from ros_sugar.io import Topic
from ros_sugar.base_clients import ServiceClientHandler

robot_feedback: Dict[str, Union[Topic, Type[ServiceClientHandler]]]
robot_action: Dict[str, Union[Topic, Type[ServiceClientHandler]]]
```

- **`robot_feedback`**: Maps standard type names to robot-specific **input** topics. When a component subscribes to an `Odometry` topic, the plugin replaces it with your robot's custom odometry topic and type.
- **`robot_action`**: Maps standard type names to robot-specific **output** topics or service clients. When a component publishes a `Twist` command, the plugin routes it to your robot's custom interface.

Dictionary keys are standard ROS 2 type names (e.g. `"Twist"`, `"Odometry"`, `"String"`). These are matched against the message types of component input/output topics.

## Step 1: Define Custom Type Wrappers

Use `create_supported_type()` to wrap your robot's custom messages with conversion functions:

```python
# types.py
import numpy as np
from ros_sugar.robot_plugin import create_supported_type
from myrobot_msgs.msg import CustomOdom, CustomTwist


# --- Feedback: custom ROS message → Python type ---

def _odom_callback(msg: CustomOdom, **_) -> np.ndarray:
    """Convert robot's odometry to a numpy array."""
    return np.array([msg.x, msg.y, msg.yaw])

RobotOdometry = create_supported_type(CustomOdom, callback=_odom_callback)


# --- Action: Python types → custom ROS message ---

def _twist_converter(vx: float, vy: float, omega: float, **_) -> CustomTwist:
    """Convert velocity command to robot's custom twist."""
    msg = CustomTwist()
    msg.vx = vx
    msg.vy = vy
    msg.vyaw = omega
    return msg

RobotTwist = create_supported_type(CustomTwist, converter=_twist_converter)
```

### `create_supported_type()` Parameters

| Parameter | For | Signature requirement |
|:----------|:----|:----------------------|
| `ros_msg_type` | Both | A valid ROS 2 message class |
| `callback` | Feedback (input) | `(ros_msg, **kwargs) -> python_type` — return annotation must differ from `ros_msg_type` |
| `converter` | Action (output) | `(python_args) -> ros_msg` — return annotation must match `ros_msg_type` |

## Step 2: Create Service Clients (Optional)

If your robot exposes commands via ROS 2 services instead of topics, subclass `RobotPluginServiceClient`:

```python
# clients.py
from rclpy.node import Node
from ros_sugar.robot_plugin import RobotPluginServiceClient
from myrobot_msgs.srv import RobotCommand


class TwistServiceClient(RobotPluginServiceClient):
    """Sends velocity commands via a ROS 2 service."""

    def __init__(self, client_node: Node, srv_name: str = "robot_cmd"):
        super().__init__(
            srv_type=RobotCommand,
            srv_name=srv_name,
            client_node=client_node,
        )

    def _publish(self, vx, vy, omega, **_) -> bool:
        """Create and send the service request."""
        req = RobotCommand.Request()
        req.vx = vx
        req.vy = vy
        req.omega = omega
        response = self.send_request(req_msg=req)
        return response is not None

    def start(self) -> bool:
        """Called when the component activates. Enable autonomous mode."""
        req = RobotCommand.Request()
        req.enable = True
        self.send_request(req_msg=req)
        return True

    def end(self) -> bool:
        """Called when the component deactivates. Disable autonomous mode."""
        req = RobotCommand.Request()
        req.enable = False
        self.send_request(req_msg=req)
        return True
```

### Methods to Implement

| Method | Required | Description |
|:-------|:---------|:------------|
| `_publish(*args, **kwargs) -> bool` | Yes | Send the actual command. Receives the same arguments the original publisher would. |
| `start() -> bool` | Yes | Lifecycle hook — runs when the component activates. |
| `end() -> bool` | Yes | Lifecycle hook — runs when the component deactivates. |

The base class handles pre-processor transfer: when a service client replaces a publisher, any pre-processors attached to the original publisher are preserved and applied before `_publish()` is called.

## Step 3: Register the Plugin

Wire everything together in `__init__.py`:

```python
# __init__.py
from typing import Dict, Union, Type
from ros_sugar.io import Topic
from ros_sugar.base_clients import ServiceClientHandler
from . import types, clients

# Input topics: standard type name → robot-specific topic
robot_feedback: Dict[str, Union[Topic, Type[ServiceClientHandler]]] = {
    "Odometry": Topic(name="myrobot/odom", msg_type=types.RobotOdometry),
}

# Output topics: standard type name → robot-specific topic or service client
robot_action: Dict[str, Union[Topic, Type[ServiceClientHandler]]] = {
    "Twist": clients.TwistServiceClient,
    "TwistStamped": clients.TwistServiceClient,
}

__all__ = ["robot_feedback", "robot_action"]
```

Actions can map to either a `Topic` (for topic-based robots) or a `RobotPluginServiceClient` subclass (for service-based robots). You can mix both in the same plugin.

## Step 4: Enable the Plugin

Set the plugin module name in the component configuration:

```python
from ros_sugar.config import BaseComponentConfig

config = BaseComponentConfig(
    _robot_plugin="myrobot_plugin",
    _enable_plugin_feedbacks_handling=True,   # Replace input topics (default True)
    _enable_plugin_actions_handling=True,      # Replace output topics/clients
)
```

Or pass it through the Launcher to apply to all components:

```python
from ros_sugar.launch import Launcher

launcher = Launcher(robot_plugin="myrobot_plugin")
launcher.add_pkg(components=[...])
launcher.bringup()
```

## How Replacement Works

During component activation, if `_robot_plugin` is set:

1. The plugin module is imported dynamically.
2. For each **input** topic, its message type name is looked up in `robot_feedback`. If found, the subscriber is destroyed and recreated with the plugin's topic name and type.
3. For each **output** topic, its message type name is looked up in `robot_action`:
   - If the value is a `Topic`: the publisher is replaced with the new topic name and type.
   - If the value is a `RobotPluginServiceClient` subclass: the publisher is replaced with a service client instance. Pre-processors are transferred automatically.

Components are unaware of the replacement — they continue calling `self.callbacks["topic"].get_output()` and `self.publishers_dict["topic"].publish(...)` as normal.

## Complete Example

A full plugin for a robot with custom odometry and service-based velocity control:

```python
# myrobot_plugin/__init__.py
from typing import Dict, Union, Type
from ros_sugar.io import Topic
from ros_sugar.base_clients import ServiceClientHandler

from . import types, clients

robot_feedback: Dict[str, Union[Topic, Type[ServiceClientHandler]]] = {
    "Odometry": Topic(name="myrobot/odom", msg_type=types.RobotOdometry),
    "LaserScan": Topic(name="myrobot/lidar", msg_type=types.RobotLidar),
}

robot_action: Dict[str, Union[Topic, Type[ServiceClientHandler]]] = {
    "Twist": clients.TwistServiceClient,
    "TwistStamped": clients.TwistServiceClient,
}

__all__ = ["robot_feedback", "robot_action"]
```

Usage in a recipe:

```python
from ros_sugar.config import BaseComponentConfig
from ros_sugar.launch import Launcher

# Same recipe works with any robot — just change the plugin
config = BaseComponentConfig(
    _robot_plugin="myrobot_plugin",
    _enable_plugin_feedbacks_handling=True,
    _enable_plugin_actions_handling=True,
)

my_component = MyComponent(config=config, inputs=[...], outputs=[...])

launcher = Launcher()
launcher.add_pkg(components=[my_component])
launcher.bringup()
```
