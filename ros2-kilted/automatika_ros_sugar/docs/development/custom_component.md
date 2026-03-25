# Creating a New BaseComponent

This guide walks through subclassing `BaseComponent` to create a new component type. This is the primary extension point in Sugarcoat — both [EmbodiedAgents](https://github.com/automatika-robotics/embodied-agents) and [Kompass](https://github.com/automatika-robotics/kompass) build their component layers by subclassing `BaseComponent`. Read {doc}`architecture` first for the overall design.

## Choosing the Right Base

`BaseComponent` wraps a ROS 2 Lifecycle Node with declarative I/O, type-safe configuration, health status, and fallback recovery. Subclass it when you need a new component abstraction that downstream packages will further extend.

If you are building an end-user component (not a framework layer), consider subclassing one of the higher-level components from EmbodiedAgents or Kompass instead.

## Constructor

Your subclass constructor should accept inputs, outputs, config, and a trigger, then call `super().__init__()`:

```python
from typing import Optional, Sequence
from ros_sugar.core import BaseComponent
from ros_sugar.io import Topic
from ros_sugar.config import BaseComponentConfig

class MyComponent(BaseComponent):
    def __init__(
        self,
        component_name: str = "my_component",
        inputs: Optional[Sequence[Topic]] = None,
        outputs: Optional[Sequence[Topic]] = None,
        config: Optional[BaseComponentConfig] = None,
        config_file: Optional[str] = None,
        **kwargs,
    ):
        super().__init__(
            component_name=component_name,
            inputs=inputs,
            outputs=outputs,
            config=config or BaseComponentConfig(),
            config_file=config_file,
            **kwargs,
        )
```

### Constructor Parameters

| Parameter | Type | Description |
|:----------|:-----|:------------|
| `component_name` | `str` | ROS 2 node name |
| `inputs` | `Sequence[Topic]` | Input topics the component subscribes to |
| `outputs` | `Sequence[Topic]` | Output topics the component publishes to |
| `config` | `BaseComponentConfig` | Component configuration (loop rate, run type, etc.) |
| `config_file` | `str` | Path to YAML/JSON/TOML config file (alternative to `config`) |
| `callback_group` | `CallbackGroup` | ROS 2 callback group; defaults to `ReentrantCallbackGroup` |
| `main_action_type` | `type` | ROS 2 action type (required when run type is `ACTION_SERVER`) |
| `main_srv_type` | `type` | ROS 2 service type (required when run type is `SERVER`) |

## Execution Methods

These are the methods your subclass must or can implement. The one you **must** implement depends on the component's run type.

### `_execution_step()` — TIMED and EVENT modes

Called every timer cycle at `loop_rate` frequency. This is where your core logic lives:

```python
def _execution_step(self):
    if not self.got_all_inputs():
        missing = self.get_missing_inputs()
        self.health_status.set_fail_system(topic_names=missing)
        return

    sensor = self.callbacks["sensor"].get_output()
    if sensor is None:
        return

    result = self.process(sensor)
    self.health_status.set_healthy()
    self.publishers_dict["output"].publish(result)
```

### `main_action_callback()` — ACTION_SERVER mode

Called when an action goal is received. Required when `run_type == ComponentRunType.ACTION_SERVER`:

```python
def main_action_callback(self, goal_handle):
    result = self.process_goal(goal_handle.request)
    goal_handle.succeed()
    return result
```

### `main_service_callback()` — SERVER mode

Called when a service request is received. Required when `run_type == ComponentRunType.SERVER`:

```python
def main_service_callback(self, request, response):
    response.result = self.compute(request.data)
    return response
```

## Lifecycle Hooks

Override these to run custom logic during lifecycle transitions. All are optional with empty defaults:

| Hook | Called when | Common use |
|:-----|:-----------|:-----------|
| `init_variables()` | Start of activation | Initialize state variables |
| `custom_on_configure()` | After configuration | Set up internal resources |
| `custom_on_activate()` | After activation | Start background tasks, create TF listeners |
| `custom_on_deactivate()` | After deactivation | Pause background tasks |
| `custom_on_cleanup()` | During cleanup | Release resources |
| `custom_on_shutdown()` | During shutdown | Final cleanup |
| `custom_on_error()` | On transition error | Error-specific handling |

The base class resets health status to healthy on `configure`, `activate`, and `deactivate`. On `error`, it sets `set_fail_component()`. You generally don't need to manage status in lifecycle hooks.

```python
class MyComponent(BaseComponent):
    def init_variables(self):
        self.counter = 0
        self.buffer = []

    def custom_on_configure(self):
        self.get_logger().info("Configured")

    def custom_on_activate(self):
        self.tf_listener = self.create_tf_listener(
            TFListenerConfig(lookup_rate=10.0)
        )
```

## Accessing Inputs and Outputs

### Reading from Inputs

Input data is available through `self.callbacks`, a dict mapping topic names to callback objects:

```python
# Check if all inputs have received at least one message
if self.got_all_inputs():
    data = self.callbacks["my_topic"].get_output()

# Check specific inputs only
if self.got_all_inputs(inputs_to_check=["critical_topic"]):
    ...

# Exclude optional inputs from the check
if self.got_all_inputs(inputs_to_exclude=["optional_topic"]):
    ...

# Get list of topics that haven't received data yet
missing = self.get_missing_inputs()
```

### Publishing to Outputs

Output publishers are available through `self.publishers_dict`:

```python
# Publish data (automatically converted via SupportedType.convert())
self.publishers_dict["output"].publish(result)

# With frame_id for stamped messages
self.publishers_dict["pose"].publish(pose_data, frame_id="map")
```

## Run Types

Set the run type via configuration to control how `_execution_step()` is triggered:

| Run Type | Behavior | Requires |
|:---------|:---------|:---------|
| `TIMED` | Fires `_execution_step()` at `loop_rate` Hz | Nothing extra |
| `EVENT` | Fires on topic/event trigger | Event wiring at Launcher level |
| `SERVER` | Fires `main_service_callback()` on service request | `main_srv_type` parameter |
| `ACTION_SERVER` | Fires `main_action_callback()` on action goal | `main_action_type` parameter |

```python
from ros_sugar.config import BaseComponentConfig, ComponentRunType

# Timed: execute at 50 Hz
config = BaseComponentConfig(loop_rate=50.0)

# Server: respond to service requests
component = MyComponent(
    config=BaseComponentConfig(_run_type=ComponentRunType.SERVER),
    main_srv_type=MyService,
)
```

## Configuration

### Extending BaseComponentConfig

Define a custom config class using `attrs` for component-specific parameters:

```python
from attrs import define, field
from ros_sugar.config import BaseComponentConfig, base_validators

@define(kw_only=True)
class MyConfig(BaseComponentConfig):
    threshold: float = field(default=0.5, validator=base_validators.in_range(0.0, 1.0))
    window_size: int = field(default=10, validator=base_validators.gt(0))
    mode: str = field(default="fast", validator=base_validators.in_(["fast", "accurate"]))
```

Key points:

- Always use `@define(kw_only=True)`.
- Use `base_validators` for field validation (`gt`, `in_range`, `in_`).
- Configs are serializable to YAML/JSON/TOML via `to_file()` / `from_file()`.

### Loading from File

```python
# At construction
component = MyComponent(config_file="/path/to/config.yaml")

# At runtime
component.config_from_file("/path/to/config.yaml")
```

YAML structure:

```yaml
my_component:            # Must match component_name
  loop_rate: 50.0
  threshold: 0.8
  window_size: 20
```

## Restricting Allowed Topics

Use `AllowedTopics` to enforce which message types a component accepts:

```python
from ros_sugar.io import AllowedTopics
from ros_sugar.io.supported_types import Image, String, Float64

class MyComponent(BaseComponent):
    def __init__(self, **kwargs):
        self.allowed_inputs = {
            "Required": AllowedTopics(types=[Image], number_required=1),
            "Optional": AllowedTopics(types=[String], number_required=0, number_optional=1),
        }
        self.allowed_outputs = {
            "Required": AllowedTopics(types=[Float64], number_required=1),
        }
        super().__init__(**kwargs)
```

The validation runs during initialization and raises if required topics are missing or types don't match.

## Custom Actions and Fallbacks

### Defining Component Actions

Use the `@component_action` decorator to mark methods as dispatchable actions. These can be used as fallback targets or wired to events:

```python
from ros_sugar.utils import component_action, component_fallback

class MyComponent(BaseComponent):
    @component_action
    def reset_buffer(self) -> bool:
        self.buffer = []
        return True

    @component_fallback
    def emergency_stop(self):
        self.publishers_dict["velocity"].publish(0.0)
```

- `@component_action`: Validates lifecycle state before execution. Return type should be `bool` or `None`.
- `@component_fallback`: Validates the component is in a valid state (active, inactive, or activating).

### Built-in Actions

Every component inherits these actions that can be used directly in fallbacks or events:

| Action | Description |
|:-------|:------------|
| `start()` | Lifecycle activate |
| `stop()` | Lifecycle deactivate |
| `restart(wait_time=None)` | Stop then start |
| `reconfigure(new_config, keep_alive=False)` | Apply new config |
| `set_param(name, value, keep_alive=True)` | Change one parameter |
| `set_params(names, values, keep_alive=True)` | Change multiple parameters |
| `broadcast_status()` | Publish current health status |

## Complete Skeleton

```python
from typing import Optional, Sequence
from attrs import define, field
from ros_sugar.core import BaseComponent, Action
from ros_sugar.io import Topic
from ros_sugar.io.supported_types import Float64, String
from ros_sugar.config import BaseComponentConfig, base_validators
from ros_sugar.utils import component_action
from ros_sugar.launch import Launcher


# --- Config ---
@define(kw_only=True)
class FilterConfig(BaseComponentConfig):
    alpha: float = field(default=0.5, validator=base_validators.in_range(0.0, 1.0))


# --- Component ---
class ExponentialFilter(BaseComponent):
    """Low-pass exponential filter on a float stream."""

    def __init__(
        self,
        component_name: str = "exp_filter",
        inputs: Optional[Sequence[Topic]] = None,
        outputs: Optional[Sequence[Topic]] = None,
        config: Optional[FilterConfig] = None,
        **kwargs,
    ):
        super().__init__(
            component_name=component_name,
            inputs=inputs,
            outputs=outputs,
            config=config or FilterConfig(),
            **kwargs,
        )

    def init_variables(self):
        self._filtered = 0.0

    def _execution_step(self):
        if not self.got_all_inputs():
            self.health_status.set_fail_system(
                topic_names=self.get_missing_inputs()
            )
            return

        raw = self.callbacks["raw_signal"].get_output()
        if raw is None:
            return

        alpha = self.config.alpha
        self._filtered = alpha * raw + (1 - alpha) * self._filtered
        self.health_status.set_healthy()
        self.publishers_dict["filtered_signal"].publish(self._filtered)

    @component_action
    def reset_filter(self) -> bool:
        self._filtered = 0.0
        return True


# --- Usage ---
raw = Topic(name="raw_signal", msg_type=Float64)
filtered = Topic(name="filtered_signal", msg_type=Float64)

filt = ExponentialFilter(
    inputs=[raw],
    outputs=[filtered],
    config=FilterConfig(loop_rate=100.0, alpha=0.3),
)

# Fallback: restart on failure
filt.on_algorithm_fail(
    action=Action(filt.restart),
    max_retries=3,
)

launcher = Launcher()
launcher.add_pkg(components=[filt])
launcher.bringup()
```
