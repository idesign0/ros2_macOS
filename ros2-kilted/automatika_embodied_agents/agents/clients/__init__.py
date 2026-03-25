"""
Clients are standard interfaces for components to interact with ML models or vector DBs served by various platforms. Currently _EmbodiedAgents_ provides the following clients, which cover the most popular open source model deployment platforms. Simple clients can be easily implemented for other platforms and the use of unnecessarily heavy duct-tape "AI" frameworks on the robot is discouraged 😅.

```{note}
Some clients might need additional dependacies, which are provided in the following table. If missing the user will also be prompted for them at runtime.
```

```{list-table}
:widths: 20 20 60
:header-rows: 1
* - Platform
  - Client
  - Description

* - **Generic**
  - [GenericHTTPClient](agents.clients.generic.GenericHTTPClient)
  - A generic client for interacting with OpenAI-compatible APIs, including vLLM, ms-swift, lmdeploy, Google Gemini, etc. Supports both standard and streaming responses, and works with LLMS and multimodal LLMs. Designed to be compatible with any API following the OpenAI standard. Supports tool calling.

* - **RoboML**
  - [RoboMLHTTPClient](agents.clients.roboml.RoboMLHTTPClient)
  - An HTTP client for interacting with ML models served on [RoboML](https://github.com/automatika-robotics/roboml). Supports streaming outputs.

* - **RoboML**
  - [RoboMLWSClient](agents.clients.roboml.RoboMLWSClient)
  - A WebSocket-based client for persistent interaction with [RoboML](https://github.com/automatika-robotics/roboml)-hosted ML models. Particularly useful for low-latency streaming of audio or text data.

* - **RoboML**
  - [RoboMLRESPClient](agents.clients.roboml.RoboMLRESPClient)
  - A Redis Serialization Protocol (RESP) based client for ML models served via [RoboML](https://github.com/automatika-robotics/roboml).
    Requires `pip install redis[hiredis]`.

* - **Ollama**
  - [OllamaClient](agents.clients.ollama.OllamaClient)
  - An HTTP client for interacting with ML models served on [Ollama](https://ollama.com). Supports LLMs/MLLMs and embedding models. It can be invoked with the generic [OllamaModel](agents.models.md#classes). Supports tool calling.
    Requires `pip install ollama`.

* - **LeRobot**
  - [LeRobotClient](agents.clients.lerobot.LeRobotClient)
  - A GRPC based asynchronous client for vision-language-action (VLA) policies served on LeRobot Policy Server. Supports various robot action policies available in LeRobot package by HuggingFace. It can be invoked with the generic wrapper [LeRobotPolicy](agents.models.md#classes).
    Requires grpc and torch (at least the CPU version):<br/>
    `pip install grpcio`<br/>
    `pip install torch --index-url https://download.pytorch.org/whl/cpu`

* - **ChromaDB**
  - [ChromaClient](agents.clients.chroma.ChromaClient)
  - An HTTP client for interacting with a ChromaDB instance running as a server.
    Ensure that a ChromaDB server is active using:<br/>
    `pip install chromadb`<br/>
    `chroma run --path /db_path`
"""

from .generic import GenericHTTPClient
from .ollama import OllamaClient
from .roboml import (
    RoboMLHTTPClient,
    RoboMLRESPClient,
    RoboMLWSClient,
)
from .lerobot import LeRobotClient
from .chroma import ChromaClient


__all__ = [
    "GenericHTTPClient",
    "OllamaClient",
    "LeRobotClient",
    "ChromaClient",
    "RoboMLHTTPClient",
    "RoboMLRESPClient",
    "RoboMLWSClient",
]
