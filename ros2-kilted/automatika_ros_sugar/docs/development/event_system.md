# Event & Action System Internals

This document covers the internal mechanics of the event-action system in Sugarcoat, including event types, condition evaluation, action dispatch, and the fallback system.

## Event Overview

An `Event` (defined in `ros_sugar.core.event`) represents a condition on ROS topic data that, when satisfied, triggers one or more `Action` instances. Events are evaluated continuously by the `Monitor` node against a shared topic cache (the "blackboard").

### Event Construction

Events are constructed from `Condition` objects, which are typically built implicitly through Python comparison operators on topic message builders:

```python
from ros_sugar.io import Topic
from ros_sugar.io.supported_types import Float32

sensor = Topic(name="/battery_level", msg_type=Float32)

# This builds a Condition via MsgConditionBuilder.__gt__
low_battery = Event(sensor.msg.data < 10.0)
```

The expression `sensor.msg.data` returns a `MsgConditionBuilder` that captures the attribute path `["data"]`. Applying a comparison operator (e.g., `<`) produces a `Condition` object with:

- `topic_name`: `"/battery_level"`
- `attribute_path`: `["data"]`
- `operator_func`: `operator.lt`
- `ref_value`: `10.0`

## Condition Tree

Conditions can be composed using logical operators to form a tree:

```python
# AND: both conditions must be true
critical = Event((sensor.msg.data < 5.0) & (motor.msg.status == 1))

# OR: either condition triggers
alert = Event((sensor.msg.data < 10.0) | (temp.msg.data > 80.0))
```

Internally, this creates a composite `Condition` with a `ConditionLogicOp` (`AND`, `OR`, or `NOT`) and a list of `sub_conditions`. Evaluation is recursive -- the `Condition.evaluate()` method walks the tree and applies each leaf condition against the topic cache.

### Nested Attribute Access

`MsgConditionBuilder` supports chained attribute access to reach deeply nested fields in ROS messages:

```python
odom = Topic(name="/odom", msg_type=Odometry)

# Access odom.pose.pose.position.x
position_event = Event(odom.msg.pose.pose.position.x > 5.0)
```

Attribute paths are validated at construction time against the ROS message type hierarchy. An `AttributeError` is raised if the path is invalid.

## Event Types (Condition Patterns)

Sugarcoat supports several event patterns, all built using `Condition` expressions:

| Pattern | Description | Example |
|:--------|:------------|:--------|
| **OnAny** | Fires when any data arrives on the topic | `Event(topic)` (pass a `Topic` directly) |
| **OnEqual** | Fires when value equals reference | `Event(topic.msg.data == 42)` |
| **OnGreater** | Fires when value exceeds reference | `Event(topic.msg.data > threshold)` |
| **OnLess** | Fires when value falls below reference | `Event(topic.msg.data < threshold)` |
| **OnDifferent** | Fires when value differs from reference | `Event(topic.msg.data != expected)` |
| **OnChange** | Fires on transition from False to True | `Event(condition, on_change=True)` |
| **OnCondition** | Fires on arbitrary compound condition | `Event((a.msg.x > 1) & (b.msg.y < 2))` |
| **OnExternalEvent** | Fires from an `InternalEvent` emitted by the launch system | Used internally by `Launcher` |

### OnAny

When a `Topic` object is passed directly (rather than a `Condition`), the event fires whenever all involved topics have data present in the blackboard:

```python
camera_ready = Event(camera_topic)
```

### OnChange

Setting `on_change=True` adds edge-detection semantics. The event fires only on the transition from `False` to `True`, not while the condition remains true:

```python
# Fires once when the robot enters the danger zone, not continuously
entered_danger = Event(sensor.msg.data < 0.5, on_change=True)
```

## JSON Serialization

Events and their conditions support full serialization for multi-process execution. When components run in separate processes, events are serialized via `Event.to_json()` / `Event.from_json()`, which in turn serializes the `Condition` tree. This is used by the `Launcher` when spawning components via `ExecuteProcess`.

```python
event_json = my_event.to_json()
restored_event = Event.from_json(event_json)
```

The serialization preserves the complete condition tree, operator functions (mapped by name), reference values, and topic metadata.

## Action Dispatch

An `Action` (defined in `ros_sugar.core.action`) wraps a callable that is executed when an event triggers. Actions can wrap:

- A component method (decorated with `@component_action`)
- A plain Python function
- An async coroutine
- A ROS launch action (e.g., `LogInfo`)

### Registering Actions

Actions are associated with events through the `Launcher.add_pkg()` method:

```python
from ros_sugar.core import Action, Event

stop_action = Action(my_component.emergency_stop)

launcher.add_pkg(
    components=[my_component],
    events_actions={low_battery: stop_action},
)
```

Internally, `event.register_actions()` stores the actions on the event. When `check_condition()` evaluates to `True`, the actions are submitted to a shared `ThreadPoolExecutor` for non-blocking execution.

### Topic Data Injection

When an action fires, the current topic cache (a dict mapping topic names to their latest messages) is passed as a `topics` keyword argument. Actions can use this to access the data that triggered the event:

```python
def handle_collision(topics: dict = None):
    scan = topics.get("/laser_scan")
    # ... react to scan data ...
```

### Automatic Type Conversion

If an event involves a single topic, `Action._setup_conversions()` attempts to create an automatic parser from the event's message type to the action's expected input type. This allows actions to receive converted data without manual parsing.

## @component_action Decorator

The `component_action` decorator (in `ros_sugar.utils`) validates action methods at call time:

1. **Instance check**: Ensures the method is called on a `LifecycleNode` instance.
2. **Return type**: Verifies the return annotation is `bool` or `None`.
3. **Lifecycle state**: If `active=True`, the component must be in the Active state.

```python
from ros_sugar.utils import component_action

class Navigator(BaseComponent):
    @component_action
    def stop(self) -> bool:
        self.cmd_vel_publisher.publish(Twist())
        return True
```

## Fallback System

The fallback system provides automatic failure recovery. It is managed by `ComponentFallbacks` (in `ros_sugar.core.fallbacks`).

### ComponentFallbacks

`ComponentFallbacks` holds a set of `Fallback` objects, one for each failure level:

| Attribute | Triggered When |
|:----------|:---------------|
| `on_algorithm_fail` | `Status` reports `STATUS_FAILURE_ALGORITHM_LEVEL` |
| `on_component_fail` | `Status` reports `STATUS_FAILURE_COMPONENT_LEVEL` |
| `on_system_fail` | `Status` reports `STATUS_FAILURE_SYSTEM_LEVEL` |
| `on_any_fail` | Any failure without a specific fallback defined |
| `on_giveup` | All fallbacks for a failure level have been exhausted |

### Fallback

Each `Fallback` (an `attrs`-defined class) wraps one or more `Action` instances and a `max_retries` count:

```python
from ros_sugar.core import Action, ComponentFallbacks, Fallback

fallbacks = ComponentFallbacks(
    on_component_fail=Fallback(
        action=[Action(component.restart), Action(component.shutdown)],
        max_retries=3,
    ),
    on_algorithm_fail=Fallback(
        action=Action(component.reset_algorithm),
        max_retries=5,
    ),
)
```

### Failure Hierarchy

When a failure is detected, `ComponentFallbacks` follows this resolution order:

1. Look for a fallback specific to the failure level (`on_algorithm_fail`, `on_component_fail`, or `on_system_fail`).
2. If no specific fallback is defined, fall back to `on_any_fail`.
3. For each fallback, execute the current action up to `max_retries` times.
4. If `max_retries` is exhausted and the fallback has a list of actions, move to the next action in the list.
5. If all actions in the list are exhausted, set the `giveup` flag and execute `on_giveup` if defined.

A successful fallback execution (action returns `True`) resets the health status to `STATUS_HEALTHY`.

### Default Behavior

By default, `BaseComponent` sets `on_any_fail` to `Action(self.broadcast_status)` with `max_retries=None` (infinite retries). This means any failure that does not have a specific fallback will cause the component to broadcast its status continuously, allowing the `Monitor` to detect the issue.

## Monitor's Role in the Event-Action Loop

The `Monitor` ties events, actions, and fallbacks together:

1. **Event evaluation**: The `Monitor` subscribes to all topics involved in registered events. On each message, it updates the blackboard and evaluates all events.
2. **Action dispatch**: When an event triggers, the `Monitor` either executes the action directly (for simple callables) or emits an `InternalEvent` that the `Launcher` handles (for lifecycle transitions or process-level actions).
3. **Health monitoring**: The `Monitor` subscribes to each component's `ComponentStatus` topic. When a failure is detected, it triggers the component's fallback chain.
4. **Staleness detection**: `EventBlackboardEntry` tracks message UUIDs and timestamps. The `Monitor` uses these to avoid re-triggering events on stale data and to perform lazy expiration of old entries.
