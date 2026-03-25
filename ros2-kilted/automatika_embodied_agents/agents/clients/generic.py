from typing import Any, Dict, Generator, Optional, Union, MutableMapping
import os
import json
import io
import numpy as np
import wave

import httpx

from .model_base import ModelClient
from ..models import (
    Model,
    GenericLLM,
    GenericSTT,
    GenericTTS,
    TransformersLLM,
)
from ..utils import encode_img_base64, validate_func_args


__all__ = ["GenericHTTPClient"]


class GenericHTTPClient(ModelClient):
    """
    A generic client for interacting with OpenAI-compatible APIs, including vLLM, ms-swift, lmdeploy, Google Gemini etc. This client works with LLM multimodal LLM models and supports both standard and streaming responses. It is designed to be compatible with any API that follows the OpenAI standard.
    """

    @validate_func_args
    def __init__(
        self,
        model: Union[Model, Dict],
        host: str = "127.0.0.1",
        port: Optional[int] = 8000,
        inference_timeout: int = 30,
        api_key: Optional[str] = None,
        logging_level: str = "info",
        **kwargs,
    ):
        """
        Initializes the Client.
        :param model: The model to be used for inference.
        :type model: Union[Model, Dict]
        :param host: The hostname of the API server.
        :type host: str
        :param port: The port of the API server.
        :type port: Optional[int]
        :param inference_timeout: The timeout for inference requests.
        :type inference_timeout: int
        :param api_key: The API key for authentication. If not provided, it will be
                        retrieved from the OPENAI_API_KEY environment variable.
        :type api_key: Optional[str]
        :param logging_level: The logging level.
        :type logging_level: str
        """
        if not isinstance(model, (GenericLLM, GenericSTT, GenericTTS, TransformersLLM)):
            raise TypeError(
                "A generic client can only take models of type GenericLLM, GenericTTS, GenericSTT, GenericMLLM, TransformersLLM and TransformersMLLM"
            )

        super().__init__(
            model=model,
            host=host,
            port=port,
            inference_timeout=inference_timeout,
            init_on_activation=True,
            logging_level=logging_level,
            **kwargs,
        )

        # try to get it from the environment variable otherwise default to empty string
        self.api_key = api_key or os.environ.get("OPENAI_API_KEY", "")
        header = {} if not self.api_key else {"Authorization": f"Bearer {self.api_key}"}

        self.url = f"http://{self.host}:{self.port}"

        # Create a synchronous httpx client
        self.client = httpx.Client(
            base_url=self.url,
            timeout=self.inference_timeout,
            headers=header,
        )

    @property
    def supports_tool_calls(self) -> bool:
        """
        Generic HTTP client (OpenAI compatible) supports tool calling.
        :rtype: bool
        """
        return True

    def _check_connection(self) -> None:
        """
        Checks the connection to the API server by making a simple request.
        """
        self.logger.info("Checking connection with OpenAI-compatible API...")
        try:
            # Make a request to the models endpoint to verify the connection
            self.client.get("/v1/models").raise_for_status()
        except Exception as e:
            self.__handle_exceptions(e)
            raise

    def _initialize(self) -> None:
        """
        Initializes the client by:
        1. Determining the correct API endpoint based on the model type.
        2. Verifying that the requested checkpoint exists on the server.
        """
        self.logger.info(f"Initializing {self.model_name}...")

        # Determine Endpoint and Mode
        if self.model_type in [
            "GenericLLM",
            "GenericMLLM",
            "TransformersLLM",
            "TransformersMLLM",
        ]:
            self.api_endpoint = "/v1/chat/completions"
            self.request_type = "json"
        elif self.model_type == "GenericTTS":
            self.api_endpoint = "/v1/audio/speech"
            self.request_type = "tts"  # Special handling for binary output
        elif self.model_type == "GenericSTT":
            self.api_endpoint = "/v1/audio/transcriptions"
            self.request_type = "multipart"  # Special handling for file upload
        else:
            # Fallback or error for unknown model types
            self.logger.warning(
                f"Model type {type(self.model_type)} not explicitly supported by GenericHTTPClient. Defaulting to chat completions."
            )
            self.api_endpoint = "/v1/chat/completions"
            self.request_type = "json"

        # Validate Model Availability
        self._validate_model_availability()

    def _validate_model_availability(self) -> None:
        """
        Queries the /v1/models endpoint to check if the requested checkpoint exists.
        """
        target_model = self.model_init_params["checkpoint"]
        self.logger.debug(f"Checking availability of model: {target_model}")

        try:
            response = self.client.get("/v1/models")
            response.raise_for_status()
            available_models = response.json().get("data", [])

            # Extract IDs from the response list
            model_ids = [m.get("id") for m in available_models]

            if target_model not in model_ids:
                self.logger.warning(
                    f"Model '{target_model}' not found in available models: {model_ids}. "
                )
            else:
                self.logger.info(f"Model '{target_model}' verified on server.")

        except Exception as e:
            # Log a warning but don't stop initialization, as some custom endpoints
            # might not implement /v1/models perfectly.
            self.logger.warning(f"Could not verify model availability: {e}")

    def _inference(
        self, inference_input: Dict[str, Any]
    ) -> Optional[MutableMapping[str, Union[str, Generator]]]:
        """Performs inference using the specified model and input.

        :param inference_input: The input for the inference. This should be a
                                dictionary containing the messages and other
                                parameters for the API request.
        :type inference_input: Dict[str, Any]
        :return: A dictionary containing the output of the inference. If
                 streaming is enabled, the output will be a generator.
        :rtype: Optional[Dict[str, Union[str, Generator]]]
        :raises httpx.RequestError: If there is an error during the inference request.
        :raises httpx.HTTPStatusError: If the API returns an error during inference.
        """
        try:
            # Standard Chat (JSON in, JSON out)
            if self.request_type == "json":
                return self._inference_chat(inference_input)

            # Text-to-Speech (Text in, bytes out)
            elif self.request_type == "tts":
                payload = {
                    "model": self.model_init_params["checkpoint"],
                    "input": inference_input.pop("query"),
                    "speed": self.model_init_params.get("speed", 1.0),
                    "voice": self.model_init_params.get(
                        "voice", "alloy"
                    ),  # adding default param
                }
                response = self.client.post(self.api_endpoint, json=payload)
                response.raise_for_status()
                # Send back audio bytes
                return {"output": response.content}  # type: ignore

            # Speech-to-Text (Multipart in, JSON out)
            elif self.request_type == "multipart":
                audio_data = inference_input.pop("query")
                # get sample rate
                sample_rate = inference_input.pop("sample_rate", 16000)
                # initialize file obj
                file_obj: io.BytesIO

                # Convert numpy to bytes if necessary
                if isinstance(audio_data, np.ndarray):
                    # Convert Float32 to Int16 if necessary
                    if audio_data.dtype.kind == "f":
                        audio_data = (audio_data * 32767).astype(np.int16)
                    elif audio_data.dtype != np.int16:
                        audio_data = audio_data.astype(np.int16)

                    # Wrap in WAV container
                    buffer = io.BytesIO()
                    with wave.open(buffer, "wb") as wav_file:
                        wav_file.setnchannels(1)
                        wav_file.setsampwidth(2)  # 16-bit
                        wav_file.setframerate(sample_rate)
                        wav_file.writeframes(audio_data.tobytes())
                    buffer.seek(0)
                    file_obj = buffer

                elif isinstance(audio_data, bytes):
                    if audio_data.startswith(b"RIFF"):
                        file_obj = io.BytesIO(audio_data)
                    else:
                        # With no header assume raw PCM (from VAD) -> Wrap in WAV
                        buffer = io.BytesIO()
                        with wave.open(buffer, "wb") as wav_file:
                            wav_file.setnchannels(1)
                            wav_file.setsampwidth(2)  # 16-bit
                            wav_file.setframerate(sample_rate)
                            wav_file.writeframes(audio_data)
                        buffer.seek(0)
                        file_obj = buffer

                else:
                    # Raises error if input is neither bytes nor numpy (satisfies type checker)
                    self.logger.error(
                        f"Unsupported audio data type: {type(audio_data)}"
                    )
                    return

                # Prepare multipart file
                files = {"file": ("audio.wav", file_obj, "audio/wav")}
                data = {
                    "model": self.model_init_params["checkpoint"],
                    "language": inference_input.get("language")
                    or self.model_init_params.get("language"),
                    "temperature": self.model_init_params.get("temperature", 0.0),
                }

                response = self.client.post(self.api_endpoint, files=files, data=data)
                response.raise_for_status()
                return {"output": response.json().get("text", "")}

        except Exception as e:
            self.__handle_exceptions(e)

    def _inference_chat(
        self, inference_input: Dict[str, Any]
    ) -> Optional[Dict[str, Union[str, Generator]]]:
        """Helper for chat completion with streaming and tool calling"""

        # Handle Images
        inference_input = self.__handle_images(inference_input)

        # openai uses max_tokens for max_new_tokens
        inference_input["max_tokens"] = inference_input.pop("max_new_tokens")

        if self.model_init_params.get("options"):
            inference_input = (
                {**self.model_init_params["options"], **inference_input}
                if self.model_init_params.get("options")
                else inference_input
            )

        payload = {
            "model": self.model_init_params["checkpoint"],
            "messages": inference_input.pop("query"),
            **inference_input,
        }

        # If streaming is enabled return a generator
        if payload.get("stream"):
            return {"output": self._stream_generator(payload)}
        else:
            response = self.client.post(self.api_endpoint, json=payload)
            response.raise_for_status()
            return self._parse_chat_response(response.json())

    def _stream_generator(
        self, payload: Dict[str, Any]
    ) -> Generator[Dict[str, Any], None, None]:
        """
        Yields parsed JSON chunks from the server stream.
        """
        with self.client.stream("POST", self.api_endpoint, json=payload) as response:
            response.raise_for_status()
            for line in response.iter_lines():
                if line.startswith("data:"):
                    data_str = line.removeprefix("data: ").strip()
                    if data_str == "[DONE]":
                        break
                    if data_str:
                        try:
                            yield json.loads(data_str)
                        except json.JSONDecodeError:
                            pass

    def _parse_chat_response(self, r_json) -> Optional[Dict]:
        """Helper to parse standard chat response"""
        if not r_json.get("choices"):
            return

        message = r_json["choices"][0]["message"]
        model_resp = {}

        if tool_calls := message.get("tool_calls"):
            model_resp["tool_calls"] = tool_calls

        model_resp["output"] = message.get("content") or ""
        return model_resp

    def _deinitialize(self) -> None:
        """
        Deinitializes the client by closing the httpx client.
        """
        self.logger.info("Deinitializing GenericHTTPClient...")
        self.client.close()

    def __handle_images(self, inference_input: Dict[str, Any]) -> Dict[str, Any]:
        """Handles images in multimodal input"""
        if images := inference_input.get("images"):
            b64_images = [encode_img_base64(img) for img in images]
            inference_input.pop("images")  # Remove images from inference_input
            # Add images to last message in query
            if inference_input["query"][-1].get("role") == "user":
                last_message = inference_input["query"][-1]
                content_parts = []

                # Add the original text prompt as the first part
                content_parts.append({"type": "text", "text": last_message["content"]})

                # Add each image as a subsequent part
                for img_b64 in b64_images:
                    content_parts.append({
                        "type": "image_url",
                        "image_url": {"url": f"data:image/jpeg;base64,{img_b64}"},
                    })

                # Replace the original content with the new list of parts
                inference_input["query"][-1]["content"] = content_parts
                return inference_input
            else:
                self.logger.warning(
                    "Images provided, but no final user message found to attach them to. Ignoring images."
                )
        return inference_input

    def __handle_exceptions(self, excep: Exception) -> None:
        """__handle_exceptions.

        :param excep:
        :type excep: Exception
        :rtype: None
        """
        if isinstance(excep, httpx.RequestError):
            self.logger.error(
                f"{excep}. API inaccessible. Might not be running. Make sure remote is correctly configured."
            )
        elif isinstance(excep, httpx.TimeoutException):
            self.logger.error(
                f"{excep}. Request to API server timed out. Make sure the server is configured correctly."
            )
        elif isinstance(excep, httpx.HTTPStatusError):
            try:
                excep_json = excep.response.json()
                self.logger.error(
                    f"API server returned an invalid status code. Error: {excep_json}"
                )
            except Exception:
                self.logger.error(
                    f"API server returned an invalid status code. Error: {excep}"
                )
        else:
            self.logger.error(str(excep))
