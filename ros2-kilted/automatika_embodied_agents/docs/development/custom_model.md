# Creating a Custom Model Wrapper

Model wrappers in EmbodiedAgents are data classes that describe an ML model's identity and initialization parameters. They are passed to `ModelClient` instances, which use them to load and configure the model on the serving platform. This guide covers the `attrs` pattern, required fields, and how to wrap a new model.

## The `attrs` `@define` Pattern

All model classes use the [attrs](https://www.attrs.org/) library:

```python
from attrs import define, field
from agents.ros import BaseAttrs

@define(kw_only=True)
class MyModel(Model):
    ...
```

Key conventions:

- **Always use `@define(kw_only=True)`**: this forces keyword-only construction, preventing positional argument bugs.
- **Inherit from `Model`** (defined in `agents.models`), which itself inherits from `BaseAttrs`.
- **Use `field()`** for every attribute, with `default`, `validator`, and `converter` as needed.
- **Validators** from `base_validators` (imported from `agents.ros`) provide common checks: `gt()`, `in_range()`, `in_()`.

## Required Fields

The `Model` base class defines three fields that every model must have:

```python
@define(kw_only=True)
class Model(BaseAttrs):
    name: str                              # Arbitrary identifier
    checkpoint: str                        # Model checkpoint / HuggingFace repo ID
    init_timeout: Optional[int] = field(default=None)  # Timeout for initialization
```

- `name`: A user-chosen identifier. Used for logging and to key the model in the client.
- `checkpoint`: The model identifier on the serving platform (e.g., an Ollama tag, a HuggingFace repo, or a custom endpoint path).
- `init_timeout`: Optional timeout for model loading on the server side.

## Implementing `_get_init_params()`

Every model subclass must override `_get_init_params()` to return a dictionary of parameters sent to the serving platform during initialization:

```python
def _get_init_params(self) -> Dict:
    return {
        "checkpoint": self.checkpoint,
        "my_custom_param": self.my_custom_param,
    }
```

This dict is stored in the client as `self.model_init_params` and included in serialization. The serving platform uses these parameters to load the model correctly.

## Existing Model Hierarchy

```
Model
  ├── LLM (adds quantization)
  │     ├── OllamaModel (adds port, options)
  │     ├── TransformersLLM (HuggingFace checkpoint)
  │     │     └── TransformersMLLM
  │     └── RoboBrain2
  ├── GenericLLM (OpenAI-compatible)
  │     └── GenericMLLM
  ├── GenericTTS
  ├── GenericSTT
  ├── Whisper (adds compute_type)
  ├── SpeechT5 (adds voice)
  ├── Bark (adds voice)
  ├── MeloTTS (adds language, speaker_id)
  ├── VisionModel (adds tracking, TensorRT options)
  └── LeRobotPolicy (adds policy_type, features, actions)
```

## Example: Wrapping a New Platform

Suppose you want to integrate a hypothetical "TurboServe" platform that serves LLMs and requires a `precision` and `max_batch_size` parameter.

### Step 1: Define the Model

```python
from typing import Optional, Dict, Any
from attrs import define, field
from agents.models import Model
from agents.ros import base_validators


@define(kw_only=True)
class TurboServeModel(Model):
    """Model configuration for the TurboServe inference platform.

    :param name: Arbitrary model name.
    :type name: str
    :param checkpoint: Model identifier on TurboServe.
    :type checkpoint: str
    :param precision: Inference precision. One of "fp16", "fp32", "int8".
    :type precision: str
    :param max_batch_size: Maximum batch size for inference.
    :type max_batch_size: int
    :param init_timeout: Timeout for model loading in seconds.
    :type init_timeout: int, optional
    """

    precision: str = field(
        default="fp16",
        validator=base_validators.in_(["fp16", "fp32", "int8"]),
    )
    max_batch_size: int = field(
        default=1,
        validator=base_validators.gt(0),
    )

    def _get_init_params(self) -> Dict:
        return {
            "checkpoint": self.checkpoint,
            "precision": self.precision,
            "max_batch_size": self.max_batch_size,
        }
```

### Step 2: Use with a Client

The model is passed to a `ModelClient` subclass. The client reads `self.model_init_params` (set from `_get_init_params()`) to configure the serving platform:

```python
model = TurboServeModel(
    name="turbo_llama",
    checkpoint="meta-llama/Llama-3.1-8B",
    precision="fp16",
    max_batch_size=4,
)

client = TurboServeClient(model, host="gpu-server.local", port=9000)
```

### Step 3: Adding Validation with `__attrs_post_init__`

For complex validation or derived fields, use `__attrs_post_init__()`:

```python
@define(kw_only=True)
class TurboServeModel(Model):
    precision: str = field(default="fp16")
    max_batch_size: int = field(default=1)
    _effective_memory: Optional[int] = field(default=None)

    def __attrs_post_init__(self):
        # Compute derived field based on precision
        memory_map = {"fp32": 4, "fp16": 2, "int8": 1}
        self._effective_memory = memory_map.get(self.precision, 2)

    def _get_init_params(self) -> Dict:
        return {
            "checkpoint": self.checkpoint,
            "precision": self.precision,
            "max_batch_size": self.max_batch_size,
            "effective_memory": self._effective_memory,
        }
```

### Step 4: Options Dict Pattern

For platforms with many tunable parameters, use a validated `options` dict (following the `OllamaModel` pattern):

```python
@define(kw_only=True)
class TurboServeModel(Model):
    precision: str = field(default="fp16")
    options: Optional[Dict[str, Any]] = field(default=None)

    @options.validator
    def _validate_options(self, _, value):
        if value is None:
            return
        allowed_keys = {
            "temperature": float,
            "top_p": float,
            "max_tokens": int,
        }
        for key, val in value.items():
            if key not in allowed_keys:
                raise ValueError(f"Invalid option: {key}")
            if not isinstance(val, allowed_keys[key]):
                raise TypeError(
                    f"Option '{key}' must be {allowed_keys[key].__name__}"
                )

    def _get_init_params(self) -> Dict:
        return {
            "checkpoint": self.checkpoint,
            "precision": self.precision,
            "options": self.options,
        }
```
