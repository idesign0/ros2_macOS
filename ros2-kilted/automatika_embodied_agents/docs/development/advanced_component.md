# Advanced: Health Status, Fallbacks & Events

This guide covers the health monitoring, fallback recovery, and event systems available to EmbodiedAgents components. Read [Creating a Custom Component](./custom_component.md) first for the basics.

## Health Status

Every component has a `health_status` object (an instance of `Status` from `ros_sugar.core.status`) that tracks the component's operational state. The status is published on the `{node_name}/status` topic and drives the fallback system.

### Status Levels

| Code | Constant | Meaning |
|---|---|---|
| 0 | `STATUS_HEALTHY` | Running normally |
| 1 | `STATUS_FAILURE_ALGORITHM_LEVEL` | An algorithm or model inference failed |
| 2 | `STATUS_FAILURE_COMPONENT_LEVEL` | The component itself (or a dependency) failed |
| 3 | `STATUS_FAILURE_SYSTEM_LEVEL` | An external dependency failed (e.g. missing topic, unavailable service) |
| 4 | `STATUS_GENERAL_FAILURE` | Unspecified failure |

### Setting Status

Use these methods inside your component to report health:

```python
# Everything is fine
self.health_status.set_healthy()

# Model inference failure -- include the algorithm name(s) for diagnostics
self.health_status.set_fail_algorithm(
    algorithm_names=["GraspNet"]
)

# Component failure -- defaults to self, or specify other component(s)
self.health_status.set_fail_component()
self.health_status.set_fail_component(
    component_names=["vision_node"]
)

# System failure -- an external dependency is unavailable
self.health_status.set_fail_system(
    topic_names=["/camera/image_raw"]
)

# Generic failure (no specific category)
self.health_status.set_failure()
```

### Checking Status

```python
self.health_status.is_healthy          # bool
self.health_status.is_algorithm_fail   # bool
self.health_status.is_component_fail   # bool
self.health_status.is_system_fail      # bool
self.health_status.is_general_fail     # bool
self.health_status.value               # int (0-4)
```

### Where to Update Status

The key principle: **set unhealthy when something goes wrong, set healthy when it recovers.**

#### In `_execution_step()` -- the most common location

```python
def _execution_step(self, **kwargs):
    inference_input = self._create_input()
    if inference_input is None:
        # No data from subscribed topics
        self.health_status.set_fail_system(
            topic_names=["/sensor_topic"]
        )
        return

    result = self._call_inference(inference_input)
    if result is None:
        # Model inference failed -- set_fail_algorithm is also
        # called automatically inside _call_inference()
        return

    # Success -- clear any previous failure
    self.health_status.set_healthy()
    self._publish(result)
```

#### Automatic Status in `ModelComponent`

`ModelComponent._call_inference()` sets health status automatically:

- Calls `set_fail_component()` when no model client is available (neither remote nor local).
- Calls `set_fail_algorithm()` when the model client raises an exception during inference.

You do not need to duplicate these checks in your component.

#### In lifecycle transitions -- handled automatically

The base class resets status to healthy on `configure`, `activate`, and `deactivate`. On `error`, it sets `set_fail_component()`. You generally don't need to touch status in lifecycle methods.

### Status Publishing

For `TIMED` components (the default), status is published automatically at the fallback check rate. For non-timed components (e.g. action servers), publish manually after updating:

```python
self.health_status.set_fail_algorithm(algorithm_names=["MyModel"])
self.health_status_publisher.publish(self.health_status())
```

## Fallbacks

Fallbacks are actions that execute automatically when the health status becomes unhealthy. They provide self-recovery without external intervention.

### Defining Fallbacks

Use the component's setter methods before launching:

```python
from agents.ros import Action

my_component = MyComponent(component_name="my_node")

# On algorithm failure: restart the component (retry up to 3 times)
my_component.on_algorithm_fail(
    action=Action(my_component.restart),
    max_retries=3,
)

# On system failure: just broadcast status (let external monitoring handle it)
my_component.on_system_fail(
    action=Action(my_component.broadcast_status),
    max_retries=None,  # None = unlimited retries
)

# On component failure: try reconfiguring with a new config, then restart
my_component.on_component_fail(
    action=[
        Action(my_component.reconfigure, args=(fallback_config,)),
        Action(my_component.restart),
    ],
    max_retries=2,
)

# Catch-all for any failure type not covered above
my_component.on_fail(
    action=Action(my_component.broadcast_status),
    max_retries=None,
)

# When all fallbacks are exhausted
my_component.on_giveup(
    action=Action(my_component.stop),
    max_retries=1,
)
```

### Fallback Hierarchy

| Method | Triggers on | Priority |
|---|---|---|
| `on_algorithm_fail()` | `set_fail_algorithm()` | Checked first |
| `on_component_fail()` | `set_fail_component()` | Checked second |
| `on_system_fail()` | `set_fail_system()` | Checked third |
| `on_fail()` | Any failure without a specific handler | Catch-all |
| `on_giveup()` | All fallbacks exhausted | Last resort |

The fallback check runs on a timer (default 100 Hz) while the component is active. When `health_status` is not healthy, the system:

1. Looks for a handler matching the specific failure type.
2. Falls back to `on_fail()` if no specific handler exists.
3. Retries the current action up to `max_retries`.
4. For action lists, moves to the next action when retries are exhausted.
5. Calls `on_giveup()` when everything is exhausted.
6. If the action returns `True`, status resets to healthy automatically.

### Action Lists (Sequential Fallbacks)

When you pass a list of actions, they execute in order. Each action is retried `max_retries` times before moving to the next:

```python
# Try broadcast first (2 retries), then restart (2 retries), then reconfigure (2 retries)
my_component.on_algorithm_fail(
    action=[
        Action(my_component.broadcast_status),
        Action(my_component.restart),
        Action(my_component.reconfigure, args=(safe_config,)),
    ],
    max_retries=2,
)
```

> **Note:** For action lists, `max_retries=None` is automatically converted to `max_retries=1` to prevent getting stuck on the first action forever.

### Built-in Component Actions

These methods are available on every component and can be used as fallback actions:

| Action | Description |
|---|---|
| `start()` | Activate the component (lifecycle transition) |
| `stop()` | Deactivate the component |
| `restart(wait_time=None)` | Stop then start (optional delay between) |
| `reconfigure(new_config, keep_alive=False)` | Apply a new config (optionally while running) |
| `set_param(param_name, new_value, keep_alive=True)` | Change a single parameter |
| `set_params(params_names, new_values, keep_alive=True)` | Change multiple parameters |
| `broadcast_status()` | Publish current status (default fallback) |

### Default Behavior

If you don't configure any fallbacks, the component uses `broadcast_status()` as the default `on_fail` action with unlimited retries. This publishes the failure status so external systems (like a Monitor node) can observe and react.

## Model-Specific Fallbacks

`ModelComponent` provides two additional fallback methods designed for AI workloads. Both are decorated with `@component_fallback`, which validates that the component is in a valid lifecycle state before executing.

### Falling Back to a Local Model

Switch from a remote model client to a built-in local model at runtime:

```python
from agents.ros import Action

llm = LLM(
    inputs=[text_in],
    outputs=[text_out],
    model_client=remote_client,
    config=LLMConfig(enable_local_model=True),
)

# If the remote server goes down, switch to local inference
switch_to_local = Action(method=llm.fallback_to_local)
llm.on_component_fail(action=switch_to_local, max_retries=3)
llm.on_algorithm_fail(action=switch_to_local, max_retries=3)
```

When `fallback_to_local()` executes, it:

1. Enables the local model flag in the config (if not already set).
2. Deploys the local model via `_deploy_local_model()`.
3. Deinitializes the remote model client.
4. Returns `True` on success (which resets health to healthy).

This requires a local model backend to be implemented for the component. Built-in components with local support: `LLM` (llama-cpp), `MLLM` (Moondream2), `SpeechToText` (sherpa-onnx Whisper), `TextToSpeech` (sherpa-onnx Kokoro), `Vision` (DEIM ONNX).

### Hot-Swapping Model Clients

Switch to a different remote model client at runtime using `additional_model_clients`:

```python
from agents.ros import Action
from agents.clients.ollama import OllamaClient
from agents.clients.generic import GenericHTTPClient
from agents.models import OllamaModel, GenericLLM

# Primary client
primary = OllamaClient(OllamaModel(name="llama3", checkpoint="llama3"))

# Backup client
backup = GenericHTTPClient(
    GenericLLM(name="backup_llm", checkpoint="mistral", endpoint="http://backup:8000")
)

llm = LLM(
    inputs=[text_in],
    outputs=[text_out],
    model_client=primary,
)

# Register backup client
llm.additional_model_clients = {"backup": backup}

# On failure, switch to backup
switch_to_backup = Action(
    method=llm.change_model_client,
    args=("backup",),
)
llm.on_algorithm_fail(action=switch_to_backup, max_retries=3)
```

When `change_model_client()` executes, it:

1. Looks up the named client in `additional_model_clients`.
2. Deinitializes the current model client.
3. Sets the new client as active and initializes it.
4. Returns `True` on success.

### Combining Fallback Strategies

Chain model-specific fallbacks with built-in actions for a layered recovery strategy:

```python
llm.additional_model_clients = {"backup": backup_client}

llm.on_algorithm_fail(
    action=[
        # First: try the backup remote model
        Action(llm.change_model_client, args=("backup",)),
        # Second: fall back to local inference
        Action(llm.fallback_to_local),
        # Third: restart the component entirely
        Action(llm.restart),
    ],
    max_retries=2,
)

llm.on_giveup(
    action=Action(llm.stop),
    max_retries=1,
)
```

## Events and Actions

Events allow components to react to data-driven conditions. An `Event` pairs a trigger condition with one or more `Action` callbacks.

### Defining Events

```python
from agents.ros import Event, Action, Topic

# Topic-based: triggers whenever a message arrives
emergency_topic = Topic(name="/emergency", msg_type="Bool")
event = Event(event_condition=emergency_topic)

# Action-based: polls a method at a given rate
event = Event(
    event_condition=Action(my_component.check_battery),
    check_rate=1.0,  # Poll at 1 Hz
)
```

### Event Options

| Parameter | Default | Description |
|---|---|---|
| `on_change` | `False` | Only trigger when the value changes (not on every message) |
| `handle_once` | `False` | Only trigger once during the component's lifetime |
| `keep_event_delay` | `0.0` | Minimum delay (seconds) between consecutive triggers |
| `check_rate` | `None` | Poll rate (Hz) for action-based events |

### Using Events as Component Triggers

Events can be passed directly as the `trigger` parameter to a component. This makes the component's `_execution_step()` fire only when the event condition is met:

```python
from agents.components import MLLM
from agents.ros import Event, Topic

detections_topic = Topic(name="/detections", msg_type="Detections")

# Fire the VLM only when a person is detected
person_detected = Event(
    event_condition=detections_topic,
    on_change=True,
    keep_event_delay=5.0,
)

vlm = MLLM(
    inputs=[camera, detections_topic],
    outputs=[description],
    model_client=client,
    trigger=person_detected,
)
```

### Wiring Events to Actions at Launch

Events and actions are connected at the Launcher level, not inside individual components. This keeps components decoupled:

```python
from agents.ros import Launcher, Event, Action, Topic

launcher = Launcher()

# Define event + response actions
emergency_event = Event(event_condition=emergency_topic)
stop_action = Action(controller.stop)

launcher.add_pkg(
    components=[llm, vision],
    events_actions={emergency_event: [stop_action]},
)

launcher.bringup()
```

## Putting It All Together

Here is a complete example showing an LLM component with health-aware fallbacks and event-driven activation:

```python
from agents.components import LLM
from agents.config import LLMConfig
from agents.clients.ollama import OllamaClient
from agents.clients.generic import GenericHTTPClient
from agents.models import OllamaModel, GenericLLM
from agents.ros import (
    Topic,
    Event,
    Action,
    Launcher,
    String,
)


# --- Topics ---
user_input = Topic(name="/user_input", msg_type="String")
llm_output = Topic(name="/llm_output", msg_type="StreamingString")

# --- Clients ---
primary_client = OllamaClient(
    OllamaModel(name="llama3", checkpoint="llama3")
)
backup_client = GenericHTTPClient(
    GenericLLM(name="backup", checkpoint="mistral", endpoint="http://backup:8000")
)

# --- Component ---
llm = LLM(
    inputs=[user_input],
    outputs=[llm_output],
    model_client=primary_client,
    config=LLMConfig(
        enable_local_model=True,
        stream=True,
    ),
    trigger=user_input,
    component_name="llm_node",
)

# --- Fallbacks ---
llm.additional_model_clients = {"backup": backup_client}

# Algorithm failure: try backup, then local, then restart
llm.on_algorithm_fail(
    action=[
        Action(llm.change_model_client, args=("backup",)),
        Action(llm.fallback_to_local),
        Action(llm.restart),
    ],
    max_retries=2,
)

# System failure (e.g. missing input topic): broadcast and wait
llm.on_system_fail(
    action=Action(llm.broadcast_status),
    max_retries=None,
)

# All fallbacks exhausted: stop the component
llm.on_giveup(
    action=Action(llm.stop),
    max_retries=1,
)

# --- Event: stop LLM when emergency is triggered ---
emergency = Topic(name="/emergency_stop", msg_type="Bool")
emergency_event = Event(event_condition=emergency)
stop_llm = Action(llm.stop)

# --- Launch ---
launcher = Launcher()
launcher.add_pkg(
    components=[llm],
    events_actions={emergency_event: [stop_llm]},
)
launcher.bringup()
```

### Health Status Flow

```
_execution_step()
    │
    ├── Missing input? ──▶ set_fail_system(topic_names=[...])
    │                           │
    │                           ▼
    │                     fallback timer checks health_status
    │                           │
    │                           ├── on_system_fail defined? ──▶ execute action
    │                           └── no? ──▶ on_fail (broadcast_status)
    │
    ├── _call_inference() fails?
    │       │
    │       ├── No client ──▶ set_fail_component()
    │       │                       │
    │       │                       ▼
    │       │                 on_component_fail ──▶ execute action
    │       │
    │       └── Inference error ──▶ set_fail_algorithm()
    │                                   │
    │                                   ▼
    │                             on_algorithm_fail
    │                                   │
    │                                   ├── change_model_client ──▶ success? ──▶ set_healthy()
    │                                   ├── fallback_to_local ──▶ success? ──▶ set_healthy()
    │                                   ├── restart ──▶ success? ──▶ set_healthy()
    │                                   └── retries exhausted ──▶ on_giveup
    │
    └── Success ──▶ set_healthy()
```
