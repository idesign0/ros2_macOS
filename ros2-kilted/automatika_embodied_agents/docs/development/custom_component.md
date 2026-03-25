# Creating a Custom Component

This guide walks through building a new EmbodiedAgents component from scratch.

## When to Subclass What

Choose your base class based on whether your component needs a model client:

| Base Class | Use When |
|---|---|
| `Component` | Your component performs pure data processing, transformations, or routing without calling an ML model. |
| `ModelComponent` | Your component wraps an ML model and needs inference via a `ModelClient`. |

Most custom components will subclass `ModelComponent`.

Think of components as **capabilities**. Each built-in component represents a distinct capability: Vision sees, SpeechToText hears, TextToSpeech speaks, LLM reasons, VLM understands images. A good custom component adds a new capability that isn't already covered.

## Defining Allowed Inputs and Outputs

Every component must declare what topic types it accepts. These are set as instance attributes before the `super().__init__()` call:

```python
from agents.ros import SupportedType, String, Image, Audio

class DepthEstimator(ModelComponent):
    def __init__(self, ...):
        self.allowed_inputs = {
            "Required": [Image],           # Must have at least one Image input
        }
        self.allowed_outputs = {
            "Required": [Image],           # Outputs a depth map as an Image
        }
        super().__init__(...)
```

### Cardinality Rules

- Each entry in the `"Required"` list must have at least one matching topic in the provided inputs/outputs.
- A nested list like `[String, Audio]` means "at least one topic of type `String` **or** `Audio`."
- `"Optional"` entries are accepted but not enforced.
- Subtypes are matched: if `StreamingString` is a subclass of the allowed type, it passes validation.

## Implementing `_execution_step()`

This is the core logic of your component. For `ModelComponent` subclasses, you must also implement `_create_input()`, `_warmup()`, and `_handle_websocket_streaming()`.

```python
from abc import abstractmethod

class DepthEstimator(ModelComponent):

    @abstractmethod
    def _execution_step(self, **kwargs):
        """Called each time the component is triggered."""
        ...

    @abstractmethod
    def _create_input(self, *args, **kwargs):
        """Assemble the inference input dict from callback data."""
        ...

    @abstractmethod
    def _warmup(self, *args, **kwargs):
        """Optional warmup call during configure phase."""
        ...

    @abstractmethod
    def _handle_websocket_streaming(self):
        """Handle streaming responses from WebSocket clients."""
        ...
```

For `Component` subclasses (no model client), you only need to implement `_execution_step()`.

## Configuration Class Pattern

Define a config class using `attrs`:

```python
from attrs import define, field
from agents.config import ModelComponentConfig
from agents.ros import base_validators

@define(kw_only=True)
class DepthEstimatorConfig(ModelComponentConfig):
    """Configuration for the Depth Estimator component."""

    input_height: int = field(default=518, validator=base_validators.gt(0))
    input_width: int = field(default=518, validator=base_validators.gt(0))
    max_depth: float = field(default=20.0, validator=base_validators.gt(0.0))

    def _get_inference_params(self):
        return {
            "input_height": self.input_height,
            "input_width": self.input_width,
        }
```

Key points:

- Always use `@define(kw_only=True)`.
- Extend `ModelComponentConfig` (which itself extends `BaseComponentConfig`).
- Implement `_get_inference_params()` to return the dict passed to the model at inference time.
- Use `base_validators` for field validation (`gt`, `in_range`, `in_`).

### Adding Local Model Support

If your custom component should support local inference (without a remote model client), add the standard local model fields to your config:

```python
@define(kw_only=True)
class MyConfig(ModelComponentConfig):
    enable_local_model: bool = field(default=False)
    device_local_model: Literal["cpu", "cuda"] = field(default="cuda")
    ncpu_local_model: int = field(default=1)
    local_model_path: Optional[str] = field(default="your/default-model")
```

Then implement `_deploy_local_model()` in your component to instantiate the appropriate local wrapper. See `agents/components/llm.py` for a reference implementation.

## Wiring the Trigger

The trigger determines when your component's `_execution_step()` fires. Set it in the constructor:

```python
# Trigger on a specific input topic
depth = DepthEstimator(
    inputs=[camera],
    outputs=[depth_map],
    model_client=my_client,
    trigger=camera,        # fires when a new frame arrives
)

# Trigger on a timer (2 Hz)
depth = DepthEstimator(
    ...,
    trigger=2.0,           # fires twice per second
)

# Trigger on an external event
from agents.ros import Event
my_event = Event(name="estimate_depth")
depth = DepthEstimator(
    ...,
    trigger=my_event,
)
```

When a `Topic` is used as trigger, it must be one of the component's inputs. Internally, the topic's callback is moved from `self.callbacks` to `self.trig_callbacks`, and `_execution_step()` is wired as a post-callback.

## Complete Skeleton: A Depth Estimation Component

Below is a complete, working skeleton for a component that takes a camera image, sends it to a depth estimation model, and publishes the depth map. This represents a distinct perception capability -- monocular depth estimation -- that is not covered by the built-in Vision (detection), VLM (visual Q&A), or other components.

```python
from typing import Any, Dict, List, Optional, Sequence, Type, Union
from types import NoneType

import numpy as np
from attrs import define, field
from agents.components.model_component import ModelComponent
from agents.clients.model_base import ModelClient
from agents.config import ModelComponentConfig
from agents.ros import (
    Topic,
    FixedInput,
    Image,
    SupportedType,
    Event,
    base_validators,
)


# --- Config ---
@define(kw_only=True)
class DepthEstimatorConfig(ModelComponentConfig):
    """Configuration for the Depth Estimator component."""

    input_height: int = field(default=518, validator=base_validators.gt(0))
    input_width: int = field(default=518, validator=base_validators.gt(0))
    max_depth: float = field(default=20.0, validator=base_validators.gt(0.0))

    def _get_inference_params(self) -> Dict:
        return {
            "input_height": self.input_height,
            "input_width": self.input_width,
        }


# --- Component ---
class DepthEstimator(ModelComponent):
    """A component that estimates depth from monocular camera images.

    This capability enables spatial understanding for navigation,
    obstacle avoidance, and manipulation tasks.
    """

    def __init__(
        self,
        inputs: Optional[Sequence[Union[Topic, FixedInput]]] = None,
        outputs: Optional[Sequence[Topic]] = None,
        model_client: Optional[ModelClient] = None,
        config: Optional[DepthEstimatorConfig] = None,
        trigger: Union[Topic, List[Topic], float, Event, NoneType] = 1.0,
        component_name: str = "depth_estimator",
        **kwargs,
    ):
        # Declare allowed I/O before super().__init__
        self.allowed_inputs = {
            "Required": [Image],
        }
        self.handled_outputs: List[Type[SupportedType]] = [Image]

        if not config:
            config = DepthEstimatorConfig()

        super().__init__(
            inputs=inputs,
            outputs=outputs,
            model_client=model_client,
            config=config,
            trigger=trigger,
            component_name=component_name,
            **kwargs,
        )

    def _create_input(self, *args, **kwargs) -> Optional[Dict[str, Any]]:
        """Assemble inference input from the latest camera frame."""
        image = None

        # Read from trigger callback
        for cb in self.trig_callbacks.values():
            image = cb.get_output()

        # Fall back to regular callbacks
        if image is None:
            for cb in self.callbacks.values():
                image = cb.get_output()

        if image is None:
            self.get_logger().warning("No image received yet")
            return None

        return {
            "images": [image],
            **self.inference_params,
        }

    def _execution_step(self, **kwargs):
        """Main processing loop: receive image, estimate depth, publish."""
        inference_input = self._create_input()
        if inference_input is None:
            return

        result = self._call_inference(inference_input)
        if result is None:
            return

        self._publish(result)

    def _warmup(self, *args, **kwargs):
        """Send a dummy image to warm up the model."""
        dummy = np.zeros(
            (self.config.input_height, self.config.input_width, 3), dtype=np.uint8
        )
        self._call_inference({"images": [dummy], **self.inference_params})

    def _handle_websocket_streaming(self) -> Optional[Any]:
        """Not used -- depth estimation is not a streaming task."""
        pass
```

### Usage

```python
from agents.clients.ollama import OllamaClient
from agents.models import OllamaModel
from agents.ros import Topic, Launcher

camera = Topic(name="camera", msg_type="Image")
depth_map = Topic(name="depth", msg_type="Image")

model = OllamaModel(name="depth_model", checkpoint="depth-anything-v2")
client = OllamaClient(model)

depth = DepthEstimator(
    inputs=[camera],
    outputs=[depth_map],
    model_client=client,
    trigger=camera,
    config=DepthEstimatorConfig(max_depth=10.0),
)

launcher = Launcher()
launcher.add_pkg(components=[depth])
launcher.bringup()
```
