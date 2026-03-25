from typing import Dict, List

import numpy as np
from .utils import encode_img_base64


class LocalVLM:
    """Local VLM inference using llama-cpp-python with Moondream2.

    :param model_path: HuggingFace repository ID for a GGUF VLM model
        (e.g. ``ggml-org/moondream2-20250414-GGUF``) or a local path to a
        ``.gguf`` file. When using a HuggingFace repo, the mmproj file is
        downloaded automatically.
    :param device: Device to run on ('cpu' or 'cuda')
    :param ncpu: Number of CPU threads
    """

    def __init__(self, model_path: str, device: str = "cuda", ncpu: int = 1):
        try:
            from llama_cpp import Llama
            from llama_cpp.llama_chat_format import MoondreamChatHandler
        except ImportError as e:
            raise ImportError(
                "Local VLM model deployment requires llama-cpp-python. "
                "Install it with: pip install llama-cpp-python\n"
                "For NVIDIA GPUs build with: "
                'CMAKE_ARGS="-DGGML_CUDA=on" pip install llama-cpp-python\n'
            ) from e

        self.device = device
        self.ncpu = ncpu

        n_gpu_layers = -1 if device == "cuda" else 0

        chat_handler = MoondreamChatHandler.from_pretrained(
            repo_id=model_path,
            filename="*mmproj*",
        )

        self.llm = Llama.from_pretrained(
            repo_id=model_path,
            filename="*text-model*.gguf",
            chat_handler=chat_handler,
            n_gpu_layers=n_gpu_layers,
            n_threads=ncpu,
            n_ctx=4096,
            verbose=False,
        )

    def __call__(self, inference_input: Dict) -> Dict:
        """Run VLM inference.

        :param inference_input: Dict with 'query' (messages list) and
            'images' (list of RGB numpy arrays)
        :returns: Dict with 'output' (str)
        """
        # Extract the text query from messages
        messages = inference_input["query"]
        query = ""
        for msg in reversed(messages):
            if msg["role"] == "user":
                query = msg["content"]
                break

        # Get images
        images: List[np.ndarray] = inference_input.get("images", [])
        if not images:
            return {"output": "No image provided."}

        # Convert first RGB numpy image to base64 data URI
        data_uri = f"data:image/png;base64,{encode_img_base64(images[0])}"

        # Build multimodal message in OpenAI vision format
        multimodal_messages = [
            {
                "role": "user",
                "content": [
                    {"type": "image_url", "image_url": {"url": data_uri}},
                    {"type": "text", "text": query},
                ],
            }
        ]

        response = self.llm.create_chat_completion(messages=multimodal_messages)
        return {"output": response["choices"][0]["message"]["content"]}
