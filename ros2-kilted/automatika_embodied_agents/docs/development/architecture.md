# Architecture Overview

This document describes the internal architecture of EmbodiedAgents for developers who want to extend the framework, contribute new components, or understand how the pieces fit together.

## Component Hierarchy

Every processing unit in EmbodiedAgents is a **component**. Components form a strict inheritance chain:

```
BaseComponent (ros_sugar)
  └── Component (agents.components.component_base)
        └── ModelComponent (agents.components.model_component)
              ├── LLM
              ├── VLM / MLLM
              ├── VLA
              ├── Vision
              ├── SpeechToText
              ├── TextToSpeech
              ├── SemanticRouter
              ├── MapEncoding
              └── VideoMessageMaker
```

### BaseComponent

`BaseComponent` is provided by [Sugarcoat](https://github.com/automatika-robotics/sugarcoat) and wraps a ROS2 Lifecycle Node. It manages the node lifecycle (configure, activate, deactivate, shutdown), subscriber/publisher creation, and the multiprocess execution model. You should never subclass `BaseComponent` directly.

### Component

`Component` (defined in `agents.components.component_base`) adds:

- **Input/output validation** via `allowed_inputs` and `allowed_outputs` dictionaries.
- **Trigger system** for deciding when `_execution_step()` fires.
- **Event/Action wiring** through `custom_on_configure()` and `activate_all_triggers()`.

### ModelComponent

`ModelComponent` (defined in `agents.components.model_component`) extends `Component` with:

- A `model_client` slot (`ModelClient` instance) initialized during the configure lifecycle phase.
- Support for `additional_model_clients` and hot-swapping via `change_model_client()`.
- `_call_inference()` dispatching to HTTP or WebSocket clients.
- Output topic validation against `handled_outputs`.
- Warmup logic.
- Streaming support via a fast timer (`_handle_websocket_streaming()`).

All specialized components (`LLM`, `VLM`, `Vision`, etc.) subclass `ModelComponent`.

## The `_execution_step()` Pattern

Every concrete component must implement `_execution_step(**kwargs)`. This is the core processing callback that runs each time the component is triggered. The general flow inside a `ModelComponent._execution_step()` is:

1. **Gather inputs** -- read the latest data from all input callbacks.
2. **Create inference input** -- call `_create_input()` to assemble a dict suitable for the model client.
3. **Run inference** -- call `_call_inference(inference_input)`, which delegates to the `ModelClient`.
4. **Publish results** -- call `_publish(result)` to send output to all registered publishers.

```python
def _execution_step(self, **kwargs):
    # 1. Read inputs from callbacks
    text = self.callbacks["text0"].get_output()

    # 2. Assemble inference dict
    inference_input = self._create_input(text)
    if inference_input is None:
        return

    # 3. Call the model
    result = self._call_inference(inference_input)
    if result is None:
        return

    # 4. Publish
    self._publish(result)
```

For non-model components (subclassing `Component` directly), `_execution_step()` performs custom logic without calling a model client.

## Input/Output Validation

Components declare what topic types they accept using two class-level dictionaries:

```python
self.allowed_inputs = {
    "Required": [Image, [String, Audio]],
    "Optional": [CompressedImage],
}
self.allowed_outputs = {
    "Required": [String],
}
```

- **Required**: Each entry must have at least one matching topic. A list entry like `[String, Audio]` means "one of these types."
- **Optional**: Accepted but not mandatory.

Validation runs in `Component.__init__()` via `_validate_topics()`. It checks that every provided topic's `msg_type` is a subclass of at least one allowed type, and that all required types are covered.

## Trigger System

The `trigger` parameter controls when `_execution_step()` fires:

| Trigger Type | Value | Behavior |
|---|---|---|
| **Timed** | `float` (e.g., `1.0`) | Runs at a fixed frequency (Hz). Sets `ComponentRunType.TIMED`. |
| **Topic** | `Topic` instance | Fires when a message arrives on that topic. The topic must be one of the component's inputs. Sets `ComponentRunType.EVENT`. |
| **Multi-topic** | `List[Topic]` | Fires when any of the listed topics receives a message. |
| **Event** | `Event` instance | Fires when an external event is raised. Wired via an `Action` in `custom_on_configure()`. |
| **None** | `None` | Only valid for `ACTION_SERVER` or `SERVER` run types (e.g., VLA). |

When a `Topic` trigger is set, the topic's callback is moved from `self.callbacks` to `self.trig_callbacks`. The trigger callback's `on_callback_execute()` is wired to call `_execution_step()` in `activate_all_triggers()`.

## Streaming

Components that support streaming (LLM, TextToSpeech) use WebSocket-based model clients. The flow:

1. `ModelComponent.custom_on_configure()` detects `config.stream == True` with a `RoboMLWSClient`.
2. A fast timer (1ms period) is created calling `_handle_websocket_streaming()`.
3. Inference requests go into `self.req_queue`; responses come back via `self.resp_queue`.
4. The child component's `_handle_websocket_streaming()` reads partial results from the queue and publishes them incrementally (e.g., token-by-token for LLM, audio chunks for TTS).

For HTTP clients, streaming is handled at the client level using generator-based responses.

## Configuration Chain

All configuration uses the `attrs` library with the `@define` decorator:

```
BaseComponentConfig (ros_sugar)
  └── ModelComponentConfig (agents.config)
        ├── LLMConfig
        │     └── MLLMConfig (aliased as VLMConfig)
        ├── VLAConfig
        ├── VisionConfig
        ├── SpeechToTextConfig
        ├── TextToSpeechConfig
        ├── SemanticRouterConfig
        └── VideoMessageMakerConfig
  └── MapConfig (extends BaseComponentConfig directly)
```

Each config class:

- Uses `@define(kw_only=True)` for explicit, keyword-only construction.
- Declares fields with `field()`, including defaults, validators (from `base_validators`), and converters.
- Implements `_get_inference_params() -> Dict` to extract the subset of parameters passed to the model client at inference time.

The config is deep-copied at component init so that multiple component instances sharing the same config class do not interfere with each other.

## Model / Client / Component Relationship

The three-layer pattern is central to the architecture:

```
Model (data class)  -->  ModelClient (connection logic)  -->  ModelComponent (ROS node)
```

- **Model** (`agents.models.Model`): An `attrs` `@define` class holding model metadata (name, checkpoint, platform-specific options). Its `_get_init_params()` returns a dict sent to the serving platform.
- **ModelClient** (`agents.clients.model_base.ModelClient`): Manages the connection to a model serving platform. Implements `_check_connection()`, `_initialize()`, `_inference()`, `_deinitialize()`. Must be serializable for multiprocess execution.
- **ModelComponent**: Holds a `ModelClient` instance, calls it during `_execution_step()`, and manages the ROS lifecycle around it.

This separation means the same model can be served by different clients (Ollama, RoboML, GenericHTTP), and the same client can be used across different component types.

## Local Model Deployment

Components that subclass `ModelComponent` can optionally run without a remote model client by enabling a built-in local model. This is controlled via `enable_local_model=True` in the component's config.

### How It Works

When `enable_local_model` is set, the component's `custom_on_configure()` calls `_deploy_local_model()`, which instantiates a lightweight local inference wrapper. The `_call_inference()` dispatcher in `ModelComponent` automatically routes to the local model when no `model_client` is set:

```
ModelComponent._call_inference()
  ├── model_client (RoboML, Ollama, GenericHTTP, ...)
  └── local_model  (LocalLLM, LocalVLM, LocalSTT, LocalTTS)
```

### Runtime Backends

Each component type uses a runtime optimized for edge deployment:

| Component | Backend | Package | Default Model |
|-----------|---------|---------|---------------|
| LLM | llama.cpp | `llama-cpp-python` | Qwen3-0.6B (GGUF) |
| MLLM/VLM | llama.cpp + MoondreamChatHandler | `llama-cpp-python` | Moondream2 (GGUF) |
| Vision | ONNX Runtime | `onnxruntime` | DEIM (CVPR 2025) |
| SpeechToText | sherpa-onnx (Whisper) | `sherpa-onnx` | Whisper tiny.en |
| TextToSpeech | sherpa-onnx (Kokoro) | `sherpa-onnx` | Kokoro English |

These backends require no PyTorch, no Transformers, and no heavy ML frameworks -- they are designed for robots and edge devices including NVIDIA Jetson.

### Local Model Wrappers

The local model wrappers live in `agents/utils/` and follow a simple callable interface:

- `LocalLLM(model_path, device, ncpu)` -- wraps `llama-cpp-python`, returns `{"output": str}` or `{"output": generator}` for streaming, with optional `"tool_calls"`
- `LocalVLM(model_path, device, ncpu)` -- wraps `llama-cpp-python` with `MoondreamChatHandler`, accepts images as RGB numpy arrays
- `LocalVisionModel(model_path, device, ncpu)` -- wraps `onnxruntime` for object detection, returns bounding boxes, labels, and scores
- `LocalSTT(model_path, device, ncpu)` -- wraps `sherpa-onnx` `OfflineRecognizer`, accepts audio bytes or numpy arrays
- `LocalTTS(model_path, device, ncpu)` -- wraps `sherpa-onnx` `OfflineTts`, returns WAV bytes

### Customizing Local Models

Each config exposes a `local_model_path` field that accepts a HuggingFace repository ID or a local file path. Users can swap in any compatible model by setting this field:

```python
config = LLMConfig(
    enable_local_model=True,
    local_model_path="bartowski/Llama-3.2-1B-Instruct-GGUF",  # any GGUF model
)
```

For available STT and TTS models, see the [sherpa-onnx pretrained models catalog](https://k2-fsa.github.io/sherpa/onnx/pretrained_models/index.html).
