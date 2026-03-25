# Architecture Overview

This document describes the internal architecture of Sugarcoat for developers contributing to or extending the framework.

## Core Module Structure

The `ros_sugar.core` package exposes the primary building blocks:

| Class | Base Class | Role |
|:------|:-----------|:-----|
| `BaseComponent` | `rclpy.lifecycle.Node` | Managed lifecycle execution unit |
| `Monitor` | `rclpy.node.Node` | Event evaluation and component supervision |
| `Event` | _(standalone)_ | Condition-based trigger on topic data |
| `Action` | _(standalone)_ | Callable dispatched when an event fires |
| `Status` | _(standalone)_ | Health status wrapper around `ComponentStatus` msg |
| `Fallback` / `ComponentFallbacks` | _(attrs / standalone)_ | Failure recovery actions |

### BaseComponent

`BaseComponent` extends `rclpy.lifecycle.Node` and is the primary unit of execution. It wraps lifecycle management, declarative I/O wiring, type-safe configuration via `attrs`, health status broadcasting, and fallback handling into a single class.

Key constructor parameters:

```python
BaseComponent(
    component_name: str,
    inputs: Optional[Sequence[Topic]] = None,
    outputs: Optional[Sequence[Topic]] = None,
    config: Optional[BaseComponentConfig] = None,
    fallbacks: Optional[ComponentFallbacks] = None,
    callback_group: Optional[CallbackGroup] = None,
    main_action_type: Optional[type] = None,
    main_srv_type: Optional[type] = None,
)
```

### Monitor

`Monitor` extends `rclpy.node.Node` (a standard, non-lifecycle node). It is responsible for:

- Subscribing to all registered `Event` topics and evaluating conditions via an event blackboard (`EventBlackboardEntry` cache).
- Subscribing to `ComponentStatus` topics for every managed component.
- Creating service clients for component reconfiguration and lifecycle transitions.
- Emitting `InternalEvent` instances back to the `Launcher` so the corresponding `Action` can be dispatched via the ROS launch event system.

When using the `Launcher`, the `Monitor` is created and configured automatically; users do not need to instantiate it directly.

### Launcher

`Launcher` (in `ros_sugar.launch.launcher`) provides a Pythonic alternative to `ros2 launch`. It:

- Accepts components, events-actions mappings, and execution configuration.
- Spawns components as lifecycle nodes in multi-threaded or multi-process mode.
- Instantiates a `Monitor` internally to supervise the system.
- Bridges `InternalEvent` / `OnInternalEvent` to the ROS 2 launch event loop for action dispatch.

## Component Lifecycle

`BaseComponent` follows the ROS 2 managed lifecycle with four transition callbacks:

```
[Unconfigured] --on_configure--> [Inactive] --on_activate--> [Active]
     ^                               |                          |
     |                               |<---on_deactivate---------|
     |<------on_cleanup--------------|
```

### on_configure

Called when the component transitions from **Unconfigured** to **Inactive**. This is where the component:

- Creates ROS subscriptions for declared `inputs` (each `Topic` is wired to a `GenericCallback`).
- Creates ROS publishers for declared `outputs`.
- Sets up service servers (parameter change, topic replacement, config-from-file).
- Initializes the health `Status` and its publisher.
- Loads configuration from file if `config_file` was provided.

### on_activate

Called when the component transitions from **Inactive** to **Active**. Override this in subclasses to start timers, enable processing loops, or begin publishing.

### on_deactivate

Transitions from **Active** back to **Inactive**. Override to pause processing, cancel timers, or stop publishing.

### on_cleanup

Transitions from **Inactive** back to **Unconfigured**. Override to release resources, destroy subscriptions, and reset internal state.

## IO Module

The `ros_sugar.io` package handles typed topic communication.

### Topic

`Topic` is a descriptor that binds a ROS topic name to a `SupportedType`. It carries the topic name, message type, and QoS profile. Topics are declared on components as `inputs` and `outputs` and are automatically wired during `on_configure`.

### Publisher

`Publisher` wraps `rclpy.publisher.Publisher` and adds the `SupportedType.convert()` step so that components can publish Python-native data (e.g., `numpy` arrays) without manually constructing ROS messages.

### SupportedType

`SupportedType` is the base class for the type system. Each subclass maps a ROS message type (`_ros_type`), a deserialization callback (`callback`), and a conversion function (`convert`) that produces the ROS message from Python data. See {doc}`custom_types` for details on extending it.

## Callback Groups

`BaseComponent` uses ROS 2 callback groups to control concurrency:

- **`MutuallyExclusiveCallbackGroup`** -- Default for service callbacks; ensures serial execution.
- **`ReentrantCallbackGroup`** -- Used when the component needs concurrent subscription callbacks (e.g., multiple sensor streams processed in parallel).

The callback group can be specified at construction via the `callback_group` parameter.

## Key Decorators

### @component_action

Defined in `ros_sugar.utils.component_action`. Marks a method as an action that can be dispatched by the event system. The decorator enforces:

1. The method belongs to a `LifecycleNode` instance.
2. The return type annotation is `bool` or `None`.
3. If `active=True`, the component must be in the **Active** lifecycle state.

```python
from ros_sugar.utils import component_action

class MyComponent(BaseComponent):
    @component_action
    def stop_motors(self) -> bool:
        # ... stop logic ...
        return True
```

### @component_fallback

Defined in `ros_sugar.utils.component_fallback`. Marks a method as a fallback handler. The decorator verifies that rclpy is initialized and the component is at least in the **Inactive** state (i.e., configured or active). This allows fallbacks to fire even when the component has been deactivated due to an error.

```python
from ros_sugar.utils import component_fallback

class MyComponent(BaseComponent):
    @component_fallback
    def restart(self) -> None:
        self.trigger_deactivate()
        self.trigger_activate()
```

### @action_handler

Defined in `ros_sugar.utils.action_handler`. Used internally to validate that a function returns `SomeEntitiesType` (the ROS launch entity type). This is primarily for functions that integrate directly with the launch event system.

## Monitor Orchestration

At runtime the `Monitor` operates a tight evaluation loop:

1. **Receive** -- Subscription callbacks write incoming messages into a shared `Dict[str, EventBlackboardEntry]` (the "blackboard"). Each entry carries a UUID and timestamp for staleness detection.
2. **Evaluate** -- For every registered `Event`, `Monitor` calls `event.check_condition(blackboard)`. The `Condition` tree is evaluated against the cached topic messages. Composite conditions (AND / OR / NOT via `ConditionLogicOp`) are resolved recursively.
3. **Trigger** -- If a condition evaluates to `True`, the event's registered actions are submitted to a shared `ThreadPoolExecutor` for non-blocking execution.
4. **Emit** -- For actions that must be handled at the launch level (lifecycle transitions, process restarts), the `Monitor` emits an `InternalEvent` which is caught by an `OnInternalEvent` handler registered by the `Launcher`.

## Launcher Process Graph

The `Launcher` supports two execution modes:

### Multi-Threaded

All components run in the same process. Each component gets its own callback group. The `Launcher` uses a `MultiThreadedExecutor` to spin all nodes concurrently. This is simpler but shares a single fault domain.

### Multi-Process

Each component is launched as a separate ROS 2 process via `ExecuteProcess`. The `Launcher` communicates with components through ROS services and the `Monitor`'s topic subscriptions. This provides process isolation -- a crash in one component does not bring down the others.

In both modes, the `Monitor` node runs in the main launcher process and coordinates lifecycle transitions via `LifecycleTransition` launch actions.
