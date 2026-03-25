# Adding a New Modality End-to-End

This guide walks through adding a completely new modality to EmbodiedAgents -- from ROS message definition through to a working component. Read [Custom ROS Message Types](./messages.md), [Creating a Custom Component](./custom_component.md), and [Creating a Custom Model Client](./custom_client.md) first; this guide ties all three layers together.

## Overview

A "modality" in EmbodiedAgents is a complete data pipeline: a ROS message carries the data, a `SupportedType` wrapper bridges it to Python, a callback deserializes incoming messages, and a component produces or consumes the data via a model client.

Adding a new modality touches these files:

| Layer | File(s) | What you add |
|---|---|---|
| ROS message | `msg/YourType.msg`, `CMakeLists.txt` | Message definition, build registration |
| Callback | `agents/callbacks.py` | Deserialization from ROS to Python |
| SupportedType | `agents/ros.py` | Type wrapper, convert method, registration |
| UI (optional) | `agents/ui_elements.py` | Visualization rendering |
| Config | `agents/config.py` | Component configuration class |
| Component | `agents/components/your_component.py` | Processing logic |
| Component registration | `agents/components/__init__.py` | Export |

The rest of this guide walks through each layer using a concrete example: **Haptic Sensing** -- a component that processes tactile sensor data through an ML model and publishes grasp quality predictions.

## Step 1: Define the ROS Message

Create a `.msg` file in the `automatika_embodied_agents` package:

```
# msg/HapticReading.msg
float32[] pressures
float32[] temperatures
uint32 sensor_id
std_msgs/Header header
```

Register it in `CMakeLists.txt`:

```cmake
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/HapticReading.msg"
  # ... existing messages ...
  DEPENDENCIES std_msgs sensor_msgs
)
```

Build the package:

```bash
cd <workspace_root>
colcon build --packages-select automatika_embodied_agents
source install/setup.bash
```

After building, the message is available as `automatika_embodied_agents.msg.HapticReading`.

## Step 2: Create the Callback

Add a callback class to `agents/callbacks.py`. The callback converts the raw ROS message into Python data that components work with:

```python
import numpy as np
from ros_sugar.io.callbacks import GenericCallback


class HapticReadingCallback(GenericCallback):
    """Callback for HapticReading messages."""

    def _get_output(self, **_):
        """Convert HapticReading ROS message to a numpy array."""
        if self.msg is None:
            return None

        pressures = np.array(self.msg.pressures, dtype=np.float32)
        temperatures = np.array(self.msg.temperatures, dtype=np.float32)

        # Stack into a single feature array: shape (2, N_sensors)
        return np.stack([pressures, temperatures], axis=0)
```

Key points:

- Inherit from `GenericCallback` (or `TextCallback` for string data).
- Implement `_get_output(self, **_)` -- this is what `callback.get_output()` returns to the component.
- Handle the `self.msg is None` case (no message received yet).
- Optionally implement `_get_ui_content(self, **_)` to return rendered HTML or JPEG-encoded bytes for UI visualization.

### Handling Fixed Inputs

If your modality should support `FixedInput` (static data from a file), check for it in the callback:

```python
def _get_output(self, **_):
    # FixedInput case: self.msg is a string path
    if isinstance(self.msg, str):
        return np.load(self.msg)

    if self.msg is None:
        return None

    # Normal ROS message case
    return np.stack([
        np.array(self.msg.pressures, dtype=np.float32),
        np.array(self.msg.temperatures, dtype=np.float32),
    ], axis=0)
```

## Step 3: Create the SupportedType Wrapper

Add the wrapper class in `agents/ros.py`. This bridges the ROS message, the callback, and the publish-side conversion:

```python
from automatika_embodied_agents.msg import HapticReading as ROSHapticReading
from .callbacks import HapticReadingCallback


class HapticReading(SupportedType):
    """Wraps automatika_embodied_agents/msg/HapticReading."""

    _ros_type = ROSHapticReading
    callback = HapticReadingCallback

    @classmethod
    def convert(cls, output, **_):
        """Convert a numpy array back to a HapticReading ROS message."""
        msg = ROSHapticReading()
        msg.pressures = output[0].tolist()
        msg.temperatures = output[1].tolist()
        return msg
```

Then register it by adding to the `agent_types` list at the bottom of the same file:

```python
agent_types = [
    StreamingString,
    Video,
    Detections,
    # ... existing types ...
    HapticReading,  # <-- add here
]

add_additional_datatypes(agent_types)
```

And export it in `__all__`:

```python
__all__ = [
    # ... existing exports ...
    "HapticReading",
]
```

### Lazy-Loading External Message Types

If your ROS message comes from an external package that might not be installed, use `get_ros_type()` instead of `_ros_type`:

```python
class HapticReading(SupportedType):
    callback = HapticReadingCallback

    @classmethod
    def get_ros_type(cls):
        from some_external_pkg.msg import HapticReading as ROSHapticReading
        return ROSHapticReading

    @classmethod
    def convert(cls, output, **_):
        ros_type = cls.get_ros_type()
        msg = ros_type()
        msg.pressures = output[0].tolist()
        return msg
```

See `RGBD` in `agents/ros.py` for a real example of this pattern.

## Step 4: Add UI Visualization (Optional)

If your modality has meaningful visual output, register a UI element in `agents/ui_elements.py`:

```python
from .ros import HapticReading


def _log_haptic_element(logging_card, output, data_src: str):
    """Render haptic data as a text summary in the logging card."""
    summary = f"Sensor pressures: mean={output[0].mean():.2f}, max={output[0].max():.2f}"
    return _log_text_element(logging_card, summary, data_src)


OUTPUT_ELEMENTS[HapticReading] = _log_haptic_element
```

## Step 5: Define the Component Configuration

Add a config class to `agents/config.py`:

```python
from attrs import define, field


@define(kw_only=True)
class HapticConfig(ModelComponentConfig):
    """Configuration for the Haptic Sensing component."""

    pressure_threshold: float = field(
        default=0.5,
        validator=base_validators.in_range(min_value=0.0, max_value=1.0),
    )
    window_size: int = field(default=10, validator=base_validators.gt(0))
    enable_local_model: bool = field(default=False)
    device_local_model: Literal["cpu", "cuda"] = field(default="cpu")

    def _get_inference_params(self):
        return {
            "pressure_threshold": self.pressure_threshold,
            "window_size": self.window_size,
        }
```

Key points:

- Always use `@define(kw_only=True)`.
- Extend `ModelComponentConfig` for model-based components, or `BaseComponentConfig` for pure data-processing components.
- Implement `_get_inference_params()` to return the dict passed to the model at inference time.
- Use `base_validators` for field validation.
- Add `enable_local_model` and `device_local_model` fields if you plan to support local inference.

## Step 6: Build the Component

Create `agents/components/haptic.py`:

```python
from typing import Any, Dict, List, Optional, Sequence, Type, Union
from types import NoneType

import numpy as np
from agents.components.model_component import ModelComponent
from agents.clients.model_base import ModelClient
from agents.config import HapticConfig
from agents.ros import (
    Topic,
    FixedInput,
    HapticReading,
    String,
    SupportedType,
    Event,
)


class Haptic(ModelComponent):
    """A component that processes tactile sensor data through an ML model.

    This capability enables grasp quality estimation, texture
    classification, and contact event detection.
    """

    def __init__(
        self,
        inputs: Optional[Sequence[Union[Topic, FixedInput]]] = None,
        outputs: Optional[Sequence[Topic]] = None,
        model_client: Optional[ModelClient] = None,
        config: Optional[HapticConfig] = None,
        trigger: Union[Topic, List[Topic], float, Event, NoneType] = 1.0,
        component_name: str = "haptic",
        **kwargs,
    ):
        # Declare allowed I/O before super().__init__
        self.allowed_inputs = {
            "Required": [HapticReading],
        }
        self.allowed_outputs = {
            "Required": [String],            # Grasp quality prediction
            "Optional": [HapticReading],     # Processed/filtered readings
        }
        self.handled_outputs: List[Type[SupportedType]] = [String]

        if not config:
            config = HapticConfig()

        super().__init__(
            inputs=inputs,
            outputs=outputs,
            model_client=model_client,
            config=config,
            trigger=trigger,
            component_name=component_name,
            **kwargs,
        )

        self._reading_buffer: List[np.ndarray] = []

    def _create_input(self, *args, **kwargs) -> Optional[Dict[str, Any]]:
        """Assemble inference input from buffered haptic readings."""
        reading = None

        for cb in self.trig_callbacks.values():
            reading = cb.get_output()
        if reading is None:
            for cb in self.callbacks.values():
                reading = cb.get_output()

        if reading is None:
            self.get_logger().warning("No haptic reading received yet")
            return None

        self._reading_buffer.append(reading)
        if len(self._reading_buffer) > self.config.window_size:
            self._reading_buffer = self._reading_buffer[-self.config.window_size:]

        # Stack readings into a time-series array
        window = np.stack(self._reading_buffer, axis=0)

        return {
            "readings": window,
            **self.inference_params,
        }

    def _execution_step(self, **kwargs):
        """Main loop: buffer readings, run inference, publish prediction."""
        inference_input = self._create_input()
        if inference_input is None:
            return

        result = self._call_inference(inference_input)
        if result is None:
            return

        self._publish(result)

    def _warmup(self, *args, **kwargs):
        """Send dummy data to warm up the model."""
        dummy = np.zeros((self.config.window_size, 2, 16), dtype=np.float32)
        self._call_inference({"readings": dummy, **self.inference_params})

    def _handle_websocket_streaming(self) -> Optional[Any]:
        """Not used -- haptic inference is not a streaming task."""
        pass
```

### Register the Component

Export it in `agents/components/__init__.py`:

```python
from .haptic import Haptic

__all__ = [
    # ... existing exports ...
    "Haptic",
]
```

## Step 7: Use It

```python
from agents.components import Haptic
from agents.config import HapticConfig
from agents.clients.roboml import RoboMLHTTPClient
from agents.models import RoboMLModel
from agents.ros import Topic, Launcher

# Topics
tactile = Topic(name="/tactile_sensor", msg_type="HapticReading")
grasp_quality = Topic(name="/grasp_quality", msg_type="String")

# Model
model = RoboMLModel(name="grasp_net", checkpoint="grasp-quality-v1")
client = RoboMLHTTPClient(model)

# Component
haptic = Haptic(
    inputs=[tactile],
    outputs=[grasp_quality],
    model_client=client,
    trigger=tactile,
    config=HapticConfig(pressure_threshold=0.3, window_size=20),
)

# Launch
launcher = Launcher()
launcher.add_pkg(components=[haptic])
launcher.bringup()
```

## Checklist

When adding a new modality, verify:

- [ ] `.msg` file defined and registered in `CMakeLists.txt`
- [ ] Package builds cleanly with `colcon build`
- [ ] Callback class in `agents/callbacks.py` with `_get_output()` implemented
- [ ] `SupportedType` wrapper in `agents/ros.py` with `_ros_type`, `callback`, and `convert()`
- [ ] Type added to `agent_types` list and `__all__` in `agents/ros.py`
- [ ] Config class in `agents/config.py` with `_get_inference_params()`
- [ ] Component in `agents/components/` with all abstract methods implemented
- [ ] Component exported in `agents/components/__init__.py`
- [ ] UI element registered in `agents/ui_elements.py` (if applicable)
