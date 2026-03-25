import asyncio
import base64
import queue
import threading
from enum import Enum
from typing import Any, Optional, Dict, Union, Generator
import websockets

import numpy as np
import httpx
import msgpack
import msgpack_numpy as m_pack

from .. import models
from ..models import Model, OllamaModel, TransformersLLM, TransformersMLLM
from ..utils import encode_img_base64
from .model_base import ModelClient

# patch msgpack for numpy arrays
m_pack.patch()


__all__ = ["RoboMLHTTPClient", "RoboMLRESPClient", "RoboMLWSClient"]


class Status(str, Enum):
    """Model Node Status."""

    LOADED = "LOADED"
    INITIALIZING = "INITIALIZING"
    READY = "READY"
    INITIALIZATION_ERROR = "INITIALIZATION_ERROR"


class RoboMLError(Exception):
    """RoboMLError."""

    pass


class RoboMLHTTPClient(ModelClient):
    """An HTTP client for interaction with ML models served on RoboML"""

    def __init__(
        self,
        model: Union[Model, Dict],
        host: str = "127.0.0.1",
        port: int = 8000,
        inference_timeout: int = 30,
        init_on_activation: bool = True,
        logging_level: str = "info",
        **kwargs,
    ):
        if isinstance(model, OllamaModel):
            raise TypeError(
                "An ollama model cannot be passed to a RoboML client. Please use the OllamaClient"
            )
        super().__init__(
            model=model,
            host=host,
            port=port,
            inference_timeout=inference_timeout,
            init_on_activation=init_on_activation,
            logging_level=logging_level,
            **kwargs,
        )
        self.url = f"http://{self.host}:{self.port}"

        # create httpx client
        self.client = httpx.Client(base_url=self.url, timeout=self.inference_timeout)
        self._check_connection()

    def _check_connection(self) -> None:
        """Check if the platfrom is being served on specified IP and port"""
        # Ping remote server to check connection
        self.logger.info("Checking connection with remote RoboML")
        try:
            self.client.get("/").raise_for_status()
        except Exception:
            self.logger.error(
                f"""Failed to connect to RoboML server at {self.url}

                Make sure an instance of RoboML is running on the given url by executing the following command:

                `roboml --host {self.host} --port {self.port}`

                To install RoboML, follow instructions on https://github.com/automatika-robotics/roboml?tab=readme-ov-file#installation
                """
            )
            raise

    def _initialize(self) -> None:
        """
        Initialize the model on platform using the paramters provided in the model specification class
        """
        # Create a model node on RoboML
        self.logger.info("Creating model node on remote")
        model_class = getattr(models, self.model_type)
        if issubclass(model_class, TransformersLLM):
            model_type = TransformersLLM.__name__
        elif issubclass(model_class, TransformersMLLM):
            model_type = TransformersMLLM.__name__
        else:
            model_type = self.model_type
        start_params = {"node_name": self.model_name, "node_model": model_type}
        try:
            r = self.client.post("/add_node", params=start_params).raise_for_status()
            self.logger.debug(str(r.json()))
            self.logger.info(f"Initializing {self.model_name} on RoboML remote")
            # get initialization params and initiale model
            self.client.post(
                f"/{self.model_name}/initialize",
                json=self.model_init_params,
                timeout=self.init_timeout,
            ).raise_for_status()
        except Exception as e:
            self.__handle_exceptions(e)
            raise
        self.logger.info(
            f"{self.model_name} with {self.model_init_params['checkpoint']} model initialized"
        )

    def _inference(
        self, inference_input: Dict[str, Any]
    ) -> Optional[Dict[str, Union[str, Generator]]]:
        """Call inference on the model using data and inference parameters from the component"""
        # encode any byte or numpy array data
        if "query" in inference_input.keys():
            if isinstance(inference_input["query"], bytes):
                inference_input["query"] = base64.b64encode(
                    inference_input["query"]
                ).decode("utf-8")
            if isinstance(inference_input["query"], np.ndarray):
                inference_input["query"] = base64.b64encode(
                    inference_input["query"].tobytes()
                ).decode("utf-8")
        if images := inference_input.get("images"):
            inference_input["images"] = [encode_img_base64(img) for img in images]

        # if stream is set to true, then return a generator
        if inference_input.get("stream"):

            def gen():
                with self.client.stream(
                    method="POST",
                    url=f"/{self.model_name}/inference",
                    json=inference_input,
                    timeout=self.inference_timeout,
                ) as r:
                    try:
                        r.raise_for_status()
                    except Exception as e:
                        self.__handle_exceptions(e)

                    for token in r.iter_text():
                        self.logger.debug(f"{token}")
                        yield token

            return {"output": gen()}

        try:
            # call inference method
            r = self.client.post(
                f"/{self.model_name}/inference",
                json=inference_input,
                timeout=self.inference_timeout,
            ).raise_for_status()
            result = r.json()
        except Exception as e:
            return self.__handle_exceptions(e)

        self.logger.debug(str(result))

        return result

    def _deinitialize(self) -> None:
        """Deinitialize the model on the platform"""

        self.logger.info(f"Deinitializing {self.model_name} model on RoboML remote")
        stop_params = {"node_name": self.model_name}
        try:
            self.client.post("/remove_node", params=stop_params).raise_for_status()
            self.client.close()
        except Exception as e:
            self.__handle_exceptions(e)
            if hasattr(self, "client") and self.client and not self.client.is_closed:
                self.logger.info("Closing HTTPX client.")
                self.client.close()

    def __handle_exceptions(self, excep: Exception) -> None:
        """__handle_exceptions.

        :param excep:
        :type excep: Exception
        :rtype: None
        """
        if isinstance(excep, httpx.RequestError):
            self.logger.error(
                f"{excep}. RoboML server inaccessible. Might not be running. Make sure remote is correctly configured."
            )
        elif isinstance(excep, httpx.TimeoutException):
            self.logger.error(
                f"{excep}. Request to RoboML server timed out. Make sure the server is configured correctly."
            )
        elif isinstance(excep, httpx.HTTPStatusError):
            try:
                excep_json = excep.response.json()
                self.logger.error(
                    f"RoboML server returned an invalid status code. Error: {excep_json}"
                )
            except Exception:
                self.logger.error(
                    f"RoboML server returned an invalid status code. Error: {excep}"
                )
        else:
            self.logger.error(str(excep))


class RoboMLWSClient(RoboMLHTTPClient):
    """An websocket client for interaction with ML models served on RoboML"""

    def __init__(
        self,
        model: Union[Model, Dict],
        host: str = "127.0.0.1",
        port: int = 8000,
        inference_timeout: int = 30,
        init_on_activation: bool = True,
        logging_level: str = "info",
        **kwargs,
    ):
        if isinstance(model, OllamaModel):
            raise TypeError(
                "An ollama model cannot be passed to a RoboML client. Please use the OllamaClient"
            )
        super().__init__(
            model=model,
            host=host,
            port=port,
            inference_timeout=inference_timeout,
            init_on_activation=init_on_activation,
            logging_level=logging_level,
            **kwargs,
        )
        # Add queues and events
        self.stop_event: Optional[threading.Event] = None
        self.request_queue: Optional[queue.Queue] = None
        self.response_queue: Optional[queue.Queue] = None
        self.websocket_endpoint = (
            f"ws://{self.host}:{self.port}/{self.model_name}/ws_inference"
        )

    def _inference(self) -> Optional[Dict]:
        """Run the event loop for websocket client function. This function is executed in a separate thread to not block the main component"""
        # Each thread needs its own asyncio event loop
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        try:
            loop.run_until_complete(self.__websocket_client())
        finally:
            self.logger.info("Closing asyncio event loop.")
            # Gracefully cancel all pending asyncio tasks in this loop
            for task in asyncio.all_tasks(loop):
                task.cancel()
            # Run loop one last time to allow tasks to process cancellation
            loop.run_until_complete(
                asyncio.gather(*asyncio.all_tasks(loop), return_exceptions=True)
            )
            loop.close()
        self.logger.info("WebSocket client thread finished.")

    async def __websocket_client(self):  # noqa: C901
        if not (self.stop_event and self.request_queue and self.response_queue):
            self.logger.error("WebSocketClient is not configured.")
            return

        try:
            async with websockets.connect(self.websocket_endpoint) as websocket:
                while not self.stop_event.is_set():
                    try:
                        # Attempt to send
                        inference_input = (
                            self.request_queue.get_nowait()
                        )  # Non-blocking
                        if websocket.close_code is None:
                            await websocket.send(msgpack.packb(inference_input))
                            self.logger.debug("Sent input to server")
                            self.request_queue.task_done()
                        else:
                            self.logger.warning(
                                "WebSocket is closed, requeueing request."
                            )
                            # Try to put it back, might fail if main thread is also stopping
                            try:
                                self.request_queue.put_nowait(inference_input)
                            except queue.Full:
                                self.logger.error(
                                    "Request queue full, cannot requeue lost message."
                                )
                            self.stop_event.set()  # Connection is closed, signal stop
                            break
                    except queue.Empty:
                        pass  # No request to send, this is normal
                    except websockets.exceptions.ConnectionClosed:
                        self.logger.error("Connection closed while trying to send.")
                        self.stop_event.set()
                        break  # Exit loop
                    except Exception as e_send:
                        self.logger.error(
                            f"Error sending message: {e_send} (Type: {type(e_send)})"
                        )

                    # Check stop event before attempting to receive
                    if self.stop_event.is_set():
                        break

                    # Attempt to receive
                    try:
                        # Use a short timeout for recv to keep the loop responsive
                        # for new send requests and the stop_event.
                        message = await asyncio.wait_for(
                            websocket.recv(), timeout=0.05
                        )  # 50ms timeout
                        self.logger.debug(f"Received from server: '{message}'")
                        self.response_queue.put(message)
                    except asyncio.TimeoutError:
                        pass  # No message received within timeout
                    except websockets.exceptions.ConnectionClosedOK:
                        self.logger.info(
                            "WebSocket connection closed gracefully by server."
                        )
                        self.stop_event.set()
                        break
                    except websockets.exceptions.ConnectionClosedError as e_close:
                        self.logger.error(
                            f"WebSocket connection closed with error: {e_close}"
                        )
                        self.stop_event.set()
                        break
                    except Exception as e_recv:
                        if not self.stop_event.is_set() and not isinstance(
                            e_recv, asyncio.CancelledError
                        ):
                            self.logger.error(
                                f"Error receiving message: {e_recv} (Type: {type(e_recv)})"
                            )
                        # assume the connection is compromised
                        self.stop_event.set()
                        break

                    # Yield control to prevent a tight loop if queues are empty
                    await asyncio.sleep(0.005)  # 5ms sleep

        except websockets.exceptions.InvalidURI:
            self.logger.error(f"Invalid WebSocket URI: {self.websocket_endpoint}")
        except (
            websockets.exceptions.WebSocketException
        ) as e:  # Covers connection errors like gaierror
            self.logger.error(
                f"Failed to connect to WebSocket server {self.websocket_endpoint}: {e}"
            )
        except Exception as e:
            self.logger.error(f"An unexpected error occurred in client_logic: {e}")
        finally:
            self.logger.info(
                "WebSocket client logic finished. Rerun the script if this was caused by an error."
            )
            self.stop_event.set()  # Ensure main thread knows if client dies


class RoboMLRESPClient(ModelClient):
    """A Redis Serialization Protocol (RESP) based client for interaction with ML models served on RoboML"""

    def __init__(
        self,
        model: Union[Model, Dict],
        host: str = "127.0.0.1",
        port: int = 6379,
        inference_timeout: int = 30,
        init_on_activation: bool = True,
        logging_level: str = "info",
        **kwargs,
    ):
        if isinstance(model, OllamaModel):
            raise TypeError(
                "An ollama model cannot be passed to a RoboML client. Please use the OllamaClient"
            )
        super().__init__(
            model=model,
            host=host,
            port=port,
            inference_timeout=inference_timeout,
            init_on_activation=init_on_activation,
            logging_level=logging_level,
            **kwargs,
        )
        try:
            from redis import Redis

            # TODO: handle timeout
            self.redis = Redis(self.host, port=self.port)
        except ModuleNotFoundError as e:
            raise ModuleNotFoundError(
                "In order to use the RESP clients, you need redis client package installed. You can install it with 'pip install redis[hiredis]'"
            ) from e
        self._check_connection()

    def _check_connection(self) -> None:
        """Check if the platfrom is being served on specified IP and port"""
        # Ping remote server to check connection
        self.logger.info("Checking connection with remote RoboML")
        try:
            self.redis.execute_command(b"PING")
        except Exception:
            self.logger.error(
                f"""Failed to connect to RoboML resp server at {self.host}:{self.port}

                Make sure an RoboML is running on the given url by executing the following command:

                `roboml-resp --host {self.host} --port {self.port}`

                To install RoboML, follow instructions on https://github.com/automatika-robotics/roboml?tab=readme-ov-file#installation
                """
            )
            raise

    def _initialize(self) -> None:
        """
        Initialize the model on platform using the paramters provided in the model specification class
        """
        # Create a model node on RoboML
        self.logger.info("Creating model node on remote")
        self.model_class = getattr(models, self.model_type)
        if issubclass(self.model_class, TransformersLLM):
            model_type = TransformersLLM.__name__
        elif issubclass(self.model_class, TransformersMLLM):
            model_type = TransformersMLLM.__name__
        else:
            model_type = self.model_type
        start_params = {"node_name": self.model_name, "node_model": model_type}
        try:
            start_params_b = msgpack.packb(start_params)
            node_init_result = self.redis.execute_command("add_node", start_params_b)
            if node_init_result:
                self.logger.debug(str(msgpack.unpackb(node_init_result)))
            self.logger.info(f"Initializing {self.model_name} on RoboML remote")
            # make initialization params
            model_dict = self.model_init_params
            # initialize model
            init_b = msgpack.packb(model_dict)
            self.redis.execute_command(f"{self.model_name}.initialize", init_b)
        except Exception as e:
            self.__handle_exceptions(e)
            raise

        # check status for init completion
        status = self.__check_model_status()
        if status == Status.READY:
            self.logger.info(
                f"{self.model_name} with {self.model_init_params['checkpoint']} model initialized"
            )
        elif status == Status.INITIALIZING:
            raise Exception(f"{self.model_name} model initialization timed out.")
        elif status == Status.INITIALIZATION_ERROR:
            raise Exception(
                f"{self.model_name} model initialization failed. Check remote for logs."
            )
        else:
            raise Exception(
                f"Unexpected Error while initializing {self.model_name}: Check remote for logs."
            )

    def _inference(self, inference_input: Dict[str, Any]) -> Optional[Dict]:
        """Call inference on the model using data and inference parameters from the component"""
        try:
            if inference_input.get("stream"):
                self.logger.warn(
                    "RoboML RESPClient currently does not handle streaming. Set stream to False in component config to get rid of this warning."
                )
                inference_input.pop("stream")
            data_b = msgpack.packb(inference_input)
            # call inference method
            result_b = self.redis.execute_command(
                f"{self.model_name}.inference", data_b
            )
            result = msgpack.unpackb(result_b)
        except Exception as e:
            return self.__handle_exceptions(e)

        self.logger.debug(str(result))

        return result

    def _deinitialize(self) -> None:
        """Deinitialize the model on the platform"""

        self.logger.error(f"Deinitializing {self.model_name} on RoboML remote")
        stop_params = {"node_name": self.model_name}
        try:
            stop_params_b = msgpack.packb(stop_params)
            self.redis.execute_command("remove_node", stop_params_b)
            self.redis.close()
        except Exception as e:
            self.__handle_exceptions(e)

    def __check_model_status(self) -> Optional[str]:
        """Check remote model node status.
        :rtype: str | None
        """
        try:
            status_b = self.redis.execute_command(f"{self.model_name}.get_status")
            status = msgpack.unpackb(status_b)
        except Exception as e:
            return self.__handle_exceptions(e)

        return status

    def __handle_exceptions(self, excep: Exception) -> None:
        """__handle_exceptions.

        :param excep:
        :type excep: Exception
        :rtype: None
        """
        from redis.exceptions import ConnectionError, ModuleError

        if isinstance(excep, ConnectionError):
            self.logger.error(
                f"{excep} RoboML server inaccessible. Might not be running. Make sure remote is correctly configured."
            )
        elif isinstance(excep, ModuleError):
            self.logger.error(
                f"{self.model_type} is not a supported model type in RoboML library. Please use another model client or another model."
            )
            raise RoboMLError(
                f"{self.model_type} is not a supported model type in RoboML library. Please use another model client or another model."
            )
        else:
            self.logger.error(str(excep))
