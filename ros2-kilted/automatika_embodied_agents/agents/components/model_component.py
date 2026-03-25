from abc import abstractmethod
import inspect
import json
import queue
import threading
from types import NoneType
from typing import Any, Optional, Sequence, Union, List, Dict, Type, MutableMapping
import msgpack


from ..clients.model_base import ModelClient
from ..clients.roboml import RoboMLWSClient
from ..config import ModelComponentConfig
from ..ros import (
    FixedInput,
    Topic,
    SupportedType,
    MutuallyExclusiveCallbackGroup,
    Event,
    component_fallback,
)
from .component_base import Component


class ModelComponent(Component):
    """ModelComponent."""

    def __init__(
        self,
        inputs: Optional[Sequence[Union[Topic, FixedInput]]] = None,
        outputs: Optional[Sequence[Topic]] = None,
        model_client: Optional[ModelClient] = None,
        config: Optional[ModelComponentConfig] = None,
        trigger: Union[Topic, List[Topic], float, Event, NoneType] = 1.0,
        component_name: str = "model_component",
        **kwargs,
    ):
        # setup model client/local model
        self.model_client = model_client if model_client else None
        self.local_model = None

        self.handled_outputs: List[Type[SupportedType]]

        if not config:
            self.config = ModelComponentConfig()

        # Initialize Component
        super().__init__(
            inputs,
            outputs,
            config,
            trigger,
            component_name,
            **kwargs,
        )

        self._additional_model_clients: Optional[Dict[str, ModelClient]] = None

    @property
    def additional_model_clients(self) -> Optional[Dict[str, ModelClient]]:
        """
        Get the dictionary of additional model clients registered to this component.

        :return: A dictionary mapping client names (str) to ModelClient instances, or None if not set.
        :rtype: Optional[Dict[str, ModelClient]]
        """
        return self._additional_model_clients

    @additional_model_clients.setter
    def additional_model_clients(self, value: Dict[str, ModelClient]) -> None:
        """
        Set additional model clients for the component.

        This property allows you to dynamically configure secondary or fallback model clients.
        These clients can be swapped in at runtime using :meth:`change_model_client`, enabling
        robustness strategies (e.g., falling back to a local model when a cloud model fails)
        or adaptive intelligence (switching models based on task complexity).

        :param value: A dictionary where keys are unique identifiers (names) for the clients and
                      values are the initialized :class:`~agents.clients.model_base.ModelClient` instances.
        :type value: Dict[str, ModelClient]

        :Example:

        ```python
            # Primary client (Cloud)
            primary_model = TransformersLLM(name="gpt4", checkpoint="gpt-4")
            primary_client = GenericHTTPClient(model=primary_model)

            # Backup client (Local)
            backup_model = OllamaModel(name="llama_local", checkpoint="llama3:8b")
            backup_client = OllamaClient(model=backup_model)

            # Initialize component with primary
            brain = LLM(..., model_client=primary_client)

            # Register backup
            brain.additional_model_clients = {"local_backup": backup_client}
        ```
        """
        self._additional_model_clients = value

    @component_fallback
    def fallback_to_local(self) -> bool:
        """Switch from remote model_client to the built-in local model at runtime.

        The local model is deployed on first call (lazy initialization) to avoid
        consuming GPU memory until actually needed. If ``enable_local_model`` is
        not already set in config, it is enabled automatically.

        This is commonly used as a target for Actions in the Event system.

        :return: True if the switch was successful, False otherwise.
        :rtype: bool

        :Example:

        ```python

            from agents.ros import Action

            # Define an action to switch to the 'local model' available in each component
            switch_to_local = Action(
                method=brain.fallback_to_local,
            )

            # Trigger this action if the component fails (e.g. internet outage)
            brain.on_component_fail(action=switch_to_local, max_retries=3)
        ```
        """
        # Auto-enable local model in config if not already set
        if hasattr(self.config, "enable_local_model"):
            self.config.enable_local_model = True

        # Deploy local model if not already loaded
        try:
            self._deploy_local_model()
        except Exception as e:
            self.get_logger().error(f"Failed to deploy local model: {e}")
            return False

        # Deinitialize remote client
        if self.model_client:
            try:
                self.model_client.deinitialize()
            except Exception as e:
                self.get_logger().warning(f"Error deinitializing model client: {e}")
            self.model_client = None

        self.get_logger().info("Switched to local model for inference.")
        return True

    @component_fallback
    def change_model_client(self, model_client_name: str) -> bool:
        """
        Hot-swap the active model client at runtime.

        This method replaces the component's current ``model_client`` with one from the
        registered ``additional_model_clients``. It handles the safe de-initialization of the
        old client and initialization of the new one.

        This is commonly used as a target for Actions in the Event system.

        :param model_client_name: The key corresponding to the desired client in ``additional_model_clients``.
        :type model_client_name: str
        :return: True if the swap was successful, False otherwise (e.g., if the name was not found or initialization failed).
        :rtype: bool

        :Example:

        ```python

            from agents.ros import Action

            # Define an action to switch to the 'remote_backup' client defined previously
            switch_to_backup = Action(
                method=brain.change_model_client,
                args=("remote_backup",)
            )

            # Trigger this action if the component fails (e.g. server down)
            brain.on_component_fail(action=switch_to_backup, max_retries=3)
        ```
        """
        if not self._additional_model_clients:
            self.get_logger().error(
                "Cannot change model client as the component was not given any additional model clients at init."
            )
            return False
        new_client = self._additional_model_clients.get(model_client_name, None)
        if not new_client:
            self.get_logger().info(
                f"No additional client named {model_client_name} is available in the component. Only the following additional clients were provided {self._additional_model_clients}"
            )
            return False

        self.get_logger().info(f"Changing model client to {model_client_name}")

        try:
            # Deinitialize any existing client
            if self.model_client:
                self.model_client.deinitialize()

            # Set the new client
            self.model_client = new_client
            self.model_client.initialize()  # initialize the new client
        except Exception:
            self.get_logger().error(
                "Error encountered during initialization when changing model client"
            )
            return False

        return True

    def custom_on_configure(self):
        """
        Create model client if provided and initialize model.
        """
        super().custom_on_configure()

        self.get_logger().debug(f"Current Status: {self.health_status.value}")

        # validate output topics if handled_outputs exist
        self.get_logger().info("Validating Model Component Output Topics")
        self._validate_output_topics()

        # get inference params
        self.inference_params = self.config.get_inference_params()

        # Initialize model
        if self.model_client:
            self.model_client.check_connection()
            self.model_client.initialize()
            if isinstance(self.model_client, RoboMLWSClient):
                # create queues and threads for the websocket client
                self.req_queue = queue.Queue()
                self.resp_queue = queue.Queue()
                self.client_stop_event = threading.Event()
                self.model_client.request_queue = self.req_queue
                self.model_client.response_queue = self.resp_queue
                self.model_client.stop_event = self.client_stop_event
                self.client_thread = threading.Thread(
                    target=self.model_client._inference,
                    name="WebSocketClientThread",
                    daemon=True,
                )
                self.client_thread.start()

                # Create a fast timer for publishing websocket client outputs
                # asynchronously in case of streaming. The callback is implemented
                # in child components and gets executed in a blocking manner
                if getattr(self.config, "stream", None):
                    self.__stream_timer = self.create_timer(
                        timer_period_sec=0.001,
                        callback=self._handle_websocket_streaming,
                        callback_group=MutuallyExclusiveCallbackGroup(),
                    )
                    self.get_logger().debug(
                        "Started timer for handling websocket streaming"
                    )
                if self.config.warmup:
                    # TODO: warmup with websockets
                    self.get_logger().warning(
                        "Warmup cannot not be called with websocket client."
                    )
            else:
                self.get_logger().debug(f"Warming up: {self.config.warmup}")
                if self.config.warmup:
                    try:
                        # call warpup twice
                        self._warmup()
                        self._warmup()
                    except Exception as e:
                        self.get_logger().error(f"Error encountered in warmup: {e}")

    def custom_on_deactivate(self):
        """
        Destroy model client if it exists
        """
        # Deinitialize model
        if self.model_client:
            self.model_client.check_connection()
            self.model_client.deinitialize()
            if isinstance(self.model_client, RoboMLWSClient):
                # stop running thread
                self.client_stop_event.set()
                self.client_thread.join(timeout=10)
                # mark any pending tasks as finished
                while not self.resp_queue.empty():
                    try:
                        self.resp_queue.get_nowait()
                        self.resp_queue.task_done()
                    except queue.Empty:
                        break
                while not self.req_queue.empty():
                    try:
                        self.req_queue.get_nowait()
                        self.req_queue.task_done()
                    except queue.Empty:
                        break

        # Destroy any stream timer if it exists
        if hasattr(self, "__stream_timer"):
            self.destroy_timer(self.__stream_timer)

    def _validate_output_topics(self) -> None:
        """
        Verify that output topics that are not handled, have pre-processing functions provided. We just check that there is a pre-processing function and do not check whether the functions have output of the corresponding type.
        """

        if hasattr(self, "publishers_dict") and hasattr(self, "handled_outputs"):
            for name, pub in self.publishers_dict.items():
                if pub.output_topic.msg_type not in self.handled_outputs and (
                    not self._external_processors
                ):
                    func_body = inspect.getsource(pub.output_topic.msg_type.convert)
                    raise TypeError(f"""{type(self).__name__} components can only handle output topics of type(s) {self.handled_outputs} automatically. Topic {name} is of type {pub.output_topic.msg_type}. EITHER provide a pre-processing function for this topic and attach it to the topic by calling the `add_publisher_preprocessor` on the component {self.node_name} OR provide a tool call that can provide structured inference output and attach it by calling `register_tool` on {self.node_name}. Make sure the output can be passed as parameter `output` to the following function:
{func_body}""")

    @property
    def warmup(self) -> bool:
        """Enable warmup of the model."""
        return self.config.warmup

    @warmup.setter
    def warmup(self, value: bool) -> None:
        """Enable warmup of the model."""
        self.config.warmup = value

    @abstractmethod
    def _create_input(self, *args, **kwargs) -> Union[Dict[str, Any], None]:
        """_create_input.

        :param args:
        :param kwargs:
        :rtype: dict[str, Any] | None
        """
        raise NotImplementedError(
            "_create_input method needs to be implemented by child components."
        )

    @abstractmethod
    def _execution_step(self, *args, **kwargs):
        """_execution_step.

        :param args:
        :param kwargs:
        """
        raise NotImplementedError(
            "_execution_step method needs to be implemented by child components."
        )

    @abstractmethod
    def _warmup(self, *args, **kwargs):
        """_warmup.

        :param args:
        :param kwargs:
        """
        raise NotImplementedError(
            "_warmup method needs to be implemented by child components."
        )

    def _deploy_local_model(self):
        """Deploy local model on demand. Override in subclasses."""
        raise NotImplementedError(
            f"{type(self).__name__} does not implement local model deployment."
        )

    @abstractmethod
    def _handle_websocket_streaming(self) -> Optional[Any]:
        """__handle_websocket_streaming.

        :param args:
        :param kwargs:
        """
        raise NotImplementedError(
            "__handle_websocket_streaming method needs to be implemented by child components."
        )

    def _call_inference(
        self, inference_input: Dict, unpack: bool = False
    ) -> Optional[MutableMapping]:
        """Call model inference"""
        if self.model_client:
            if isinstance(self.model_client, RoboMLWSClient):
                self.req_queue.put_nowait(inference_input)
                if getattr(self.config, "stream", None):
                    return
                else:
                    result = {}
                    try:
                        if not unpack:
                            result["output"] = self.resp_queue.get(
                                block=True, timeout=self.model_client.inference_timeout
                            )
                        else:
                            result["output"] = msgpack.unpackb(
                                self.resp_queue.get(
                                    block=True,
                                    timeout=self.model_client.inference_timeout,
                                )
                            )
                        return result
                    except queue.Empty:
                        self.get_logger().error(
                            "Did not receive result in websocket response queue"
                        )
                        # raise a fallback trigger via health status
                        self.health_status.set_fail_algorithm()
                        return None
            else:
                result = self.model_client.inference(inference_input)
                if not result:
                    # raise a fallback trigger via health status
                    self.health_status.set_fail_algorithm()
                return result
        elif self.local_model:
            if stream := getattr(self.config, "stream", None):
                result = self.local_model(inference_input, stream)
            else:
                result = self.local_model(inference_input)

            if not result:
                # raise a fallback trigger via health status
                self.health_status.set_fail_algorithm()
            return result
        else:
            self.get_logger().error("No model client or local model available")
            self.health_status.set_fail_component()

    def _publish(self, result: MutableMapping, **kwargs) -> None:
        """
        Publishes the given result to all registered publishers.

        :param result: A dictionary containing the data to be published.
        :type result: dict
        """
        for publisher in self.publishers_dict.values():
            publisher.publish(**result, **kwargs)

    def _update_cmd_args_list(self):
        """
        Update launch command arguments
        """
        super()._update_cmd_args_list()

        self.launch_cmd_args = [
            "--model_client",
            self._get_model_client_json(),
        ]

        self.launch_cmd_args = [
            "--additional_model_clients",
            self._get_additional_model_clients_json(),
        ]

    def _get_model_client_json(self) -> Union[str, bytes, bytearray]:
        """
        Serialize component model client to json

        :return: Serialized inputs
        :rtype:  str | bytes | bytearray
        """
        if not self.model_client:
            return ""
        return json.dumps(self.model_client.serialize())

    def _get_additional_model_clients_json(self) -> Union[str, bytes, bytearray]:
        """
        Serialize component additional model clients to json

        :return: Serialized inputs
        :rtype:  str | bytes | bytearray
        """
        if not self._additional_model_clients:
            return ""

        serialized_clients = {}
        for client_name, client in self._additional_model_clients.items():
            serialized_clients[client_name] = client.serialize()

        return json.dumps(serialized_clients)
