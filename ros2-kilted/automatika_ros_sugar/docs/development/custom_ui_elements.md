# Adding UI Elements

This guide covers how to register custom input and output elements in Sugarcoat's web UI for new data types. This is how downstream packages like EmbodiedAgents add visualization for types such as `StreamingString`, `Detections`, or `PointsOfInterest`.

## How the UI Extension System Works

Sugarcoat's web UI renders input forms and output displays based on the message types of component topics. Built-in types (`String`, `Image`, `Float64`, etc.) have default UI elements. For custom types, downstream packages register their own elements through the `UI_EXTENSIONS` hook.

The flow:

1. Your package defines UI element functions and registers them in `UI_EXTENSIONS`.
2. The Launcher serializes the registrations and passes them to the UI node.
3. The UI node deserializes and loads them at runtime.

## Step 1: Write Element Functions

### Output Elements

Output elements render component output data in the logging panel. They receive a logging card container and must return it with the new content appended:

```python
from ros_sugar.ui_node.elements import _log_text_element

def _log_my_data_element(logging_card, output, data_src: str):
    """Render MyDataType output as a text summary."""
    summary = f"Value: {output['value']:.2f}, Status: {output['status']}"
    return _log_text_element(logging_card, summary, data_src)
```

#### Output Element Signature

```python
def output_element(
    logging_card,          # FastHTML container to append to
    output: Any,           # The deserialized callback output
    data_src: str,         # Source label (e.g. "info", "user", "robot")
) -> FT:
    """Return the logging_card with new content appended."""
```

You can reuse built-in rendering helpers from `ros_sugar.ui_node.elements`:

| Helper | Renders |
|:-------|:--------|
| `_log_text_element(card, text, src, id="text")` | Text log entry |
| `_out_image_element(card, output, src)` | JPEG-encoded image |
| `_out_map_element(card, output, src)` | Occupancy grid map |
| `_log_audio_element(card, output, src)` | Audio playback element |
| `augment_text_in_logging_card(card, text, target_id)` | Append text to an existing element (for streaming) |

### Input Elements

Input elements render forms that let users send data to component input topics:

```python
from fasthtml.common import Form, Input

def _in_my_data_element(topic_name: str, topic_type: str, **_):
    """Render an input form for MyDataType."""
    return (
        Form(cls="mb-1 p-1")(
            Input(name="topic_name", type="hidden", value=topic_name),
            Input(name="topic_type", type="hidden", value=topic_type),
            Input(
                name="data",
                placeholder="Enter value...",
                type="number",
                required=True,
            ),
            id=f"{topic_name}-form",
            ws_send=True,
            hx_on__ws_after_send="this.reset(); return false;",
        ),
    )
```

#### Input Element Signature

```python
def input_element(
    topic_name: str,       # ROS topic name
    topic_type: str,       # Message type name (e.g. "MyDataType")
    **_,                   # Ignore extra kwargs
) -> FT:
    """Return a FastHTML form element."""
```

The form must include hidden fields for `topic_name` and `topic_type`, and use `ws_send=True` for WebSocket submission.

## Step 2: Register the Elements

Create a `ui_elements.py` module in your package that maps your `SupportedType` classes to their UI functions:

```python
# my_package/ui_elements.py
from ros_sugar.ui_node.elements import _log_text_element, _out_image_element
from .ros import MyDataType, MyImageType

def _log_my_data_element(logging_card, output, data_src: str):
    summary = f"Value: {output['value']:.2f}"
    return _log_text_element(logging_card, summary, data_src)

OUTPUT_ELEMENTS = {
    MyDataType: _log_my_data_element,
    MyImageType: _out_image_element,     # Reuse built-in image renderer
}

INPUT_ELEMENTS = {
    # Add input elements here if needed
}
```

Dictionary keys must be `SupportedType` subclasses (the actual class, not a string name).

## Step 3: Hook into UI_EXTENSIONS

Register your elements in Sugarcoat's `UI_EXTENSIONS` dictionary. This should happen at import time (e.g. in your package's `ros.py` or `__init__.py`):

```python
# my_package/ros.py (or __init__.py)
from ros_sugar import UI_EXTENSIONS

def augment_ui():
    from .ui_elements import INPUT_ELEMENTS, OUTPUT_ELEMENTS
    return INPUT_ELEMENTS, OUTPUT_ELEMENTS

UI_EXTENSIONS["my_package"] = augment_ui
```

Key points:

- The value is a **callable** (not the dicts directly) — it is called lazily by the Launcher.
- The callable must return a tuple: `(input_elements_dict, output_elements_dict)`.
- The dictionary key (`"my_package"`) is arbitrary but should be unique.
- Use a deferred import inside the callable to avoid circular imports.

## How It Works at Runtime

1. When `Launcher.enable_ui()` is called, it iterates `UI_EXTENSIONS` and calls each registered function.
2. The returned element classes and functions are serialized as module-qualified paths (e.g. `my_package.ui_elements._log_my_data_element`).
3. The UI node deserializes them via `importlib.import_module()` and registers them in the global `_INPUT_ELEMENTS` / `_OUTPUT_ELEMENTS` dictionaries.
4. When the UI renders a topic, it looks up the message type name in these dictionaries and calls the corresponding function.

## Complete Example

Adding UI support for a `HapticReading` type from a custom package:

```python
# my_package/ui_elements.py
from ros_sugar.ui_node.elements import _log_text_element
from .ros import HapticReading


def _log_haptic_element(logging_card, output, data_src: str):
    """Render haptic readings as a summary."""
    mean_pressure = output[0].mean()
    max_pressure = output[0].max()
    summary = f"Pressure — mean: {mean_pressure:.2f}, max: {max_pressure:.2f}"
    return _log_text_element(logging_card, summary, data_src)


OUTPUT_ELEMENTS = {
    HapticReading: _log_haptic_element,
}

INPUT_ELEMENTS = {}
```

```python
# my_package/ros.py
from ros_sugar import UI_EXTENSIONS

def augment_ui():
    from .ui_elements import INPUT_ELEMENTS, OUTPUT_ELEMENTS
    return INPUT_ELEMENTS, OUTPUT_ELEMENTS

UI_EXTENSIONS["my_package"] = augment_ui
```

No further configuration is needed — the Launcher picks up the extension automatically when the package is imported.
