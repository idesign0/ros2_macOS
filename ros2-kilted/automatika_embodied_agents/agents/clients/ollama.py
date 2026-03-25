from typing import Any, Optional, Dict, Union, List, Generator

import httpx

from ..models import OllamaModel
from ..utils import encode_img_base64
from .model_base import ModelClient

__all__ = ["OllamaClient"]


class OllamaClient(ModelClient):
    """An HTTP client for interaction with ML models served on ollama"""

    def __init__(
        self,
        model: Union[OllamaModel, Dict],
        host: str = "127.0.0.1",
        port: int = 11434,
        inference_timeout: int = 30,
        init_on_activation: bool = True,
        logging_level: str = "info",
        **kwargs,
    ):
        try:
            from ollama import Client

            self.client = Client(host=f"{host}:{port}")
        except ModuleNotFoundError as e:
            raise ModuleNotFoundError(
                "In order to use the OllamaClient, you need ollama-python package installed. You can install it with 'pip install ollama'"
            ) from e

        if not isinstance(model, OllamaModel) and model["model_type"] != "OllamaModel":
            raise TypeError("OllamaClient can only be used with an OllamaModel")

        super().__init__(
            model=model,
            host=host,
            port=port,
            inference_timeout=inference_timeout,
            init_on_activation=init_on_activation,
            logging_level=logging_level,
            **kwargs,
        )
        self._check_connection()

    @property
    def supports_tool_calls(self) -> bool:
        """
        Ollama client supports tool calling.

        :rtype: bool
        """
        return True

    def _check_connection(self) -> None:
        """Check if the platfrom is being served on specified IP and port"""
        # Ping remote server to check connection
        self.logger.info("Checking connection with remote_host Ollama")
        try:
            httpx.get(f"http://{self.host}:{self.port}").raise_for_status()
        except Exception:
            self.logger.error(
                f"""Failed to connect to Ollama server at {self.host}:{self.port}

                Make sure an Ollama instance is running on the given url by executing the following command:

                `export OLLAMA_HOST={self.host}:{self.port}  # if not using default`
                `ollama serve`

                To install ollama, follow instructions on https://ollama.com/download
                """
            )
            raise

    def _initialize(self) -> None:
        """
        Initialize the model on platform using the paramters provided in the model specification class
        """
        self.logger.info(f"Initializing {self.model_name} on ollama")
        try:
            # set timeout on underlying httpx client
            self.client._client.timeout = self.init_timeout
            r = self.client.pull(self.model_init_params["checkpoint"])
            if r.get("status") != "success":  # type: ignore
                raise Exception(
                    f"Could not pull model {self.model_init_params['checkpoint']}"
                )
            # load model in memory with empty request
            if (
                self.model_name == "internal_ollama_embeddings"
            ):  # Internal embeddings model name
                self.client.embed(
                    model=self.model_init_params["checkpoint"], keep_alive=10
                )
            else:
                self.client.generate(
                    model=self.model_init_params["checkpoint"], keep_alive=10
                )
        except Exception as e:
            self.logger.error(
                f"Failed to initialize model {self.model_init_params['checkpoint']} on Ollama, please ensure that the checkpoint name is correct. Received the following error: {e}"
            )
            raise
        self.logger.info(
            f"{self.model_name} with {self.model_init_params['checkpoint']} model initialized"
        )

    def _inference(
        self, inference_input: Dict[str, Any]
    ) -> Optional[Dict[str, Union[str, Generator]]]:
        """Call inference on the model using data and inference parameters from the component"""
        # create input
        input = {
            "model": self.model_init_params["checkpoint"],
            "messages": inference_input.pop("query"),
            "stream": inference_input.pop("stream"),
        }

        # make images part of the latest message in message list
        if images := inference_input.get("images"):
            input["messages"][-1]["images"] = [encode_img_base64(img) for img in images]
            inference_input.pop("images")

        # Add tools as part of input, if available
        if tools := inference_input.get("tools"):
            input["tools"] = tools
            inference_input.pop("tools")

        # ollama uses num_predict for max_new_tokens
        inference_input["num_predict"] = inference_input.pop("max_new_tokens")

        # merge any options specified during OllamaModel definition
        input["options"] = (
            {**self.model_init_params["options"], **inference_input}
            if self.model_init_params.get("options")
            else inference_input
        )

        self.logger.debug(f"Sending to ollama server: {input}")

        # call inference method
        try:
            # set timeout on underlying httpx client
            self.client._client.timeout = self.inference_timeout
            ollama_result = self.client.chat(**input)
        except Exception as e:
            self.logger.error(str(e))
            return

        if isinstance(ollama_result, Generator):
            input["output"] = ollama_result
            return input

        if not ollama_result.get("message"):
            self.logger.debug(
                f"Ollama Response does not contain any model output: {ollama_result}"
            )
            return

        # if tool calls exist
        if tool_calls := ollama_result["message"].get("tool_calls"):  # type: ignore
            input["tool_calls"] = tool_calls

        input["output"] = ollama_result["message"].get("content", "")
        return input

    def _embed(
        self, input: Union[str, List[str]], truncate: bool = False
    ) -> Optional[List[List]]:
        # create input
        embedding_input = {
            "model": self.model_init_params["checkpoint"],
            "input": input,
            "truncate": truncate,
        }

        # call embedding method
        try:
            # set timeout on underlying httpx client
            self.client._client.timeout = self.inference_timeout
            ollama_result = self.client.embed(**embedding_input)
        except Exception as e:
            self.logger.error(str(e))
            return

        self.logger.debug(
            f"Created embeddings of length: {len(ollama_result['embeddings'])}"
        )

        # make result part of the input
        if not (result := ollama_result.get("embeddings")) or not result:
            self.logger.error("Embeddings not generated")
            return

        return ollama_result["embeddings"]

    def _deinitialize(self):
        """Deinitialize the model on the platform"""

        self.logger.error(f"Deinitializing {self.model_name} model on ollama")
        try:
            self.client.generate(
                model=self.model_init_params["checkpoint"], keep_alive=0
            )
        except Exception as e:
            self.logger.error(str(e))
