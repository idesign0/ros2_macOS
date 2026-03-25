from typing import Dict, Optional, Sequence, Any, Callable, Union, Tuple, List, Literal
import threading
import asyncio
import os
from attr import define, field, Factory
import json
import importlib
from functools import partial

from ..config.base_attrs import BaseAttrs
from ..core.component import BaseComponent, BaseComponentConfig, Publisher
from .. import base_clients
from ..io.callbacks import GenericCallback
from ..io.topic import Topic
from ..base_clients import (
    ServiceClientHandler,
    ActionClientHandler,
    ServiceClientConfig,
    ActionClientConfig,
)
from ..io import supported_types
from automatika_ros_sugar.srv import ChangeParameters

from rclpy.logging import get_logger
from std_msgs.msg import Header


@define
class UINodeConfig(BaseComponentConfig):
    components: Dict[str, Dict] = field(default=Factory(dict))
    port: int = field(default=5001)
    ssl_keyfile: str = field(default="key.pem")
    ssl_certificate: str = field(default="cert.pem")
    hide_settings: bool = field(default=False)


class UINode(BaseComponent):
    def __init__(
        self,
        config: UINodeConfig,
        inputs: Optional[
            Sequence[Union[Topic, ServiceClientConfig, ActionClientConfig]]
        ] = None,
        outputs: Optional[Sequence[Topic]] = None,
        component_name: str = "ui_node",
        component_configs: Optional[Dict[str, BaseComponentConfig]] = None,
        **kwargs,
    ):
        if component_configs:
            # create UI specific configs for components
            comp_configs_fields = {
                comp_name: BaseAttrs.get_fields_info(conf)
                for comp_name, conf in component_configs.items()
            }
            config.components = comp_configs_fields

        # clients for config update
        self._update_parameters_srv_client: Dict[
            str, base_clients.ServiceClientHandler
        ] = {}

        # Initialize websocket callbacks
        self.default_websocket_callback: Callable = lambda *args, **kwargs: (
            asyncio.sleep(0)
        )

        try:
            self.loop = asyncio.get_running_loop()
        except RuntimeError:
            self.loop = asyncio.new_event_loop()
            asyncio.set_event_loop(self.loop)
            self.loop_thread = threading.Thread(
                target=self.loop.run_forever, daemon=True
            )

        topic_inputs = (
            [inp for inp in inputs if isinstance(inp, Topic)] if inputs else []
        )
        self._srv_client_inputs = (
            [inp for inp in inputs if isinstance(inp, ServiceClientConfig)]
            if inputs
            else []
        )
        self._action_client_inputs = (
            [inp for inp in inputs if isinstance(inp, ActionClientConfig)]
            if inputs
            else []
        )

        super().__init__(
            component_name=f"{component_name}_{os.getpid()}",
            config=config,
            inputs=outputs,  # create listeners for outputs
            outputs=topic_inputs,  # create publishers for inputs
            **kwargs,
        )

        self._ros_service_clients: Dict[str, ServiceClientHandler] = {}
        self._ros_action_clients: Dict[str, ActionClientHandler] = {}
        # Used to store callbacks for active clients
        self._ros_action_clients_feedback_callbacks: Dict[str, Callable] = {}
        self._ros_action_clients_feedback_timers: Dict = {}

        self.config: UINodeConfig

    def srv_clients_inputs_dicts(self) -> List[Dict]:
        """Get a list of all given service clients inputs as dictionaries

        :return: Service clients config dictionaries
        :rtype: List[Dict]
        """
        clients_configs_dicts = []
        for client_config in self._srv_client_inputs:
            request_fields: Dict[str, Union[Dict, str]] = (
                supported_types.get_ros_msg_fields_dict(client_config.srv_type.Request)
            )
            config_dict = {
                "name": client_config.name,
                "type": client_config.srv_type.__name__,
                "fields": request_fields,
            }
            clients_configs_dicts.append(config_dict)
        return clients_configs_dicts

    def action_clients_inputs_dicts(self) -> List[Dict]:
        """Get a list of all given action clients inputs as dictionaries

        :return: Service clients config dictionaries
        :rtype: List[Dict]
        """
        clients_configs_dicts = []
        for client_config in self._action_client_inputs:
            request_fields: Dict[str, Union[Dict, str]] = (
                supported_types.get_ros_msg_fields_dict(client_config.action_type.Goal)
            )
            config_dict = {
                "name": client_config.name,
                "type": client_config.action_type.__name__,
                "fields": request_fields,
            }
            clients_configs_dicts.append(config_dict)
        return clients_configs_dicts

    def get_active_action_clients(self) -> Dict[str, ActionClientHandler]:
        return self._ros_action_clients

    @property
    def _client_inputs_json(self) -> str:
        """Serialize service clients configs

        :return: serialized_clients_configs
        :rtype: str
        """
        serialized_srv_clients_configs = []
        for client_config in self._srv_client_inputs:
            # Parse the interface package name
            service_module = client_config.srv_type.__module__.split(".srv")[0]
            # Get the service name
            service_type = client_config.srv_type.__name__
            config_dict = {
                "service_module": service_module,
                "service_type": service_type,
                "service_name": client_config.name,
            }
            serialized_srv_clients_configs.append(json.dumps(config_dict))

        # Serialize action clients (if any)
        serialized_action_clients_configs = []
        for client_config in self._action_client_inputs:
            # Parse the interface package name
            action_module = client_config.action_type.__module__.split(".action")[0]
            # Get the service name
            action_type = client_config.action_type.__name__
            config_dict = {
                "action_module": action_module,
                "action_type": action_type,
                "action_name": client_config.name,
            }
            serialized_action_clients_configs.append(json.dumps(config_dict))
        clients_dict = {
            "services": serialized_srv_clients_configs,
            "actions": serialized_action_clients_configs,
        }

        return json.dumps(clients_dict)

    @_client_inputs_json.setter
    def _client_inputs_json(self, clients_json: str):
        """Parse ServiceClientConfig and ActionClientConfig from serialized configs

        :param clients_json: Serialized configs
        :type clients_json: str
        """
        all_clients_serialized = json.loads(clients_json)
        self._srv_client_inputs = []
        for client_serialized in all_clients_serialized["services"]:
            client_dict: dict = json.loads(client_serialized)
            try:
                # Get the service name and module name
                module_name = client_dict.get("service_module")
                service_type_name = client_dict.get("service_type")
                name = client_dict.get("service_name")

                # Import the service class
                module = importlib.import_module(f"{module_name}.srv")
                service_type = getattr(module, service_type_name)

                # Re-create the service config
                service_config = ServiceClientConfig(srv_type=service_type, name=name)
                self._srv_client_inputs.append(service_config)

            except Exception as e:
                get_logger(self.node_name).warning(
                    f"Error parsing service clients inputs: {e}"
                )

        self._action_client_inputs = []
        for client_serialized in all_clients_serialized["actions"]:
            client_dict: dict = json.loads(client_serialized)
            try:
                # Get the service name and module name
                module_name = client_dict.get("action_module")
                action_type_name = client_dict.get("action_type")
                name = client_dict.get("action_name")

                # Import the service class
                module = importlib.import_module(f"{module_name}.action")
                action_type = getattr(module, action_type_name)

                # Re-create the service config
                action_config = ActionClientConfig(action_type=action_type, name=name)
                self._action_client_inputs.append(action_config)

            except Exception as e:
                get_logger(self.node_name).warning(
                    f"Error parsing action clients inputs: {e}"
                )

    def _return_error(self, error_msg: str):
        """Return error msg to the UI"""
        self.get_logger().error(error_msg)
        payload = {"type": "error", "payload": error_msg}
        asyncio.run_coroutine_threadsafe(
            self.default_websocket_callback(payload), self.loop
        )

    def _add_ros_subscriber(self, callback: GenericCallback):
        """Overrides creating subscribers to run the ui callback instead of the main callback
        :param callback:
        :type callback: GenericCallback
        """
        payload = {
            "type": callback.input_topic.msg_type.__name__,
            "topic": callback.input_topic.name,
        }

        setattr(self, f"{payload['type']}_callback", None)

        def _ui_callback(msg) -> None:
            ws_callback = (
                getattr(self, f"{payload['type']}_callback")
                or self.default_websocket_callback
            )
            callback.msg = msg
            if hasattr(msg, "header") and isinstance(msg.header, Header):
                callback._frame_id = msg.header.frame_id
            try:
                ui_content = callback._get_ui_content()
                payload["payload"] = ui_content
            except Exception as e:
                return self._return_error(f"Topic callback error: {e}")
            asyncio.run_coroutine_threadsafe(ws_callback(payload, msg=msg), self.loop)

        _subscriber = self.create_subscription(
            msg_type=callback.input_topic.ros_msg_type,
            topic=callback.input_topic.name,
            qos_profile=callback.input_topic.qos_profile.to_ros(),
            callback=_ui_callback,
            callback_group=self.callback_group,
        )
        self.get_logger().debug(
            f"Started subscriber to topic: {callback.input_topic.name} of type {callback.input_topic.msg_type.__name__}"
        )
        return _subscriber

    def custom_on_activate(self):
        """Custom activation configuration"""
        # Setup settings updater clients
        if self.config.components:
            for component_name in self.config.components:
                self._update_parameters_srv_client[component_name] = (
                    base_clients.ServiceClientHandler(
                        client_node=self,
                        srv_type=ChangeParameters,
                        srv_name=f"{component_name}/update_config_parameters",
                    )
                )

        # Initialize all input service clients (if any)
        for inp in self._srv_client_inputs:
            self._ros_service_clients[inp.name] = base_clients.ServiceClientHandler(
                client_node=self, config=inp
            )

        # Initialize all input service clients (if any)
        for inp in self._action_client_inputs:
            self._ros_action_clients[inp.name] = base_clients.ActionClientHandler(
                client_node=self, config=inp
            )

        # Start loop thread if necessary
        if hasattr(self, "loop_thread"):
            self.loop_thread.start()

        return super().custom_on_activate()

    def custom_on_deactivate(self):
        """Custom deactivation configuration"""
        for inp in self._update_parameters_srv_client.values():
            if inp.client is not None:
                self.destroy_client(inp.client)

        for inp in self._ros_service_clients.items():
            if inp.client is not None:
                self.destroy_client(inp.client)
                inp.client = None

        for inp in self._ros_action_clients:
            if inp.client is not None:
                self.destroy_client(inp.client)
                inp.client = None

        return super().custom_on_deactivate()

    def attach_websocket_callback(
        self, ws_callback: Callable, topic_type: Optional[str] = None
    ):
        """Adds websocket callback to listeners of outputs"""
        if topic_type:
            setattr(self, f"{topic_type}_callback", ws_callback)
        else:
            self.default_websocket_callback = ws_callback

    def update_configs(self, new_configs: Dict):
        self.get_logger().debug("Updating configs")
        component_name = new_configs.pop("component_name")

        srv_request = ChangeParameters.Request()
        srv_request.names = list(new_configs.keys())
        srv_request.values = list(new_configs.values())
        srv_request.keep_alive = False  # restart component

        result = self._update_parameters_srv_client[component_name].send_request(
            req_msg=srv_request
        )
        return result

    def attach_client_feedback_callback(self, ws_callback, action_name: str):
        if self._ros_action_clients.get(action_name, None):
            self._ros_action_clients_feedback_callbacks[action_name] = ws_callback

    def send_srv_call(self, srv_call_data: Dict) -> Tuple[bool, Any]:
        """
        Send a service call using the service request form data
        """
        srv_name = srv_call_data.pop("srv_name")
        if srv_name not in self._ros_service_clients:
            return (False, f'Service client "{srv_name}" not found!')

        try:
            output = self._ros_service_clients[srv_name].send_request_from_dict(
                request_fields=srv_call_data
            )
        except Exception as e:
            return (
                False,
                f'Error occurred when sending service request to "{srv_name}": {e}',
            )

        if output:
            # Parse response to display on UI
            response_fields = self._ros_service_clients[
                srv_name
            ].client.srv_type.Response.get_fields_and_field_types()
            # Format a response string with fields dictionary keys and actual response values
            response_str = ""
            for key in response_fields.keys():
                response_str += f"{key} = {getattr(output, key)}, "
            return (True, response_str)
        return (
            False,
            f'Server Error - Service "{srv_name}" request send but no service response received',
        )

    def send_action_goal(self, action_goal_data: Dict) -> Tuple[bool, Any]:
        """
        Send a service call using the service request form data
        """
        action_name = action_goal_data.pop("action_name")
        if action_name not in self._ros_action_clients:
            return (False, f'Action client "{action_name}" not found!')

        try:
            sent_done = self._ros_action_clients[action_name].send_request_from_dict(
                request_fields=action_goal_data, wait_until_first_feedback=False
            )
        except Exception as e:
            return (
                False,
                f'Error occurred when sending service request to "{action_name}": {e}',
            )

        if sent_done:
            # If goal is sent, start a timer to send the feedback to the websocket
            self._ros_action_clients_feedback_timers[action_name] = self.create_timer(
                timer_period_sec=1 / self.config.loop_rate,
                callback=partial(
                    self._action_feedback_callback, action_name=action_name
                ),
            )
            return (True, f"Starting requested action {action_name}...")
        return (
            False,
            f'Server Error - Was not able to send goal for action "{action_name}"',
        )

    async def _action_feedback_callback(self, action_name: str):
        """Get feedback message from action (if available)

        :param action_name: Action name
        :type action_name: str
        :return: Feedback Info
        :rtype: Optional[str]
        """
        if action_name not in self._ros_action_clients:
            return
        feedback_data = self._ros_action_clients[action_name].get_ui_elements()
        if feedback_func := self._ros_action_clients_feedback_callbacks.get(
            action_name, None
        ):
            await feedback_func(feedback_data)

    def cancel_action(self, action_name: str) -> Tuple[bool, str]:
        """Cancel ongoing action goal

        :param action_name: Action name
        :type action_name: str
        :return: If action is cancelled, Log message
        :rtype: (bool, str)
        """
        if action_name not in self._ros_action_clients:
            return (
                True,
                f"Action cancellation is not possible: '{action_name}' is not found",
            )
        return self._ros_action_clients[action_name].cancel_request()

    def publish_data(self, data: Any):
        """
        Publish data to input topics if any
        """
        topic_name = data.pop("topic_name")
        topic_type_str = data.pop("topic_type")
        frame_id = data.pop("frame_id", None)
        topic_type = getattr(supported_types, topic_type_str, None)

        if not topic_type:
            return self._return_error(
                f'Data type "{topic_type_str}" not found in supported types. Make sure the UI element is created correctly'
            )

        if self.count_subscribers(topic_name) == 0:
            return self._return_error(
                f'No subscribers found for the topic "{topic_name}". Please check the topic name and re-send data'
            )

        try:
            output = topic_type.convert_ui_dict(
                data
            )  # Convert to publisher compatible data
        except NotImplementedError:
            return self._return_error(
                f'Data type "{topic_type_str}" does not implement a converter'
            )
        except Exception as e:
            return self._return_error(
                f'Error occurred when converting {data} to Sugar type "{topic_type_str}": {e}'
            )
        kwargs = {"output": output}

        # Add the data frame_id if it is sent
        if frame_id:
            kwargs["frame_id"] = frame_id

        try:
            if publisher := self.publishers_dict.get(topic_name, None):
                # Confirm that the topic type was not updated dynamically in the UI
                if publisher.output_topic.msg_type.__name__ == topic_type_str:
                    publisher.publish(**kwargs)
                    return
                else:
                    # Destroy the old publisher to create a new one
                    self.destroy_publisher(publisher._publisher)
                    self.get_logger().debug(
                        f"Destroying old publisher for {topic_name} of type {publisher.output_topic.msg_type.__name__}"
                    )
            # Handle creating publishers for dynamically added or modified outputs in the UI
            self.publishers_dict[topic_name] = Publisher(
                Topic(name=topic_name, msg_type=topic_type_str),
                node_name=self.node_name,
            )
            self.publishers_dict[topic_name].set_node_name(self.node_name)
            # Set ROS publisher for each output publisher
            self.publishers_dict[topic_name].set_publisher(
                self._add_ros_publisher(self.publishers_dict[topic_name])
            )
            self.publishers_dict[topic_name].publish(**kwargs)
            return
        except Exception as e:
            self.get_logger().error(
                f"Error publishing UI input data to topic {topic_name}: {e}"
            )

    def _execution_step(self):
        """
        Main execution of the component, executed at each timer tick with rate 'loop_rate' from config
        """
        pass
