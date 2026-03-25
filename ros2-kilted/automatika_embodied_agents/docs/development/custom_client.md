# Creating a Custom Model Client

Model clients in EmbodiedAgents manage the connection to a model serving platform. This guide covers the `ModelClient` base contract, serialization requirements, tool calling support, the `DBClient` variant, and a complete skeleton for an HTTP-based client.

## ModelClient Base Contract

All model clients must subclass `ModelClient` (defined in `agents.clients.model_base`) and implement four abstract methods:

```python
from agents.clients.model_base import ModelClient

class MyClient(ModelClient):

    def _check_connection(self) -> None:
        """Verify that the serving platform is reachable.
        Called during the component's configure lifecycle phase.
        Raise an exception if the connection cannot be established."""
        ...

    def _initialize(self) -> None:
        """Initialize the model on the serving platform.
        Called after _check_connection() during configure.
        Only runs if init_on_activation is True (default)."""
        ...

    def _inference(self, inference_input: dict) -> dict | None:
        """Run a single inference call.
        Takes a dict assembled by the component's _create_input().
        Returns a dict with at least an 'output' key, or None on failure.
        For streaming, may return a Generator as the 'output' value."""
        ...

    def _deinitialize(self) -> None:
        """Clean up model resources on the serving platform.
        Called during the component's deactivate lifecycle phase."""
        ...
```

### Constructor

The `ModelClient.__init__()` accepts:

| Parameter | Type | Description |
|---|---|---|
| `model` | `Model` or `Dict` | The model definition (attrs class or deserialized dict). |
| `host` | `str` or `None` | Hostname of the serving platform. |
| `port` | `int` or `None` | Port of the serving platform. |
| `inference_timeout` | `int` | Timeout in seconds for inference calls. Default: 30. |
| `init_on_activation` | `bool` | Whether to call `_initialize()` on activation. Default: True. |
| `logging_level` | `str` | Logging level (e.g., "info", "debug"). |

Your subclass constructor must call `super().__init__()`:

```python
def __init__(self, model, host="127.0.0.1", port=8080, **kwargs):
    super().__init__(model=model, host=host, port=port, **kwargs)
    # Custom init here
```

## Serialization for Multiprocess Execution

EmbodiedAgents components run in separate processes. The model client must be serializable so it can be reconstructed in the child process. The base `ModelClient.serialize()` method returns a dict:

```python
{
    "client_type": "MyClient",
    "model": {
        "model_name": "my_model",
        "model_type": "OllamaModel",
        "init_timeout": None,
        "model_init_params": {"checkpoint": "llama3.2:3b", ...},
    },
    "host": "127.0.0.1",
    "port": 8080,
    "init_on_activation": True,
    "logging_level": "INFO",
    "inference_timeout": 30,
}
```

If your client stores additional state (e.g., API keys, custom headers), override `serialize()` to include them, and handle deserialization from a `Dict` in your `__init__()`:

```python
def __init__(self, model, host=None, port=None, api_key=None, **kwargs):
    super().__init__(model=model, host=host, port=port, **kwargs)
    self.api_key = api_key

def serialize(self):
    base = super().serialize()
    base["api_key"] = self.api_key
    return base
```

## Tool Calling Support

If your model serving platform supports function/tool calling, override the `supports_tool_calls` property:

```python
@property
def supports_tool_calls(self) -> bool:
    return True
```

When `supports_tool_calls` returns `True`, components like `LLM` will include tool descriptions in the inference payload. Your `_inference()` method must then handle the tool calling protocol (parsing tool call responses, executing tools, and returning final results).

The default implementation returns `False`.

## DBClient for Vector Databases

`DBClient` (defined in `agents.clients.db_base`) is the equivalent of `ModelClient` for vector database connections. It is used by `MapEncoding` and `SemanticRouter` components.

The abstract methods:

```python
from agents.clients.db_base import DBClient

class MyDBClient(DBClient):

    def _check_connection(self) -> None: ...
    def _initialize(self) -> None: ...
    def _add(self, db_input: dict) -> dict | None: ...
    def _conditional_add(self, db_input: dict) -> dict | None: ...
    def _query(self, db_input: dict) -> dict | None: ...
    def _metadata_query(self, db_input: dict) -> dict | None: ...
    def _deinitialize(self) -> None: ...
```

The constructor takes a `DB` instance (from `agents.vectordbs`) instead of a `Model`.

## Complete Skeleton: Custom HTTP Client

Below is a complete skeleton for a client that communicates with a custom HTTP inference server.

```python
from typing import Any, Dict, Optional, Union, MutableMapping

import httpx

from agents.clients.model_base import ModelClient
from agents.models import Model


class CustomHTTPClient(ModelClient):
    """Client for a custom HTTP model serving endpoint."""

    def __init__(
        self,
        model: Union[Model, Dict],
        host: str = "127.0.0.1",
        port: int = 5000,
        inference_timeout: int = 30,
        api_key: Optional[str] = None,
        logging_level: str = "info",
        **kwargs,
    ):
        super().__init__(
            model=model,
            host=host,
            port=port,
            inference_timeout=inference_timeout,
            logging_level=logging_level,
            **kwargs,
        )
        self.api_key = api_key
        self.base_url = f"http://{self.host}:{self.port}"
        self._client: Optional[httpx.Client] = None

    def _check_connection(self) -> None:
        """Ping the server health endpoint."""
        try:
            resp = httpx.get(
                f"{self.base_url}/health",
                timeout=5.0,
            )
            resp.raise_for_status()
            self.logger.info("Server is reachable")
        except httpx.HTTPError as e:
            raise ConnectionError(
                f"Cannot reach server at {self.base_url}: {e}"
            )

    def _initialize(self) -> None:
        """Load the model on the server and create the HTTP client."""
        headers = {}
        if self.api_key:
            headers["Authorization"] = f"Bearer {self.api_key}"

        self._client = httpx.Client(
            base_url=self.base_url,
            timeout=self.inference_timeout,
            headers=headers,
        )

        # Send model init params to the server
        resp = self._client.post(
            "/v1/models/load",
            json=self.model_init_params,
        )
        resp.raise_for_status()
        self.logger.info(f"Model {self.model_name} initialized on server")

    def _inference(
        self, inference_input: Dict[str, Any]
    ) -> Optional[MutableMapping]:
        """Send an inference request and return the result."""
        if not self._client:
            self.logger.error("Client not initialized")
            return None

        try:
            resp = self._client.post(
                "/v1/inference",
                json=inference_input,
            )
            resp.raise_for_status()
            data = resp.json()
            return {"output": data.get("result", "")}
        except httpx.HTTPError as e:
            self.logger.error(f"Inference failed: {e}")
            return None

    def _deinitialize(self) -> None:
        """Unload the model and close the HTTP client."""
        if self._client:
            try:
                self._client.post("/v1/models/unload")
            except httpx.HTTPError:
                pass
            self._client.close()
            self._client = None
        self.logger.info(f"Model {self.model_name} deinitialized")

    def serialize(self) -> Dict:
        """Include api_key in serialization for multiprocess."""
        base = super().serialize()
        base["api_key"] = self.api_key
        return base
```

### Usage

```python
from agents.models import GenericLLM
from agents.components import LLM
from agents.ros import Topic, Launcher

model = GenericLLM(name="my_model", checkpoint="my-custom-model")
client = CustomHTTPClient(model, host="10.0.0.5", port=5000, api_key="sk-...")

text_in = Topic(name="input_text", msg_type="String")
text_out = Topic(name="output_text", msg_type="String")

llm = LLM(
    inputs=[text_in],
    outputs=[text_out],
    model_client=client,
    trigger=text_in,
)

launcher = Launcher()
launcher.add_pkg(components=[llm])
launcher.bringup()
```
