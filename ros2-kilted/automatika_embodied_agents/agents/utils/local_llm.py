import json
from typing import Dict, Generator, Union


class LocalLLM:
    """Local LLM inference using llama-cpp-python.

    :param model_path: HuggingFace repository ID for a GGUF model
        (e.g. ``Qwen/Qwen3-0.6B-GGUF``) or a local path to a ``.gguf`` file.
    :param device: Device to run on ('cpu' or 'cuda')
    :param ncpu: Number of CPU threads
    """

    def __init__(self, model_path: str, device: str = "cuda", ncpu: int = 1):
        try:
            from llama_cpp import Llama
        except ImportError as e:
            raise ImportError(
                "Local LLM model deployment requires llama-cpp-python. "
                "Install it with: pip install llama-cpp-python\n"
                "For NVIDIA GPUs build with: "
                'CMAKE_ARGS="-DGGML_CUDA=on" pip install llama-cpp-python\n'
            ) from e

        self.device = device
        self.ncpu = ncpu

        n_gpu_layers = -1 if device == "cuda" else 0

        if model_path.endswith(".gguf"):
            self.llm = Llama(
                model_path=model_path,
                n_gpu_layers=n_gpu_layers,
                n_threads=ncpu,
                verbose=False,
            )
        else:
            self.llm = Llama.from_pretrained(
                repo_id=model_path,
                filename="*.gguf",
                n_gpu_layers=n_gpu_layers,
                n_threads=ncpu,
                verbose=False,
            )

    def __call__(
        self, inference_input: Dict, stream=False
    ) -> Union[Dict, Generator[str, None, None]]:
        """Run inference and return complete response.

        :param inference_input: Dict with 'query' (messages list) and optional
            'temperature', 'max_new_tokens', 'tools'
        :returns: Dict with 'output' (str) and optionally 'tool_calls'
        """
        kwargs = {
            "messages": inference_input["query"],
            "stream": stream,
        }
        if temperature := inference_input.get("temperature"):
            kwargs["temperature"] = temperature
        if max_new_tokens := inference_input.get("max_new_tokens"):
            kwargs["max_tokens"] = max_new_tokens

        if tools := inference_input.get("tools"):
            kwargs["tools"] = tools
            kwargs["tool_choice"] = "auto"

        response = self.llm.create_chat_completion(**kwargs)

        if stream:
            return {"output": self._stream_tokens(response)}

        choice = response["choices"][0]
        message = choice["message"]

        result = {"output": message.get("content") or ""}

        if message.get("tool_calls"):
            result["tool_calls"] = [
                {
                    "function": {
                        "name": tc["function"]["name"],
                        "arguments": json.loads(tc["function"]["arguments"])
                        if isinstance(tc["function"]["arguments"], str)
                        else tc["function"]["arguments"],
                    }
                }
                for tc in message["tool_calls"]
            ]

        return result

    def _stream_tokens(self, response) -> Generator[str, None, None]:
        """Yield decoded text tokens from a streaming response.

        :param response: Streaming iterator from create_chat_completion
        :yields: Decoded text strings, one per chunk
        """
        for chunk in response:
            delta = chunk["choices"][0]["delta"]
            if "content" in delta and delta["content"]:
                yield delta["content"]
