"""Base Component"""

import os
import time
import json
import yaml
import toml
import socket
from abc import abstractmethod
from copy import deepcopy
import threading
from typing import Any, Dict, List, Optional, Union, Callable, Sequence, Tuple, Type
from functools import wraps, partial
import importlib

from rclpy import logging as rclpy_logging
from rclpy.action.server import ActionServer, CancelResponse, GoalResponse
from rclpy.utilities import try_shutdown
import rclpy.callback_groups as ros_callback_groups
from rclpy.callback_groups import MutuallyExclusiveCallbackGroup, ReentrantCallbackGroup
from rclpy import lifecycle
from rclpy.lifecycle.node import TransitionCallbackReturn, LifecycleState
from rclpy.publisher import Publisher as ROSPublisher
from rclpy.subscription import Subscription
from rclpy.client import Client
from tf2_ros.transform_listener import TransformListener
from builtin_interfaces.msg import Time
from lifecycle_msgs.msg import State as LifecycleStateMsg

from automatika_ros_sugar.srv import (
    ChangeParameter,
    ChangeParameters,
    ConfigureFromFile,
    ReplaceTopic,
    ExecuteMethod,
)

from .action import Action
from .event import Event, EventBlackboardEntry
from ..io.callbacks import GenericCallback
from ..config.base_config import (
    BaseComponentConfig,
    ComponentRunType,
    ExternalProcessorType,
    BaseAttrs,
    QoSConfig,
)
from ..io.topic import Topic
from ..io.supported_types import SupportedType
from ..io.publisher import Publisher
from .fallbacks import ComponentFallbacks, Fallback
from .status import Status
from ..utils import (
    camel_to_snake_case,
    component_fallback,
    component_action,
    get_methods_with_decorator,
    log_srv,
)
from ..base_clients import ActionClientConfig
from ..robot_plugin import RobotPluginServiceClient
from ..tf import TFListener, TFListenerConfig


class BaseComponent(lifecycle.Node):
    def __init__(
        self,
        component_name: str,
        inputs: Optional[Sequence[Topic]] = None,
        outputs: Optional[Sequence[Topic]] = None,
        config: Optional[BaseComponentConfig] = None,
        config_file: Optional[str] = None,
        callback_group: Optional[ros_callback_groups.CallbackGroup] = None,
        fallbacks: Optional[ComponentFallbacks] = None,
        main_action_type: Optional[type] = None,
        main_srv_type: Optional[type] = None,
        **kwargs,
    ):
        """Initialize a component

        :param component_name: ROS2 node name
        :type component_name: str
        :param inputs: Component input topics, defaults to None
        :type inputs: Optional[Sequence[Topic]], optional
        :param outputs: Component output topics, defaults to None
        :type outputs: Optional[Sequence[Topic]], optional
        :param config: Component config, defaults to None
        :type config: Optional[BaseComponentConfig], optional
        :param config_file: Path to configuration file (yaml, json, toml), defaults to None
        :type config_file: Optional[str], optional
        :param callback_group: Main callback group, defaults to None
        :type callback_group: rclpy.callback_groups.CallbackGroup, optional
        :param fallbacks: Component fallbacks, defaults to None
        :type fallbacks: Optional[ComponentFallbacks], optional
        :param main_action_type: Component main ROS2 action server type (Used when the component is running as an ActionServer), defaults to None
        :type main_action_type: Optional[type], optional
        :param main_srv_type: Component main ROS2 service type (Used when the component is running as a Server), defaults to None
        :type main_srv_type: Optional[type], optional
        """
        # Component health status - Inits with healthy status
        self.health_status = Status()

        # Setup Config
        self.config: BaseComponentConfig = config or BaseComponentConfig()

        # Set callback group
        self.callback_group = callback_group or ReentrantCallbackGroup()

        # SET NAME AND CALLBACK GROUP
        self.node_name = component_name

        # Setup the launch command-line arguments list
        self._cmd_line_kwargs_list = []

        # List to keep all node clients
        self.clients_list = []

        # setup inputs and outputs
        self.callbacks: Dict[str, GenericCallback] = {}
        if inputs:
            self.in_topics = self._reparse_inputs_callbacks(inputs)
            self.callbacks = {
                input.name: input.msg_type.callback(input, node_name=self.node_name)
                for input in self.in_topics
            }

        self.publishers_dict: Dict[str, Publisher] = {}
        if outputs:
            self.out_topics = self._reparse_outputs_converts(outputs)
            self.publishers_dict = {
                output.name: Publisher(output, node_name=self.node_name)
                for output in self.out_topics
            }

        self._config_file = config_file

        self.__fallbacks = fallbacks or ComponentFallbacks()
        self.__fallbacks_giveup: bool = False
        self.__fallbacks_listeners: List[Subscription] = []
        # Blackboard to store latest messages for all topics required for all fallbacks
        # {'topic_1_name': RosMsg, 'topic_2_name': ROSMsg, ... }
        self._fallbacks_topics_blackboard: Dict[str, EventBlackboardEntry] = {}
        self._fallbacks_topics_timeout: Dict[str, float] = {}

        if self.config._use_without_launcher:
            # Create default services for changing config/inputs/outputs during runtime
            self._create_default_services()

        self.action_type = main_action_type
        self.service_type = main_srv_type
        self._external_processors: Dict[
            str, Tuple[List[Union[Callable, socket.socket]], ExternalProcessorType]
        ] = {}

        self.__events: Optional[List[Event]] = None
        self.__actions: Optional[List[List[Action]]] = None
        self.__event_listeners: List[Subscription] = []

        # To manage algorithms config
        self._algorithms_config: Dict[
            str, Dict
        ] = {}  # Dictionary of user defined algorithms configuration

        # Health status topic
        self.__health_status_topic = Topic(
            name=f"{self.node_name}/status", msg_type="ComponentStatus"
        )

        # Main goal handle (to execute one goal at a time)
        # TODO: add config parameter (one goal vs goal queue)
        self._main_goal_handle = None
        self._main_goal_lock = threading.Lock()

        # Additional types from derived packages
        self._additional_types: List[Type[SupportedType]] = []

        # To use without launcher -> Init the ROS2 node directly
        if self.config._use_without_launcher:
            self.rclpy_init_node(component_name, **kwargs)

    def rclpy_init_node(self, *args, **kwargs):
        """
        To init the node with rclpy and activate default services
        """
        # Apply Logging Level
        rclpy_logging.set_logger_level(
            self.node_name,
            rclpy_logging.get_logging_severity_from_string(self.config.log_level),
        )
        # Activate Node
        lifecycle.Node.__init__(self, self.node_name, *args, **kwargs)
        self.get_logger().info(
            f"LIFECYCLE NODE {self.get_name()} STARTED AND REQUIRES CONFIGURATION"
        )

    def is_node_initialized(self) -> bool:
        """Checks if the rclpy Node is initialized

        :return: Is node initialized
        :rtype: bool
        """
        from rclpy.utilities import ok

        return ok()

    def _reparse_inputs_callbacks(self, inputs: Sequence[Topic]) -> Sequence[Topic]:
        """Select inputs callbacks. Selects a callback for each input from the same component package if it exists. Otherwise, the first available callback will be assigned. Note: This method is added to enable using components from multiple packages in the same script, where each component prioritizes using callbacks from its own package.

        :param inputs: Input topics
        :type inputs: List[Topic]
        :return: Input topics with selected callbacks
        :rtype: List[Topic]
        """
        for inp in inputs:
            if not inp or not isinstance(inp.msg_type.callback, List):
                continue
            module_name = (
                self.__module__[: self.__module__.index(".")]
                if self.__module__.index(".") > -1
                else self.__module__
            )
            # Get first callback by default
            selected_callback = inp.msg_type.callback[0]
            for callback in inp.msg_type.callback:
                msg_module = (
                    callback.__module__[: callback.__module__.index(".")]
                    if callback.__module__.index(".") > -1
                    else ""
                )
                if msg_module == module_name:
                    selected_callback = callback
                    break
            inp.msg_type.callback = selected_callback
        return inputs

    def _reparse_outputs_converts(self, outputs: Sequence[Topic]) -> Sequence[Topic]:
        """Select outputs converters. Selects a converter for each output from the same component package if it exists. Otherwise, the first available converter will be assigned. Note: This method is added to enable using components from multiple packages in the same script, where each component prioritizes using converters from its own package.

        :param outputs: Output topics
        :type outputs: List[Topic]
        :return: Output topics with selected converters
        :rtype: List[Topic]
        """
        for out in outputs:
            if not out or not isinstance(out.msg_type.convert, List):
                continue
            module_name = self.__module__
            # Get first callback by default
            selected_convert = out.msg_type.convert[0]
            for conv in out.msg_type.convert:
                msg_module = (
                    conv.__module__[: conv.__module__.index(".")]
                    if conv.__module__.index(".") > -1
                    else ""
                )
                if msg_module == module_name:
                    selected_convert = conv
                    break
            out.msg_type.convert = selected_convert
        return outputs

    def _use_robot_plugin(self):
        """Replace the component inputs/outputs with topics or clients provided by the robot plugin

        :raises ModuleNotFoundError: If the robot plugin module is not found
        :raises AttributeError: If the robot plugin module is malformed or missing required 'robot_feedback' or 'robot_action' dictionaries
        """
        # Load the plugin module and its definitions
        try:
            robot_plugins = importlib.import_module(self.config._robot_plugin)
            robot_feedback_plugins: Dict[str, Topic] = robot_plugins.robot_feedback
            robot_action_plugins: Dict[str, Union[Topic, RobotPluginServiceClient]] = (
                robot_plugins.robot_action
            )
            self.get_logger().info(
                f"Successfully loaded robot plugin: '{self.config._robot_plugin}'"
            )
        except ModuleNotFoundError as e:
            raise ModuleNotFoundError(
                f"Robot Plugin is enabled for component '{self.node_name}', "
                f"but no plugin with name '{self.config._robot_plugin}' is found. "
                "Install the plugin or disable the plugin use in this component"
            ) from e
        except AttributeError as e:
            raise AttributeError(
                f"Robot plugin '{self.config._robot_plugin}' is malformed or "
                f"missing required 'robot_feedback' or 'robot_action' dictionaries: {e}"
            ) from e

        # Handle Robot Feedback (System Input Topics)
        if self.config._enable_plugin_feedbacks_handling:
            for topic in self.in_topics:
                msg_type_name = topic.msg_type.__name__
                # find a matching plugin by message type
                plugin_value = robot_feedback_plugins.get(msg_type_name)

                if isinstance(plugin_value, Topic):
                    if topic.name != plugin_value.name:
                        self.get_logger().warn(
                            f"Input Topic is given with name '{topic.name}' ({msg_type_name}), "
                            f"but robot plugin provides default name '{plugin_value.name}' ({plugin_value.msg_type.__name__})."
                        )
                        self.get_logger().warn(
                            f"Robot Plugin Topic Name will be Used! To change it, replace the name in '{self.config._robot_plugin}'"
                        )
                    self._replace_input_topic(
                        topic.name, plugin_value.name, plugin_value.msg_type
                    )

        # Handle Actions (Output Topics)
        if self.config._enable_plugin_actions_handling:
            for topic in self.out_topics:
                msg_type_name = topic.msg_type.__name__
                plugin_value = robot_action_plugins.get(msg_type_name)

                if isinstance(plugin_value, Topic):
                    if topic.name != plugin_value.name:
                        self.get_logger().warn(
                            f"Input Topic is given with name '{topic.name}' ({msg_type_name}), "
                            f"but robot plugin provides default name '{plugin_value.name}' ({plugin_value.msg_type.__name__})."
                        )
                        self.get_logger().warn(
                            f"Robot Plugin Topic Name will be Used! To change it, replace the name in '{self.config._robot_plugin}'"
                        )
                    self._replace_output_topic(
                        topic.name, plugin_value.name, plugin_value.msg_type
                    )
                elif plugin_value and issubclass(
                    plugin_value, RobotPluginServiceClient
                ):
                    # Check for plugin_value truthiness before issubclass to avoid error on None
                    self.get_logger().info(
                        f"Plugin provides a client for type '{topic.msg_type}': Replacing output topic '{topic.name}' ({msg_type_name}) "
                        f"with a plugin service client."
                    )
                    # Instantiate the client class, passing 'self' (the component)
                    client_instance = plugin_value(self)
                    self._replace_output_by_client(topic.name, client_instance)

    # Managing algorithms
    @property
    def algorithms_config(self) -> Dict:
        """
        Getter of the user defined algorithms config types

        :return: Algorithms configurations types
        :rtype: List[type]
        """
        return self._algorithms_config

    @algorithms_config.setter
    def algorithms_config(self, configs: Union[BaseAttrs, List[BaseAttrs]]):
        """
        Setter of robot configuration

        :param config: Robot configuration
        :type config: RobotConfig
        """
        if not isinstance(configs, List):
            configs = [configs]
        for config in configs:
            # Add new config
            self._algorithms_config[config.__class__.__name__] = config.asdict()

    def _configure_algorithm(self, algo_config: BaseAttrs) -> BaseAttrs:
        """Configure an algorithm from the user defined configuration classes

        :param algo_config: Algorithm base configuration class
        :type algo_config: BaseAttrs
        :return: Updated algorithm configuration or default
        :rtype: BaseAttrs
        """
        algo_config_name = algo_config.__class__.__name__
        if algo_config_name in self.algorithms_config.keys():
            config_dict = self.algorithms_config[algo_config_name]
            algo_config.from_dict(config_dict)
        elif self._config_file:
            # configure directly from file if available
            algo_config.from_file(
                self._config_file,
                nested_root_name=f"{self.node_name}.{algo_config_name.partition('Config')[0]}",
            )
        return algo_config

    @property
    def ui_main_action_input(self) -> Optional[ActionClientConfig]:
        """Get a UI input for the the component's main action server (if present)

        :return: Client config for the component's main action
        :rtype: Optional[ActionClientConfig]
        """
        if self.main_action_name and self.action_type:
            return ActionClientConfig(
                action_type=self.action_type, name=self.main_action_name
            )
        return None

    @property
    def status_topic(self) -> Topic:
        """Get the component health status topic

        :return: Health status topic
        :rtype: Topic
        """
        return self.__health_status_topic

    # Managing Inputs/Outputs
    def _add_ros_subscriber(self, callback: GenericCallback):
        """Creates a subscriber to be attached to an input message.

        :param msg:
        :type msg: Input
        :param callback:
        :type callback: GenericCallback
        """
        _subscriber = self.create_subscription(
            msg_type=callback.input_topic.ros_msg_type,
            topic=callback.input_topic.name,
            qos_profile=callback.input_topic.qos_profile.to_ros(),
            callback=callback.callback,
            callback_group=self.callback_group,
        )
        self.get_logger().debug(
            f"Started subscriber to topic: {callback.input_topic.name} of type {callback.input_topic.msg_type}"
        )
        return _subscriber

    def _add_ros_publisher(self, publisher: Publisher) -> ROSPublisher:
        """
        Sets the publisher attribute of a component for a given Topic
        """
        qos_profile = publisher.output_topic.qos_profile.to_ros()
        return self.create_publisher(
            publisher.output_topic.ros_msg_type,
            publisher.output_topic.name,
            qos_profile,
        )

    def _pre_post_processor_closure(self, func: Callable) -> Callable:
        """Wrapper for external pre and post processors
        Ensures that external functions get passed only one argument
        """

        @wraps(func)
        def _wrapper(*, output, **_):
            """_wrapper"""
            return func(output)

        _wrapper.__name__ = func.__name__
        return _wrapper

    def attach_custom_callback(self, input_topic: Topic, func: Callable) -> None:
        """
        Method to attach custom method to subscriber callbacks
        """
        if not callable(func):
            raise TypeError(f"A custom callback must be a Callable, got {type(func)}")
        if callback := self.callbacks.get(input_topic.name):
            if not callback:
                raise TypeError("Specified input topic does not exist")
            callback.on_callback_execute(func)

    def add_callback_postprocessor(self, input_topic: Topic, func: Callable) -> None:
        """Adds a callable as a post processor for topic callback.
        :param input_topic:
        :type input_topic: Topic
        :param callable:
        :type func: Callable
        """
        if not callable(func):
            raise TypeError(
                "A postprocessor must be a Callable with input and output types the same as the topic."
            )
        if callback := self.callbacks.get(input_topic.name):
            if not callback:
                raise TypeError("Specified input topic does not exist")

            if self._external_processors.get(input_topic.name):
                self._external_processors[input_topic.name][0].append(
                    self._pre_post_processor_closure(func)
                )
            else:
                self._external_processors[input_topic.name] = (
                    [self._pre_post_processor_closure(func)],
                    ExternalProcessorType.MSG_POST_PROCESSOR,
                )

    def add_publisher_preprocessor(self, output_topic: Topic, func: Callable) -> None:
        """Adds a callable as a pre processor for topic publisher.
        :param output_topic:
        :type output_topic: Topic
        :param callable:
        :type func: Callable
        """
        if not callable(func):
            raise TypeError(
                "A preprocessor must be a Callable with input and output types the same as the topic."
            )
        if self.publishers_dict:
            if publisher := self.publishers_dict.get(output_topic.name):
                if not publisher:
                    raise TypeError("Specified output topic does not exist")
            if self._external_processors.get(output_topic.name):
                self._external_processors[output_topic.name][0].append(
                    self._pre_post_processor_closure(func)
                )
            else:
                self._external_processors[output_topic.name] = (
                    [self._pre_post_processor_closure(func)],
                    ExternalProcessorType.MSG_PRE_PROCESSOR,
                )
        else:
            raise TypeError(
                "The component does not have any output topics specified. Add output topics with Component.outputs method"
            )

    # TRANSITIONS
    def activate(self):
        """
        Create required subscriptions, publications, timers, ... etc. to activate the node
        """
        # Init any global node variables
        self.init_variables()

        # Update the component topics using the robot plugin if provided
        if self.config._robot_plugin:
            self._use_robot_plugin()

        self.create_all_subscribers()

        self.create_all_publishers()

        # Setup node services: servers and clients
        self.create_all_services()

        self.create_all_service_clients()

        # Setup node actions: servers and clients
        self.create_all_action_servers()

        self.create_all_action_clients()

        self._turn_on_fallbacks_subscribers()

        # Setup node timers
        self.create_all_timers()

    def deactivate(self):
        """
        Destroy all declared subscriptions, publications, timers, ... etc. to deactivate the node
        """
        self.destroy_all_timers()

        self.destroy_all_action_servers()

        self.destroy_all_services()

        self.destroy_all_action_clients()

        self.destroy_all_service_clients()

        self.destroy_all_subscribers()

        self.destroy_all_publishers()

    def configure(self, config_file: Optional[str] = None):
        """
        Configure component from configuration file

        :param config_file: Path to file with configuration (yaml, json or toml), defaults to None
        :type config_file: str
        """
        config_file = config_file or self._config_file
        if config_file:
            self.config_from_file(config_file)

    # CREATION AND DESTRUCTION METHODS
    def init_variables(self):
        """
        Set up node variables
        """
        pass

    def create_all_subscribers(self):
        """
        Creates all node subscribers from component inputs
        """
        self.get_logger().info("STARTING ALL SUBSCRIBERS")
        # Create subscribers
        for callback in self.callbacks.values():
            callback.set_node_name(self.node_name)
            callback.set_subscriber(self._add_ros_subscriber(callback))

    def create_all_publishers(self):
        """
        Creates all node publishers from component outputs
        """
        self.get_logger().info("STARTING ALL PUBLISHERS")
        # Create status publisher
        self.health_status_publisher: ROSPublisher = self.create_publisher(
            msg_type=self.__health_status_topic.ros_msg_type,
            topic=self.__health_status_topic.name,
            qos_profile=1,
        )
        # Create publisher and attach it to output publisher object
        for publisher in self.publishers_dict.values():
            if isinstance(publisher, Publisher):
                publisher.set_node_name(self.node_name)
                # Set ROS publisher for each output publisher
                publisher.set_publisher(self._add_ros_publisher(publisher))

    def create_all_timers(self):
        """
        Creates all node timers
        """
        # If component is not used as a server start the main execution timer
        self.get_logger().info("CREATING MAIN TIMER")
        self._execution_timer = self.create_timer(
            timer_period_sec=1 / self.config.loop_rate,
            callback=self._main,
            callback_group=MutuallyExclusiveCallbackGroup(),
        )

    def create_all_action_servers(self):
        """
        Action servers creation
        """
        if self.run_type != ComponentRunType.ACTION_SERVER:
            return
        if not self.action_type:
            raise TypeError(
                f"Cannot start component '{self.node_name}' as an ActionServer without specifying 'action_type'"
            )
        action_name = self.main_action_name
        # callback group to avoid parallel execution of action loops
        action_callback_group = MutuallyExclusiveCallbackGroup()
        self.action_server = ActionServer(
            node=self,
            action_type=self.action_type,
            action_name=action_name,
            execute_callback=self.main_action_callback,
            goal_callback=self._main_action_goal_callback,
            handle_accepted_callback=self._main_action_handle_accepted_callback,
            cancel_callback=self._main_action_cancel_callback,
            callback_group=action_callback_group,
        )

    def create_all_action_clients(self):
        """
        Creates all node action clients
        """
        pass

    def create_all_services(self):
        """
        Services creation
        """
        if (
            not hasattr(self, "_maintain_default_services")
            or not self._maintain_default_services
        ):
            # Default services were not maintained during a restart or never created
            self._create_default_services()
        if hasattr(self, "_maintain_default_services"):
            # Reset the flag
            self._maintain_default_services = False

        if self.run_type != ComponentRunType.SERVER:
            return
        if not self.service_type:
            raise TypeError(
                f"Cannot start component '{self.node_name}' as a Server without specifying 'service_type'"
            )
        srv_name = self.main_srv_name
        if not srv_name:
            raise TypeError(
                f"Cannot start component '{self.node_name}' as a Server without specifying 'service_type'"
            )
        self.server = self.create_service(
            self.service_type,
            srv_name,
            self.main_service_callback,
        )

    def create_all_service_clients(self):
        """
        Creates all node service clients
        """
        pass

    def destroy_all_timers(self):
        """
        Destroys all node timers
        """
        if hasattr(self, "_execution_timer"):
            self.get_logger().info("DESTROYING MAIN TIMER")
            self.destroy_timer(self._execution_timer)

    def destroy_all_subscribers(self):
        """
        Destroys all node subscribers
        """
        self.get_logger().info("DESTROYING ALL SUBSCRIBERS")
        for listener in self.__event_listeners:
            self.destroy_subscription(listener)
        for listener in self.__fallbacks_listeners:
            self.destroy_subscription(listener)
        # Destroy all input subscribers
        for callback in self.callbacks.values():
            if callback._subscriber:
                self.destroy_subscription(callback._subscriber)
                callback._subscriber = None

    def destroy_all_publishers(self):
        """
        Destroys all node publishers
        """
        self.get_logger().info("DESTROYING ALL PUBLISHERS")
        # Destroy health status publisher
        self.destroy_publisher(self.health_status_publisher)

        for publisher in self.publishers_dict.values():
            if publisher._publisher:
                self.destroy_publisher(publisher._publisher)

    def destroy_all_services(self):
        """
        Destroys all node services
        """
        # Destroy node main Server if runtype is server
        if self.run_type == ComponentRunType.SERVER:
            self.destroy_service(self.server)
        if (
            hasattr(self, "_maintain_default_services")
            and self._maintain_default_services
        ):
            # Do not destroy default services
            return

        for srv in self._default_services:
            self.destroy_service(srv)

    def destroy_all_action_servers(self):
        """
        Destroys all action servers
        """
        # Destroy node main Server if runtype is action server
        if self.run_type == ComponentRunType.ACTION_SERVER and hasattr(
            self, "action_server"
        ):
            self.action_server.destroy()

    def destroy_all_action_clients(self):
        """
        Destroys all action clients
        """
        pass

    def destroy_all_service_clients(self):
        """destroy_all_service_clients."""
        pass

    def config_from_file(self, config_file: str):
        """
        Configure component from file

        :param config_file: Path to configuration file (yaml, json or toml)
        :type config_file: str
        """
        self.config.from_file(
            config_file, nested_root_name=self.node_name, get_common=True
        )
        # Update algorithms config from file
        if self.algorithms_config:
            # Load the file
            ext: str = os.path.splitext(config_file)[1].lower()

            with open(config_file, "r", encoding="utf-8") as f:
                if ext in [".yaml", ".yml"]:
                    raw_config: Dict[str, Any] = yaml.safe_load(f)
                elif ext == ".json":
                    raw_config = json.load(f)
                elif ext == ".toml":
                    raw_config = toml.load(f)
                else:
                    raise ValueError(f"Unsupported config format: {ext}")

            for algo_name, algo_conf in self._algorithms_config.items():
                config = raw_config
                for key in [self.node_name, algo_name.partition("Config")[0]]:
                    config = config.get(key, {})
                if not config:
                    continue
                for item_key in algo_conf.keys():
                    if hasattr(config, item_key):
                        algo_conf[item_key] = getattr(config, item_key)

    def create_tf_listener(self, tf_config: TFListenerConfig) -> TFListener:
        """
        Creates a new transform listener to lookup a transform with given config and return the transform lookup handler

        :param tf_config: Transform listener config
        :type tf_config: TFListenerConfig

        :return: Transform lookup handler object
        :rtype: TransformListener
        """
        tf_handler = TFListener(tf_config=tf_config, node_name=self.node_name)
        transform_listener = TransformListener(buffer=tf_handler.tf_buffer, node=self)
        tf_handler.set_listener(transform_listener)
        transform_timer = self.create_timer(
            1 / tf_config.lookup_rate, tf_handler.timer_callback
        )  # timer to lookup the transform with given rate
        tf_handler.timer = transform_timer
        return tf_handler

    def create_client(self, *args, **kwargs) -> Client:
        """
        Overwrites the Node create client method to add to the clients list

        :return: ROS service client
        :rtype: rclpy.client.Client
        """
        _new_client = super().create_client(*args, **kwargs)
        if hasattr(self, "clients_list"):
            self.clients_list.append(_new_client)
        return _new_client

    # EVENT MANAGEMENT
    def _turn_on_events_management(self) -> None:
        """
        Turn on event by starting a listener to the event topic

        :param event: Event to be activated
        :type event: Event
        :param qos_profile: Subscriber QoS Profile, defaults to 1
        :type qos_profile: Union[QoSProfile, int], optional
        """
        if not self.__events or not self.__actions:
            return

        # Blackboard to store latest messages for all topics required for all event:
        # {'topic_1_name': RosMsg, 'topic_2_name': ROSMsg, ... }
        self._events_topics_blackboard: Dict[str, EventBlackboardEntry] = {}

        # Identify all unique topics required across ALL events
        unique_topics = {}
        self.__events_per_topic: Dict[str, List[Event]] = {}
        for event in self.__events:
            required_topics = event.get_involved_topics()
            # Ensure topic is not already there, then add to unique topics
            for topic in required_topics:
                if topic.name not in unique_topics:
                    unique_topics[topic.name] = topic
                # update to keep a record of the events to check for each topic
                if topic.name not in self.__events_per_topic:
                    self.__events_per_topic[topic.name] = [event]
                else:
                    self.__events_per_topic[topic.name].append(event)

        # Register the actions
        for event, actions in zip(self.__events, self.__actions, strict=True):
            # Register action to event to get executed on trigger when calling event.check_condition
            event.register_actions(actions)

        # Create ONE subscription per Topic
        self.__event_listeners = []
        for name, topic_obj in unique_topics.items():
            listener = self.create_subscription(
                msg_type=topic_obj.ros_msg_type,
                topic=topic_obj.name,
                callback=partial(self.__event_topic_callback, name),
                qos_profile=topic_obj.qos_profile.to_ros(),
                callback_group=MutuallyExclusiveCallbackGroup(),
            )
            self.__event_listeners.append(listener)

    def _turn_on_fallbacks_subscribers(self):
        # Create ONE subscription per Topic
        self.__fallbacks_listeners = []
        for topic_obj in self.__fallbacks.required_topics:
            listener = self.create_subscription(
                msg_type=topic_obj.ros_msg_type,
                topic=topic_obj.name,
                callback=partial(self.__fallback_topic_callback, topic_obj.name),
                qos_profile=topic_obj.qos_profile.to_ros(),
                callback_group=MutuallyExclusiveCallbackGroup(),
            )
            self.__fallbacks_listeners.append(listener)
            self._fallbacks_topics_timeout[topic_obj.name] = topic_obj.data_timeout

    def __fallback_topic_callback(self, topic_name: str, msg: Any):
        """
        Updates Cache of all required fallbacks topics
        """
        # Update Fallbacks Blackboard with stamped entry
        self._fallbacks_topics_blackboard[topic_name] = EventBlackboardEntry(
            msg=msg, timestamp=time.time()
        )

    def __event_topic_callback(self, topic_name: str, msg: Any):
        """
        Central Handler:
        1. Updates Cache of all required events topics
        2. Re-evaluates all events that depend on this topic
        """
        # Update Blackboard with stamped entry
        self._events_topics_blackboard[topic_name] = EventBlackboardEntry(
            msg=msg, timestamp=time.time()
        )

        # READ & CLEAN: Identify events dependent on this topic
        relevant_events = self.__events_per_topic.get(topic_name, [])

        for event in relevant_events:
            # Instead of passing the raw blackboard
            # we perform a lazy cleanup right here for the topics THIS event needs.

            clean_cache_subset = {}
            for topic in event.get_involved_topics():
                # This call performs the check and DELETES expired data if necessary
                valid_entry = EventBlackboardEntry.get(
                    self._events_topics_blackboard,
                    topic.name,
                    topic.data_timeout,
                    event.get_last_processed_id(topic.name),
                )
                if valid_entry:
                    clean_cache_subset[topic.name] = valid_entry
            # Pass the clean subset to the event
            event.check_condition(clean_cache_subset)

    def _add_event_action_pair(self, event: Event, action: Union[Action, List[Action]]):
        """Add an event/action pair.
        This method is supposed to be used by child components if required
        """
        action_set = action if isinstance(action, List) else [action]
        if self.__events and self.__actions:
            self.__events.append(event)
            self.__actions.append(action_set)
        else:
            self.__events = [event]
            self.__actions = [action_set]

    def got_all_inputs(
        self,
        inputs_to_check: Optional[List[str]] = None,
        inputs_to_exclude: Optional[List[str]] = None,
    ) -> bool:
        """
        Check if all input topics are being published

        :param inputs_to_check: List of input keys to check, defaults to None
        :type inputs_to_check: list[str] | None, optional

        :param inputs_to_exclude: List of input keys to exclude from check, defaults to None
        :type inputs_to_exclude: list[str] | None, optional

        :return: If all inputs are published
        :rtype: bool
        """

        if inputs_to_exclude:
            # If a non valid key is provided raise an error
            if not all(item in self.callbacks.keys() for item in inputs_to_exclude):
                raise ValueError(
                    f"Checking inputs is trying to exclude a non existing topic key(s): {inputs_to_exclude}. Available keys: {self.callbacks.keys()}"
                )
            inputs_dict_to_check = self.__exclude_keys_from_dict(
                self.callbacks, inputs_to_exclude
            )

        elif inputs_to_check:
            # If a non valid key is provided raise an error
            if not all(item in self.callbacks.keys() for item in inputs_to_check):
                raise ValueError(
                    f"Checking inputs is trying to restrict check to non existing topic key(s): {inputs_to_check}. Available keys: {self.callbacks.keys()}"
                )
            inputs_dict_to_check = self.__restrict_keys_from_dict(
                self.callbacks, inputs_to_check
            )

        else:
            inputs_dict_to_check = self.callbacks

        # Check if all callbacks of the selected topics got input messages
        for callback in inputs_dict_to_check.values():
            if callback._subscriber and not callback.got_msg:
                return False
        return True

    def get_missing_inputs(self) -> List[str]:
        """
        Get a list of input topic names not being published

        :return: List of unpublished topics
        :rtype: list[str]
        """
        unpublished_topics = []
        for callback in self.callbacks.values():
            if callback._subscriber and not callback.got_msg:
                unpublished_topics.append(callback.input_topic.name)
        return unpublished_topics

    def __exclude_keys_from_dict(self, input_dict: Dict, key_list: List) -> Dict:
        """Return input_dict without excluded keys

        :param input_dict: Input dictionary
        :type input_dict: Dict
        :param key_list: Keys to exclude
        :type key_list: List
        :return: Output dictionary
        :rtype: Dict
        """
        return {key: value for key, value in input_dict.items() if key not in key_list}

    def __restrict_keys_from_dict(self, input_dict: Dict, key_list: List) -> Dict:
        """Return input_dict with only restricted keys

        :param input_dict: Input dictionary
        :type input_dict: Dict
        :param key_list: Keys to restrict
        :type key_list: List
        :return: Output dictionary
        :rtype: Dict
        """
        return {key: value for key, value in input_dict.items() if key in key_list}

    @property
    def run_type(self) -> ComponentRunType:
        """
        Component run type: Timed, ActionServer or Server

        :return: Timed, ActionServer or Server
        :rtype: str
        """
        return self.config._run_type

    @run_type.setter
    def run_type(self, value: ComponentRunType):
        self.config._run_type = value

    @property
    def fallback_rate(self) -> float:
        """
        Component fallback rate: Rate in which the component checks for fallbacks and executes a fallback actions if a failure is detected

        :return: Fallback rate (Hz)
        :rtype: float
        """
        return self.config.fallback_rate

    @fallback_rate.setter
    def fallback_rate(self, value: float):
        self.config.fallback_rate = value

    @property
    def loop_rate(self) -> float:
        """
        Component loop rate: Rate in which the component executes its main (_execution_step)

        :return: Loop rate (Hz)
        :rtype: float
        """
        return self.config.loop_rate

    @loop_rate.setter
    def loop_rate(self, value: float):
        self.config.loop_rate = value

    def get_events_actions(self) -> Dict[Event, List[Action]]:
        """Get all Events/Actions registered to the component

        :return: Dictionary of monitored Events and associated Actions
        :rtype: Dict[str, List[Action]]
        """
        if not self.__events or not self.__actions:
            return {}
        return dict(zip(self.__events, self.__actions, strict=True))

    def clear_events_actions(self) -> None:
        """Clear all Events/Actions registered to the component

        :return: Dictionary of monitored Events and associated Actions
        :rtype: Dict[str, List[Action]]
        """
        self.__events = []
        self.__actions = []

    @property
    def _events_actions(self) -> Dict[str, List[Action]]:
        """Getter of component Events Names/Actions

        :return: Dictionary of monitored Events and associated Actions
        :rtype: Dict[str, List[Action]]
        """
        if not self.__events or not self.__actions:
            return {}
        return {
            event.id: action
            for event, action in zip(self.__events, self.__actions, strict=True)
        }

    @_events_actions.setter
    def _events_actions(
        self, events_actions_dict: Dict[str, Union[Action, List[Action]]]
    ):
        """Setter of component Events/Actions

        :param events_actions_dict: Dictionary of Events and associated Actions
        :type events_actions_dict: Dict[Event, List[Action]]
        :raises ValueError: If a given Action does not correspond to a valid component method
        """
        # Initialize only if one of events/actions is None
        if not self.__events or not self.__actions:
            self.__events = []
            self.__actions = []
        for event_serialized, actions in events_actions_dict.items():
            action_set = actions if isinstance(actions, list) else [actions]
            for action in action_set:
                if not hasattr(self, action.action_name):
                    raise ValueError(
                        f"Component '{self.node_name}' does not support action '{action.action_name}'"
                    )
            self.__events.append(Event.from_json(event_serialized))
            self.__actions.append(action_set)

    # SERIALIZATION AND DESERIALIZATION
    @property
    def launch_cmd_args(self) -> List[str]:
        """
        List of command line arguments

        :return: ROS launch command line arguments
        :rtype: List[str]
        """
        return self._cmd_line_kwargs_list

    @launch_cmd_args.setter
    def launch_cmd_args(self, values: List):
        """launch_cmd_args.

        :param values:
        :type values: List
        """
        try:
            for i, val in enumerate(values):
                if val.startswith("--"):
                    if val not in self._cmd_line_kwargs_list:
                        # Add new value to the list
                        self._cmd_line_kwargs_list.append(val)
                        self._cmd_line_kwargs_list.append(str(values[i + 1]))
                    else:
                        # Update an existing value
                        idx = self._cmd_line_kwargs_list.index(val)
                        self._cmd_line_kwargs_list[idx + 1] = str(values[i + 1])
        except IndexError as e:
            raise IndexError("Launch commands require more arguments to update") from e

    def _update_cmd_args_list(self):
        """
        Update launch command arguments
        """
        self.launch_cmd_args = [
            "--component_type",
            self.__class__.__name__,
            "--config_type",
            self.config.__class__.__name__,
            "--config",
            self._config_json,
            "--node_name",
            self.node_name,
            "--inputs",
            self._inputs_json,
            "--outputs",
            self._outputs_json,
            "--algorithms_config",
            self._algorithms_json,
        ]

        if self._config_file:
            self.launch_cmd_args = ["--config_file", self._config_file]

        if self.__events:
            self.launch_cmd_args = ["--events", self._events_json]

        if self.__actions:
            self.launch_cmd_args = ["--actions", self._actions_json]

        if self.__fallbacks:
            self.launch_cmd_args = ["--fallbacks", self._fallbacks_json]

        if self._external_processors:
            self.launch_cmd_args = [
                "--external_processors",
                self._external_processors_json,
            ]

    @property
    def _events_json(self) -> Union[str, bytes]:
        """Getter of serialized component Events

        :return: Serialized Events List
        :rtype: Union[str, bytes]
        """
        if not self.__events:
            return "[]"
        return json.dumps([event.to_json() for event in self.__events])

    @_events_json.setter
    def _events_json(self, events_serialized: Union[str, bytes]):
        """Setter of component events from JSON serialized events

        :param events_serialized: Serialized Events List
        :type events_serialized: Union[str, bytes]
        """
        list_obj = json.loads(events_serialized)

        self.__events = []
        for event_serialized in list_obj:
            new_event = Event.from_json(event_serialized)
            self.__events.append(
                deepcopy(new_event)
            )  # deepcopy is needed to avoid copying the previous event

    @property
    def _actions_json(self) -> Union[str, bytes]:
        """Getter of serialized component Actions

        :return: Serialized Actions: {event_name: serialized_action}
        :rtype: Union[str, bytes]
        """
        actions_dict = {}
        for event_name, action_set in self._events_actions.items():
            actions_serialized = []
            for action in action_set:
                actions_serialized.append(action.dictionary)
            actions_dict[event_name] = actions_serialized
        return json.dumps(actions_dict)

    @_actions_json.setter
    def _actions_json(self, actions_serialized: Union[str, bytes]):
        """Setter of component events from JSON serialized actions

        :param actions_serialized: Serialized Actions List
        :type actions_serialized: Union[str, bytes]
        """
        self.__actions = []
        actions_dict: Dict = json.loads(actions_serialized)
        for action_list in actions_dict.values():
            reconstructed_action_list = []
            for action_dict in action_list:
                if not hasattr(self, action_dict["action_name"]):
                    raise AttributeError(
                        f"Component '{self.node_name}' does not contain requested Action method '{action_dict['action_name']}'"
                    )
                # reparse the method using the given action name
                method = getattr(self, action_dict["action_name"])
                reconstructed_action = Action.deserialize_action(
                    serialized_action_dict=action_dict,
                    deserialized_method=method,
                )
                reconstructed_action_list.append(reconstructed_action)
            self.__actions.append(reconstructed_action_list)

    @property
    def _fallbacks_json(self) -> Union[str, bytes, bytearray]:
        """Getter of serialized component Fallbacks

        :return: Serialized Fallbacks: {fallback_type: serialized_fallback}
        :rtype: Union[str, bytes]
        """
        return self.__fallbacks.json

    @_fallbacks_json.setter
    def _fallbacks_json(self, serialized_fallbacks: Union[str, bytes]):
        deserialized_fallbacks = json.loads(serialized_fallbacks)
        fallbacks_kwargs = {}
        for key, value in deserialized_fallbacks.items():
            # If a fallback was defined
            if (value is not None) and (
                fallback_serialized_actions_list := value.get("action_list", None)
            ):
                reconstructed_actions_list = []
                for fallback_serialized_action in fallback_serialized_actions_list:
                    if not hasattr(self, fallback_serialized_action["action_name"]):
                        raise AttributeError(
                            f"Component '{self.node_name}' does not contain requested Fallback Action method '{fallback_serialized_action['action_name']}'"
                        )
                    # reparse the method using the given action name
                    method = getattr(self, fallback_serialized_action["action_name"])
                    reconstructed_actions_list.append(
                        Action.deserialize_action(
                            serialized_action_dict=fallback_serialized_action,
                            deserialized_method=method,
                        )
                    )
                reconstructed_fallback = Fallback(
                    reconstructed_actions_list, value.get("max_retries", None)
                )
                fallbacks_kwargs[key] = reconstructed_fallback
        self.__fallbacks = ComponentFallbacks(**fallbacks_kwargs)

    def set_additional_types(self, value: str):
        """
        Additional types from serialized types (json)

        :param value: Serialized Additional Types
        :type value: str
        """
        serialized_types = json.loads(value)
        for s_t in serialized_types:
            module_name, _, class_name = s_t.rpartition(".")
            if not module_name:
                continue
            module = importlib.import_module(module_name)
            new_type = getattr(module, class_name)
            if issubclass(new_type, SupportedType):
                self._additional_types.append(new_type)

    @property
    def _inputs_json(self) -> Union[str, bytes, bytearray]:
        """
        Serialize component inputs to json

        :return: Serialized inputs
        :rtype:  str | bytes | bytearray
        """
        if not hasattr(self, "in_topics"):
            return "[]"
        return json.dumps([topic.to_json() for topic in self.in_topics])

    @_inputs_json.setter
    def _inputs_json(self, value: Union[str, bytes, bytearray]):
        """
        Component inputs from serialized inputs (json)

        :return: Serialized inputs
        :rtype:  str | bytes | bytearray

        :param value: Serialized inputs
        :type value: Union[str, bytes, bytearray]
        """
        topics = json.loads(value)
        inputs = []
        for t in topics:
            topic_dict = json.loads(t)
            topic_dict["qos_profile"] = QoSConfig(**topic_dict.get("qos_profile", {}))
            topic_dict["additional_types"] = (
                self._additional_types
            )  # Add any additional types
            inputs.append(Topic(**topic_dict))

        self.in_topics = self._reparse_inputs_callbacks(inputs)
        self.callbacks = {
            input.name: input.msg_type.callback(input, node_name=self.node_name)
            for input in self.in_topics
        }

    @property
    def _outputs_json(self) -> Union[str, bytes, bytearray]:
        """
        Serialize component inputs to json

        :return: Serialized inputs
        :rtype:  str | bytes | bytearray
        """
        if not hasattr(self, "out_topics"):
            return "[]"
        return json.dumps([topic.to_json() for topic in self.out_topics])

    @_outputs_json.setter
    def _outputs_json(self, value: Union[str, bytes, bytearray]):
        """
        Component inputs from serialized inputs (json)

        :return: Serialized inputs
        :rtype:  str | bytes | bytearray

        :param value: Serialized inputs
        :type value: Union[str, bytes, bytearray]
        """
        topics = json.loads(value)
        outputs = []
        for t in topics:
            topic_dict = json.loads(t)
            topic_dict["qos_profile"] = QoSConfig(**topic_dict.get("qos_profile", {}))
            topic_dict["additional_types"] = (
                self._additional_types
            )  # Add any additional types
            outputs.append(Topic(**topic_dict))
        self.out_topics = self._reparse_outputs_converts(outputs)
        self.publishers_dict = {
            output.name: Publisher(output, node_name=self.node_name)
            for output in self.out_topics
        }

    @property
    def _external_processors_json(self) -> Union[str, bytes]:
        """Getter of serialized external processors

        :return: Serialized external processors definition
        :rtype: Union[str, bytes]
        """
        return json.dumps({
            topic_name: ([p.__name__ for p in processors], str(processor_type))  # type: ignore
            for topic_name, (
                processors,
                processor_type,
            ) in self._external_processors.items()
        })

    @_external_processors_json.setter
    def _external_processors_json(self, processors_serialized: Union[str, bytes]):
        """Setter of external processors from JSON serialized processors

        :param processors_serialized: Serialized Processors Dict
        :type processors_serialized: Union[str, bytes]
        """
        loaded_processors = json.loads(processors_serialized)

        # reconstruct the dictionary, validating the processor type string
        self._external_processors = {}

        for key, processor_data in loaded_processors.items():
            # get processor data and type
            func_names = processor_data[0]
            proc_type_str = processor_data[1]

            # Validate string back to Enum-compatible string
            valid_proc_type = ExternalProcessorType(proc_type_str)

            # Initialize the list with function names
            self._external_processors[key] = (func_names, valid_proc_type)

            # Create sockets out of function names and connect them
            current_processors_list = self._external_processors[key][0]

            for idx, func_name in enumerate(current_processors_list):
                sock_file = f"/tmp/{self.node_name}_{key}_{func_name}.socket"
                if not os.path.exists(sock_file):
                    raise RuntimeError(
                        f"File {sock_file} doesn't exists. The external processors have not been setup properly. Exiting .. "
                    )

                sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
                sock.settimeout(1)  # timeout set to 1s
                sock.connect(sock_file)
                # The processessing functions are a list which is the first element of the tuple
                # stored in _external_processors dict
                processor_data[0][idx] = sock

    @property
    def _algorithms_json(self) -> Union[str, bytes, bytearray]:
        """
        Serialize component inputs to json

        :return: Serialized inputs
        :rtype:  str | bytes | bytearray
        """
        return json.dumps(self._algorithms_config)

    @_algorithms_json.setter
    def _algorithms_json(self, value: Union[str, bytes, bytearray]):
        """
        Component inputs from serialized inputs (json)

        :return: Serialized inputs
        :rtype:  str | bytes | bytearray

        :param value: Serialized inputs
        :type value: Union[str, bytes, bytearray]
        """
        self._algorithms_config = json.loads(value)

    @property
    def _config_json(self) -> Union[str, bytes, bytearray]:
        """
        Component config as a json string

        :return: Config json
        :rtype: str
        """
        return self.config.to_json()

    @_config_json.setter
    def _config_json(self, value: str):
        """
        Component config from json string

        :param value: Config json
        :type value: str
        """
        self.config.from_json(value)

    # TODO: Implement dunder methods for a more intuitive API with components

    # MAIN ACTION SERVER HELPER METHODS AND CALLBACKS
    @abstractmethod
    def main_action_callback(self, goal_handle) -> Any:
        """
        Component main action server callback - used if component started with run_as_action_server=True

        :param goal_handle: Action goal handle
        :type goal_handle: action_type.Goal
        """
        if self.run_type == ComponentRunType.ACTION_SERVER:
            raise NotImplementedError

    def _main_action_goal_callback(self, _) -> GoalResponse:
        """
        Goal callback for the main component action server

        :param goal_request: _description_
        :type goal_request: Any action goal handler type
        :return: ACCEPT
        :rtype: rclpy.action.GoalResponse
        """
        # Cancel any ongoing action
        self.get_logger().info("Received goal request")
        return GoalResponse.ACCEPT

    def _main_action_handle_accepted_callback(self, goal_handle):
        """
        Main component action server callback when handle is accepted
        """
        with self._main_goal_lock:
            if self._main_goal_handle is not None and self._main_goal_handle.is_active:
                # Abort the existing goal
                self.get_logger().info("Aborting previous goal")
                self._main_goal_handle.abort()
            self._main_goal_handle = goal_handle
            self.get_logger().info("Goal accepted")
            self._main_goal_handle.execute()

    def _main_action_cancel_callback(self, _) -> Optional[CancelResponse]:
        """Main component action server callback when handle is canceled

        :param goal_handle: _description_
        :type goal_handle: _type_
        :return: _description_
        :rtype: _type_
        """
        self.get_logger().warn("Received cancel request")
        try:
            return CancelResponse.ACCEPT
        except Exception as e:
            self.get_logger().error(f"Failed to send accept cancel request: {e}")

    @property
    def main_action_name(self) -> Optional[str]:
        """
        Name of the main action server created by the component

        :return: ActionServer name
        :rtype: str
        """
        if self.action_type and hasattr(self.action_type, "__name__"):
            return f"{self.node_name}/{camel_to_snake_case(self.action_type.__name__)}"
        return None

    # MAIN SERVER HELPER METHODS AND CALLBACK
    @property
    def main_srv_name(self) -> Optional[str]:
        """
        Name of the main server created by the component

        :return: Server name
        :rtype: str
        """
        if self.service_type and hasattr(self.service_type, "__name__"):
            return f"{self.node_name}/{camel_to_snake_case(self.service_type.__name__)}"
        return None

    @abstractmethod
    def main_service_callback(self, request, response):
        """
        Component main service callback - used if component started with run_as_server=True

        :param request: Service request
        :type request: service_type.Request
        :param response: Service response
        :type response: service_type.Response
        """
        if self.run_type == ComponentRunType.SERVER:
            raise NotImplementedError
        return response

    def _create_default_services(self):
        """
        Creates default services for updating parameters and changing input/output topics
        """
        # to handle one call at a time set callback to new MutuallyExclusiveCallbackGroup
        self._default_services = [
            self.create_service(
                srv_type=ChangeParameter,
                srv_name=f"{self.get_name()}/update_config_parameter",
                callback=self._update_config_parameter_srv_callback,
                callback_group=MutuallyExclusiveCallbackGroup(),
            ),
            self.create_service(
                srv_type=ChangeParameters,
                srv_name=f"{self.get_name()}/update_config_parameters",
                callback=self._update_config_parameters_srv_callback,
                callback_group=MutuallyExclusiveCallbackGroup(),
            ),
            # Input/Output update services
            self.create_service(
                srv_type=ReplaceTopic,
                srv_name=f"{self.get_name()}/change_topic",
                callback=self._change_topic_srv_callback,
                callback_group=MutuallyExclusiveCallbackGroup(),
            ),
            self.create_service(
                srv_type=ConfigureFromFile,
                srv_name=f"{self.get_name()}/configure_from_file",
                callback=self._configure_from_file_srv_callback,
                callback_group=MutuallyExclusiveCallbackGroup(),
            ),
            # Run component method
            self.create_service(
                srv_type=ExecuteMethod,
                srv_name=f"{self.get_name()}/execute_method",
                callback=self._execute_method_srv_callback,
                callback_group=MutuallyExclusiveCallbackGroup(),
            ),
        ]

    # EVENTS/ACTIONS RELATED SERVICES
    @log_srv
    def _configure_from_file_srv_callback(
        self, request: ConfigureFromFile.Request, response: ConfigureFromFile.Response
    ) -> ConfigureFromFile.Response:
        """
        Configure the component from file service callback

        :param request: _description_
        :type request: ConfigureFromFile.Request
        :param response: _description_
        :type response: ConfigureFromFile.Response
        :return: _description_
        :rtype: ConfigureFromFile.Response
        """
        try:
            # Reconfigure and restart the node
            reconfigured = self.reconfigure(request.path_to_file)
            if reconfigured:
                response.success = True
            else:
                response.success = False
                response.error_msg = "Failed to Restart the node"
        except Exception as e:
            response.success = False
            response.error_msg = f"{e}"
        return response

    def _update_config_param_from_str_value(
        self, param_name: str, param_str_value: str
    ) -> Optional[str]:
        """
        Helper method to update config parameter value and return an error message if it is not updated

        :param param_name: _description_
        :type param_name: str
        :param param_str_value: _description_
        :type param_str_value: str

        :return: Error message - None if the parameter is updated without errors
        :rtype: str | None
        """
        error_msg: Optional[str] = None

        param_type = self.config.get_attribute_type(param_name)
        try:
            parsed_param = param_type(param_str_value) if param_type else None
            self.config.update_value(param_name, parsed_param)
            self.get_logger().debug(
                f"Updates {self.node_name} config param {param_name} to : {parsed_param}"
            )

        except Exception as e:
            error_msg = f"'Error setting {self.config.__class__.__name__}' attribute '{param_name}' of type {param_type} with value '{param_str_value}'. Error message details: {e}"

        return error_msg

    @classmethod
    def get_change_parameters_msg_from_config(
        cls, config: BaseComponentConfig
    ) -> ChangeParameters.Request:
        """
        Helper method to update config parameter value and return an error message if it is not updated

        :param config: _description_
        :type config: ComponentConfig or child class

        :return: Request message for change parameters services corresponding to the given config
        :rtype: ChangeParameters.Request
        """
        param_names = []
        param_values = []
        for param_name, param_value in config.asdict().items():
            if not isinstance(param_value, Dict):
                param_names.append(param_name)
                param_values.append(str(param_value))
            else:
                for nested_name, nested_value in param_value.items():
                    param_names.append(f"{param_name}.{nested_name}")
                    param_values.append(str(nested_value))

        request_msg = ChangeParameters.Request()
        request_msg.names = param_names
        request_msg.values = param_values

        return request_msg

    @log_srv
    def _update_config_parameter_srv_callback(
        self, request: ChangeParameter.Request, response: ChangeParameter.Response
    ) -> ChangeParameter.Response:
        """
        Update config parameter service callback

        :param request: _description_
        :type request: ChangeParameter.Request
        :param response: _description_
        :type response: ChangeParameter.Response
        :return: _description_
        :rtype: ChangeParameter.Response
        """
        param_name: str = request.name
        param_str_value: str = request.value

        # To keep the component alive while reconfiguring
        keep_alive = request.keep_alive

        if not keep_alive:
            # Set the flag so the default services are not destroyed or re-created
            self._maintain_default_services = True
            # Stop the component
            self.stop()
            self.trigger_cleanup()
            self.trigger_configure()

        error_msg = self._update_config_param_from_str_value(
            param_name, param_str_value
        )

        if not keep_alive:
            # start again
            self.start()

        if not error_msg:
            response.success = True
        else:
            response.success = False
            response.error_msg = error_msg

        timeout_counter = 0  # Add timeout to avoid an infinite loop
        while self.lifecycle_state != LifecycleStateMsg.PRIMARY_STATE_ACTIVE and (
            timeout_counter < self.config.wait_for_restart_time
        ):
            self.get_logger().warn(
                f"Component {self.node_name} is not in ACTIVE state. Waiting for it to become active again.",
                once=True,
            )
            time.sleep(1 / self.config.loop_rate)
            timeout_counter += 1 / self.config.loop_rate

        if self.lifecycle_state != LifecycleStateMsg.PRIMARY_STATE_ACTIVE:
            response.success = False
            response.error_msg = "Error restarting the component"
            self.health_status.set_fail_component()

        return response

    @log_srv
    def _update_config_parameters_srv_callback(
        self, request: ChangeParameters.Request, response: ChangeParameters.Response
    ) -> ChangeParameters.Response:
        """
        Update list of config parameters service callback

        :param request: _description_
        :type request: ChangeParameters.Request
        :param response: _description_
        :type response: ChangeParameters.Response
        :return: _description_
        :rtype: ChangeParameters.Response
        """
        param_names: List[str] = request.names
        param_str_values: List[str] = request.values

        # To keep the component alive while reconfiguring
        keep_alive = request.keep_alive

        if not keep_alive:
            # Set the flag so the default services are not destroyed or re-created
            self._maintain_default_services = True
            # Stop the component
            self.stop()
            self.trigger_cleanup()
            self.trigger_configure()

        response.success = []
        response.error_msg = []

        for name, val in zip(param_names, param_str_values, strict=True):
            error_msg = self._update_config_param_from_str_value(name, val)

            if not error_msg:
                response.success.append(True)
                response.error_msg.append("")
            else:
                response.success.append(False)
                response.error_msg.append(error_msg)

        if not keep_alive:
            # start again
            self.start()

        return response

    def _replace_input_topic(
        self, topic_name: str, new_name: str, msg_type: str
    ) -> Optional[str]:
        """Replaces a component input topic by a new topic

        :param topic_name: Old Topic name
        :type topic_name: str
        :param new_name: New topic name
        :type new_name: str
        :param msg_type: New topic message type
        :type msg_type: str
        :return: Error message or None if no errors are found
        :rtype: Optional[str]
        """
        normalized_topic_name = (
            topic_name[1:] if topic_name.startswith("/") else topic_name
        )

        if topic_name not in self.callbacks.keys():
            error_msg = f"Topic {topic_name} is not found in Component inputs"
            return error_msg

        old_callback = self.callbacks[normalized_topic_name]

        # Create New Topic/Callback
        try:
            new_topic = Topic(name=new_name, msg_type=msg_type)
            new_callback = new_topic.msg_type.callback(
                new_topic, node_name=self.node_name
            )
        except Exception as e:
            error_msg = f"Invalid topic parameters: {e}"
            return error_msg

        # Handle Active Subscriber
        if old_callback._subscriber:
            self.get_logger().info(
                f"Destroying subscriber for old topic '{topic_name}'"
            )
            self.destroy_subscription(old_callback._subscriber)

            new_callback.set_subscriber(self._add_ros_subscriber(new_callback))

        # Update callbacks dictionary
        self.callbacks.pop(normalized_topic_name)
        self.callbacks[new_name] = new_callback

        # update the internal lists
        old_topic = old_callback.input_topic
        self._update_inactive_input_topic(old_topic, new_topic)

        return None

    def _update_inactive_input_topic(self, old_topic, new_topic):
        """Updates internal topics list with a new input topic"""
        # Update in_topics list
        try:
            idx = self.in_topics.index(old_topic)
            self.in_topics.pop(idx)
            self.in_topics.insert(idx, new_topic)
        except ValueError:
            self.get_logger().warn(
                f"Old topic {old_topic.name} not found in self.in_topics. "
                "Updating callbacks anyway."
            )

    def _replace_output_topic(
        self, topic_name: str, new_name: str, msg_type: str
    ) -> Optional[str]:
        """Replaces a component output topic by a new topic

        :param topic_name: Old Topic name
        :type topic_name: str
        :param new_name: New topic name
        :type new_name: str
        :param msg_type: New topic message type
        :type msg_type: str
        :return: Error message or None if no errors are found
        :rtype: Optional[str]
        """
        normalized_topic_name = (
            topic_name[1:] if topic_name.startswith("/") else topic_name
        )

        if topic_name not in self.publishers_dict.keys():
            error_msg = f"Topic {topic_name} is not found in Component outputs"
            return error_msg

        publisher = self.publishers_dict[normalized_topic_name]

        # Create New Topic
        try:
            new_topic = Topic(name=new_name, msg_type=msg_type)
        except Exception as e:
            error_msg = f"Invalid topic parameters: {e}"
            return error_msg

        # Get old_topic *before* replacing it on the publisher
        old_topic = publisher.output_topic
        publisher.output_topic = new_topic

        # Handle Active Publisher
        # If the publisher is active, "hot-swap" it
        if publisher._publisher:
            self.get_logger().info(f"Destroying publisher for old topic '{topic_name}'")
            self.destroy_publisher(publisher._publisher)

            self.get_logger().info(f"Creating publisher for new topic '{new_name}'")
            publisher.set_publisher(self._add_ros_publisher(publisher))

        # Update publishers_dict
        self.publishers_dict.pop(normalized_topic_name)
        # Note: new_name comes from new_topic.name
        self.publishers_dict[new_topic.name] = publisher

        # update the internal lists
        self._update_inactive_output_topic(old_topic, new_topic)

        return None

    def _update_inactive_output_topic(self, old_topic, new_topic):
        """Updates internal topics list with a new output topic"""
        # Update out_topics list
        try:
            idx = self.out_topics.index(old_topic)
            self.out_topics.pop(idx)
            self.out_topics.insert(idx, new_topic)
        except ValueError:
            self.get_logger().warn(
                f"Old topic {old_topic.name} not found in self.out_topics. "
                "Updating publisher dictionary anyway."
            )

    def _replace_output_by_client(
        self, topic_name: str, service_client: RobotPluginServiceClient
    ) -> Optional[str]:
        """Replaces an output topic by a robot service client - Used for enabling clients from robot plugins

        :param topic_name: Old topic name
        :type topic_name: str
        :param service_client: Robot service client
        :type service_client: RobotPluginServiceClient
        :return: Error message or None if no errors are found
        :rtype: Optional[str]
        """
        # Normalize names
        normalized_topic_name = (
            topic_name[1:] if topic_name.startswith("/") else topic_name
        )

        if topic_name not in self.publishers_dict.keys():
            return f"Topic '{topic_name}' not found in publishers"

        publisher = self.publishers_dict[normalized_topic_name]

        # Get old_topic *before* replacing it on the publisher
        old_topic = publisher.output_topic

        # Handle Active Publisher
        # If the publisher is active, "hot-swap"
        if publisher._publisher:
            self.destroy_publisher(publisher._publisher)

        # Update publishers_dict
        old_publisher = self.publishers_dict.pop(normalized_topic_name)
        service_client.replace_publisher(old_publisher)
        # Note: new_name comes from new_topic.name
        self.publishers_dict[service_client.name] = service_client

        # update the internal lists
        self._update_inactive_output_topic(old_topic, service_client)
        return None

    @log_srv
    def _change_topic_srv_callback(
        self, request: ReplaceTopic.Request, response: ReplaceTopic.Response
    ) -> ReplaceTopic.Response:
        """
        Change topic service callback

        :param request: _description_
        :type request: ReplaceTopic.Request
        :param response: _description_
        :type response: ReplaceTopic.Response
        :return: _description_
        :rtype: ReplaceTopic.Response
        """
        if request.direction == ReplaceTopic.Request.INPUT_TOPIC:
            error_msg = self._replace_input_topic(
                request.old_name, request.new_name, request.new_msg_type
            )
            if not error_msg:
                response.success = True
            else:
                response.success = False
                response.error_msg = error_msg

        elif request.direction == ReplaceTopic.Request.OUTPUT_TOPIC:
            self._replace_output_topic(
                request.old_name, request.new_name, request.new_msg_type
            )
            response.success = False
            response.error_msg = "Not implemented"
        else:
            response.success = False
            response.error_msg = f"Got invalid direction value '{request.direction}'. Direction can only be in [{ReplaceTopic.Request.INPUT_TOPIC} -> input, or {ReplaceTopic.Request.OUTPUT_TOPIC} -> output]"

        return response

    @log_srv
    def _execute_method_srv_callback(
        self, request: ExecuteMethod.Request, response: ExecuteMethod.Response
    ) -> ExecuteMethod.Response:
        if not hasattr(self, request.name):
            response.success = False
            response.error_msg = f"Component {self.node_name} does not have a method with requested name '{request.name}'"
            return response
        kwargs = {}
        if request.kwargs_json:
            try:
                kwargs = json.loads(request.kwargs_json)
            except json.decoder.JSONDecodeError as e:
                response.success = False
                response.error_msg = (
                    f"Expecting json style keyword arguments, got {request.kwargs_json}"
                )
                self.get_logger().warn(f"Error parsing request parameters: {e}")
                return response
        try:
            method = getattr(self, request.name)
            method(**kwargs)
            response.success = True
        except Exception as e:
            response.success = False
            response.error_msg = f"Component {self.node_name} has a method with requested name '{request.name}' but the following error raised while running: {e}"
        return response

    # END OF EVENTS/ACTIONS RELATED SERVICES
    def is_topic_of_type(self, input, msg_type: type) -> bool:
        """
        Checks if a topic contains a msg of given type

        :param input: Object to check
        :type input: Topic
        :param msg_type: Topic message type to check
        :type msg_type: type

        :return: If input is a Topic with given message type
        :rtype: bool
        """
        return isinstance(input, Topic) and input.ros_msg_type == msg_type

    def _attach_external_processors(self):
        """
        Attach external processors
        """
        if self._external_processors:
            self.get_logger().info("ATTACHING EXTERNAL PROCESSORS")
        for topic_name, (
            processors,
            processor_type,
        ) in self._external_processors.items():
            if processor_type == ExternalProcessorType.MSG_PRE_PROCESSOR:
                self.publishers_dict[topic_name].add_pre_processors(processors)
            elif processor_type == ExternalProcessorType.MSG_POST_PROCESSOR:
                self.callbacks[topic_name].add_post_processors(processors)

    def _destroy_external_processors(self):
        """
        Destroy external processors
        """
        if len(self._external_processors):
            for processors, _ in self._external_processors.values():
                for processor in processors:
                    if isinstance(processor, socket.socket):
                        processor.close()

    # MAIN
    def _main(self):
        """
        Component execution step every loop_step
        """
        if self.health_status_publisher:
            self.health_status_publisher.publish(self.health_status())

        # If it is not a timed component -> only publish status
        if self.run_type != ComponentRunType.TIMED:
            return

        # Additional execution loop if exists
        if hasattr(self, "_extra_execute_loop"):
            self._extra_execute_loop()

        # Execute main loop
        self._execution_step()

        # Execute once
        if not hasattr(self, "_exec_started"):
            self._execute_once()
            if hasattr(self, "_extra_execute_once"):
                self._extra_execute_once()
            self._exec_started = True

    # Timed runtype execution step
    def _execution_step(self):
        """
        Main execution of the component, executed at each timer tick with rate 'loop_rate' from config
        """
        raise NotImplementedError

    # Timed runtype execution step
    def _execute_once(self):
        """
        Executed once when the component is started in TIMED runtype
        """
        pass

    def add_execute_once(self, method: Callable):
        """
        Add method to be executed once when the component is started

        :param method: Callable to be executed
        :type method: Callable
        """
        self._extra_execute_once = method

    def add_execute_in_loop(self, method: Callable):
        """
        Add method to be executed each loop_step in the component

        :param method: Callable to be executed
        :type method: Callable
        """
        self._extra_execute_loop = method

    def get_ros_time(self) -> Time:
        """
        Helper method to get ROS time from the node

        :return: ROS time now
        :rtype: Time
        """
        return self.get_clock().now().to_msg()

    def get_secs_time(self) -> float:
        """
        Gets the current ROS time as float in seconds

        :param node: ROS node
        :type node: Node

        :return: ROS time as float
        :rtype: float
        """
        ros_time = self.get_ros_time()
        return float(ros_time.sec + 1e-9 * ros_time.nanosec)

    # COMPONENT ACTIONS
    @property
    def available_actions(self) -> List[str]:
        """
        Getter of available component actions

        :return: Methods names
        :rtype: List[str]
        """
        return get_methods_with_decorator(self, decorator_name="component_action")

    def __wait_for_node_start(self) -> bool:
        """Executes a waiting loop until the node is discoverable in ROS

        :return: If node is active
        :rtype: bool
        """
        timeout = 0.0
        # Wait until node is actually up and active
        while timeout < self.config.wait_for_restart_time:
            all_nodes = self.get_node_names()
            if self.node_name in all_nodes:
                self.get_logger().info(f"Node {self.node_name} is successfully started")
                break
            time.sleep(1 / self.config.loop_rate)
            timeout += 1 / self.config.loop_rate
        return timeout < self.config.wait_for_restart_time

    def __wait_for_state_transition(self) -> bool:
        """Waits until the component is not in a transitioning state in:
        TRANSITION_STATE_CONFIGURING = 10
        TRANSITION_STATE_CLEANINGUP = 11
        TRANSITION_STATE_SHUTTINGDOWN = 12
        TRANSITION_STATE_ACTIVATING = 13
        TRANSITION_STATE_DEACTIVATING = 14
        TRANSITION_STATE_ERRORPROCESSING = 15

            :return: _description_
            :rtype: bool
        """
        timeout_counter = 0  # Add timeout to avoid an infinite loop
        while (
            self.lifecycle_state >= LifecycleStateMsg.TRANSITION_STATE_CONFIGURING
        ) and (timeout_counter < self.config._lifecycle_state_transition_timeout):
            self.get_logger().warn(
                "Waiting for ongoing transition to end before executing new transition",
                once=True,
            )
            time.sleep(1 / self.config.loop_rate)
            timeout_counter += 1 / self.config.loop_rate

        if self.lifecycle_state >= LifecycleStateMsg.TRANSITION_STATE_CONFIGURING:
            self.get_logger().debug(
                "Error: Component stuck in lifecycle transition",
            )
            self.health_status.set_fail_component()
            return False
        return True

    @component_action
    def start(self, **_) -> bool:
        """
        Start the component - trigger_activate

        :return: If the component is started
        :rtype: bool
        """
        if self.lifecycle_state == LifecycleStateMsg.PRIMARY_STATE_ACTIVE:
            # Component already active
            return True

        elif self.lifecycle_state in [
            LifecycleStateMsg.PRIMARY_STATE_UNCONFIGURED,
            LifecycleStateMsg.PRIMARY_STATE_FINALIZED,
        ]:
            # unconfigured or finalized -> configure again before starting
            self.trigger_configure()

        transition_done = self.__wait_for_state_transition()

        if not transition_done:
            return False

        # configured and inactive
        self.trigger_activate()

        return self.__wait_for_node_start()

    @component_action
    def stop(self, **_) -> bool:
        """
        Stop the component - trigger_deactivate

        :return: If the component is stopped
        :rtype: bool
        """
        if self.lifecycle_state in [
            LifecycleStateMsg.PRIMARY_STATE_UNCONFIGURED,
            LifecycleStateMsg.PRIMARY_STATE_INACTIVE,
            LifecycleStateMsg.PRIMARY_STATE_FINALIZED,
        ]:
            # Already not active
            return True

        transition_done = self.__wait_for_state_transition()

        if not transition_done:
            return False

        self.trigger_deactivate()

        return True

    @component_action
    def reconfigure(self, new_config: Any, keep_alive: bool = False, **_) -> bool:
        """
        Reconfigure the component - cleanup->stop->trigger_configure->start

        :param new_config: New component config
        :type new_config: Any
        :param keep_alive: Reconfigure while the component is online, defaults to False
        :type keep_alive: bool, optional

        :return: If the component is Reconfigured
        :rtype: bool
        """
        self.get_logger().warn("Reconfiguring component...")

        if keep_alive:
            # set new config as params attr
            if isinstance(new_config, str):
                self.configure(config_file=new_config)
            elif isinstance(new_config, self.config.__class__):
                self.config = new_config
            return True

        initial_state = self.lifecycle_state

        reactivate = initial_state >= LifecycleStateMsg.PRIMARY_STATE_ACTIVE

        if initial_state == LifecycleStateMsg.PRIMARY_STATE_UNCONFIGURED:
            # Already configured -> cleanup first
            self.trigger_cleanup()

        if initial_state == LifecycleStateMsg.PRIMARY_STATE_ACTIVE:
            # active -> deactivate then cleanup
            self.trigger_deactivate()
            self.trigger_cleanup()

        transition_done = self.__wait_for_state_transition()

        if not transition_done:
            return False

        # set new config as params attr
        if isinstance(new_config, str):
            self._config_file = new_config
        elif isinstance(new_config, self.config.__class__):
            self.config = new_config

        # configure and go to configure (or active if the component was already active)
        self.trigger_configure()

        if reactivate:
            self.trigger_activate()
            return self.__wait_for_node_start()

        return True

    @component_action
    def restart(self, wait_time: Optional[float] = None, **_) -> bool:
        """
        Restart the component - stop->start

        :return: If the component is Reconfigured
        :rtype: bool
        """

        if self.lifecycle_state == LifecycleStateMsg.PRIMARY_STATE_UNCONFIGURED:
            self.trigger_configure()

        if self.lifecycle_state == LifecycleStateMsg.PRIMARY_STATE_ACTIVE:
            self.trigger_deactivate()

        transition_done = self.__wait_for_state_transition()

        if not transition_done:
            # timeout
            return False

        if wait_time:
            self.get_logger().warn(
                f"Waiting for requested time '{wait_time}'seconds before starting again...",
            )
            time.sleep(wait_time)

        # not configured -> configure and start
        self.trigger_activate()
        return self.__wait_for_node_start()

    @component_action
    def set_param(
        self, param_name: str, new_value: Any, keep_alive: bool = True, **_
    ) -> bool:
        """
        Change the value of one component parameter

        :param param_name: _description_
        :type param_name: str
        :param new_value: _description_
        :type new_value: Any
        :param keep_alive: To keep the component running when updating value, defaults to True
        :type keep_alive: bool, optional

        :raises Exception: Parameter could not be updated to given value

        :return: Parameter updated
        :rtype: bool
        """
        try:
            if keep_alive:
                self.config.update_value(param_name, new_value)
            else:
                self.stop()
                self.config.update_value(param_name, new_value)
                self.start()
        except Exception:
            raise
        return True

    @component_action
    def set_params(
        self, params_names: List[str], new_values: List, keep_alive: bool = True, **_
    ) -> bool:
        """
        Change the value of multiple component parameters

        :param param_name: _description_
        :type param_name: str
        :param new_value: _description_
        :type new_value: Any
        :param keep_alive: To keep the component running when updating value, defaults to True
        :type keep_alive: bool, optional

        :raises Exception: Parameter could not be updated to given value

        :return: Parameter updated
        :rtype: bool
        """
        try:
            if keep_alive:
                for param_name, new_value in zip(params_names, new_values, strict=True):
                    self.config.update_value(param_name, new_value)
            else:
                self.stop()
                for param_name, new_value in zip(params_names, new_values, strict=True):
                    self.config.update_value(param_name, new_value)
                self.start()
        except Exception:
            raise
        return True

    # END OF ACTIONS

    # FALLBACKS
    def _fallbacks_check_callback(self):
        """
        Checks component health status and executes corresponding fallback in case of any detected failure
        """
        if self.health_status.is_healthy:
            # Component is healthy -> nothing to do,
            return

        try:
            # Validate the fallback cache registry
            for topic_name, value in self._fallbacks_topics_blackboard.items():
                value.validate(
                    timeout=self._fallbacks_topics_timeout.get(topic_name, None)
                )
            self.__fallbacks.update_topics_blackboard(self._fallbacks_topics_blackboard)

            if self.__fallbacks_giveup:
                # All fallbacks are already exhausted
                self.get_logger().error(
                    "All possible fallbacks are exhausted -> Component is givingup",
                    once=True,
                )
                if self.__fallbacks.on_giveup:
                    self.__fallbacks.execute_giveup()
                    self.health_status.value = self.__fallbacks.latest_status
                return

            elif (
                self.health_status.is_algorithm_fail
                and self.__fallbacks.on_algorithm_fail
            ):
                self.get_logger().warn(
                    "Algorithm Failure Detected -> Executing Fallback ..."
                )
                self.__fallbacks_giveup = self.__fallbacks.execute_algorithm_fallback()

            elif (
                self.health_status.is_component_fail
                and self.__fallbacks.on_component_fail
            ):
                self.get_logger().warn(
                    "Component Failure Detected -> Executing Fallback ..."
                )
                self.__fallbacks_giveup = self.__fallbacks.execute_component_fallback()

            elif self.health_status.is_system_fail and self.__fallbacks.on_system_fail:
                self.get_logger().warn(
                    "System Failure Detected -> Executing Fallback ..."
                )
                self.__fallbacks_giveup = self.__fallbacks.execute_system_fallback()

            elif self.__fallbacks.on_any_fail:
                self.__fallbacks_giveup = self.__fallbacks.execute_generic_fallback()

            # Update the health status from the fallback after execution
            self.health_status.value = self.__fallbacks.latest_status

        except ValueError:
            # ValueError is thrown when no fallbacks are defined for detected failure
            self.get_logger().warn(
                "No fallback policy is defined for detected failure -> Failure is broadcasted"
            )

    @property
    def fallbacks(self) -> List[str]:
        """
        Gets all available component fallback methods

        :return: List of available fallback names
        :rtype: List[str]
        """
        component_actions = get_methods_with_decorator(
            self, decorator_name="component_action"
        )
        component_fallbacks = get_methods_with_decorator(
            self, decorator_name="component_fallback"
        )
        return component_actions + component_fallbacks

    def _is_valid_fallback_action(self, action: Union[List[Action], Action]) -> bool:
        """Checks if a given action / list of actions are valid component fallbacks

        :param fallback: Method to be checked
        :type fallback: Union[List[Action], Action]

        :return: If action is a valid component fallback
        :rtype: bool
        """
        if isinstance(action, List):
            for f in action:
                self.__check_fallback_action(f)
            return True
        return self.__check_fallback_action(action)

    def __check_fallback_action(self, action: Optional[Action]) -> bool:
        """Checks if a given action is a valid component fallback

        :param fallback: Method to be checked
        :type fallback: Union[List[Action], Action]

        :return: If action is a valid component fallback
        :rtype: bool
        """
        if not action:
            raise ValueError("Cannot set fallback action to None")

        if action.parent_component is not self.node_name:
            raise TypeError(
                f"Non valid failure fallback {action.parent_component}.{action.action_name}. Component fallback can only be a component's own fallback method"
            )
        if action.action_name not in self.fallbacks:
            raise TypeError(
                f"Non valid failure fallback {action.parent_component}.{action.action_name}. Available component fallbacks are the following methods: '{self.fallbacks}'"
            )
        return True

    def on_fail(
        self, action: Union[List[Action], Action], max_retries: Optional[int] = None
    ) -> None:
        """
        Set the fallback strategy (action) on any fail

        :param action: Action to be executed on failure
        :type action: Union[List[Action], Action]
        :param max_retries: Maximum number of action execution retries. None is equivalent to unlimited retries, defaults to None
        :type max_retries: Optional[int], optional
        """
        if self._is_valid_fallback_action(action):
            self.__fallbacks.on_any_fail = Fallback(
                action=action, max_retries=max_retries
            )

    def on_system_fail(
        self, action: Union[List[Action], Action], max_retries: Optional[int] = None
    ) -> None:
        """
        Set the fallback strategy (action) on system fail

        :param action: Action to be executed on failure
        :type action: Union[List[Action], Action]
        :param max_retries: Maximum number of action execution retries. None is equivalent to unlimited retries, defaults to None
        :type max_retries: Optional[int], optional
        """
        if self._is_valid_fallback_action(action):
            self.__fallbacks.on_system_fail = Fallback(
                action=action, max_retries=max_retries
            )

    def on_component_fail(
        self, action: Union[List[Action], Action], max_retries: Optional[int] = None
    ) -> None:
        """
        Set the fallback strategy (action) on component fail

        :param action: Action to be executed on failure
        :type action: Union[List[Action], Action]
        :param max_retries: Maximum number of action execution retries. None is equivalent to unlimited retries, defaults to None
        :type max_retries: Optional[int], optional
        """
        if self._is_valid_fallback_action(action):
            self.__fallbacks.on_component_fail = Fallback(
                action=action, max_retries=max_retries
            )

    def on_algorithm_fail(
        self, action: Union[List[Action], Action], max_retries: Optional[int] = None
    ) -> None:
        """
        Set the fallback strategy (action) on algorithm fail

        :param action: Action to be executed on failure
        :type action: Union[List[Action], Action]
        :param max_retries: Maximum number of action execution retries. None is equivalent to unlimited retries, defaults to None
        :type max_retries: Optional[int], optional
        """
        if self._is_valid_fallback_action(action):
            self.__fallbacks.on_algorithm_fail = Fallback(
                action=action, max_retries=max_retries
            )

    def on_giveup(
        self, action: Union[List[Action], Action], max_retries: int = 1
    ) -> None:
        """
        Set the fallback strategy (action) on giveup

        :param action: Action to be executed on give up
        :type action: Union[List[Action], Action]
        """
        if self._is_valid_fallback_action(action):
            self.__fallbacks.on_giveup = Fallback(
                action=action, max_retries=max_retries
            )

    @component_fallback
    def broadcast_status(self, **_) -> None:
        """
        Component fallback defined to only broadcast the current state so it is handled by an external manager.
        Used as the default fallback strategy for any system (external) failure
        """
        # If node is active publish status
        if (
            hasattr(self, "health_status_publisher")
            and self.lifecycle_state == LifecycleStateMsg.PRIMARY_STATE_ACTIVE
        ):
            self.health_status_publisher.publish(self.health_status())

    # LIFECYCLE ON TRANSITIONS CUSTOM METHODS
    @property
    def lifecycle_state(self) -> int:
        """
        lifecycle state machine current state getter

        :return: _description_
        :rtype: int
        """
        if hasattr(self, "_state_machine"):
            return self._state_machine.current_state[0]
        return 0

    def on_configure(
        self, state: lifecycle.State
    ) -> lifecycle.TransitionCallbackReturn:
        """
        Method on node state transition to Configured
        Declares node parameters and inits the base node (with initial flags and variables)
        A custom on configure method runs after component configuration

        :param state: Current node state
        :type state: lifecycle.State

        :return: Node state transition result
        :rtype: lifecycle.TransitionCallbackReturn
        """
        try:
            self.configure()

            # Call custom method
            self.custom_on_configure()

            self.health_status.set_healthy()
            self.get_logger().info(
                f"Node '{self.get_name()}' is in state '{state.label}'. Transitioning to 'configured'"
            )
        except Exception as e:
            self.get_logger().error(
                f"Transition error for node {self.get_name()} to transition to state 'configured': {e}"
            )
            return self.on_error(state)

        return super().on_configure(state)

    def on_activate(self, state: lifecycle.State) -> lifecycle.TransitionCallbackReturn:
        """
        Method on node state transition to Active
        Starts node subscriptions, publications, services and clients
        A custom activate method runs after component activation

        :param state: Current node state
        :type state: lifecycle.State

        :return: Node state transition result
        :rtype: lifecycle.TransitionCallbackReturn
        """
        try:
            self.activate()
            # Declare transition
            self.get_logger().info(
                f"Node '{self.get_name()}' is in state '{state.label}'. Transitioning to 'active'"
            )

            self._turn_on_events_management()

            # Create external processors
            self._attach_external_processors()

            # Create failure check timer
            self.__fallbacks_check_timer = self.create_timer(
                timer_period_sec=1 / self.config.fallback_rate,
                callback=self._fallbacks_check_callback,
                callback_group=MutuallyExclusiveCallbackGroup(),
            )

            self.health_status.set_healthy()

            # Call custom method
            self.custom_on_activate()

        except Exception as e:
            self.get_logger().error(
                f"Transition error for node {self.get_name()} to transition to state 'active': {e}"
            )
            return self.on_error(state)

        return super().on_activate(state)

    def on_deactivate(
        self, state: lifecycle.State
    ) -> lifecycle.TransitionCallbackReturn:
        """
        Method on node state transition to Deactivate.
        A custom deactivate method runs before component deactivation

        :param state: Current node state
        :type state: lifecycle.State

        :return: Node state transition result
        :rtype: lifecycle.TransitionCallbackReturn
        """
        try:
            self.destroy_timer(self.__fallbacks_check_timer)
            self.deactivate()
            # Declare transition
            self.get_logger().info(
                f"Node '{self.get_name()}' is in state '{state.label}'. Transitioning to 'inactive'"
            )

            self.health_status.set_healthy()

            # Call custom method
            self.custom_on_deactivate()

        except Exception as e:
            self.get_logger().error(
                f"Transition error for node {self.get_name()} to transition to state 'inactive': {e}"
            )
            return self.on_error(state)

        return super().on_deactivate(state)

    def on_shutdown(self, state: lifecycle.State) -> lifecycle.TransitionCallbackReturn:
        """
        Method on node state transition to Finalized
        A custom shutdown method runs before component shutdown

        :param state: Current node state
        :type state: lifecycle.State

        :return: Node state transition result
        :rtype: lifecycle.TransitionCallbackReturn
        """
        try:
            # Call custom method
            self.custom_on_shutdown()

            try_shutdown()
            self.get_logger().info(
                f"Node '{self.get_name()}' is in state '{state.label}'. Transitioning to 'finalized'"
            )
            self.health_status.set_healthy()
        except Exception as e:
            self.get_logger().error(
                f"Transition error for node {self.get_name()} to transition to state 'finalized': {e}"
            )
            return self.on_error(state)

        return super().on_shutdown(state)

    def on_cleanup(self, state: lifecycle.State) -> lifecycle.TransitionCallbackReturn:
        """
        Method on node state transition to unConfigured
        Starts node subscriptions, publications, services and clients
        A custom cleanup method runs before the component cleanup

        :param state: Current node state
        :type state: lifecycle.State

        :return: Node state transition result
        :rtype: lifecycle.TransitionCallbackReturn
        """
        try:
            # Call custom method
            self.custom_on_cleanup()

            # Declare transition
            self.get_logger().info(
                f"Node '{self.get_name()}' is in state '{state.label}'. Transitioning to 'unconfigured'"
            )

        except Exception as e:
            self.get_logger().error(
                f"Transition error for node {self.get_name()} to transition from state 'unconfigured': {e}"
            )
            return self.on_error(state)

        return super().on_cleanup(state)

    def on_error(
        self, state: lifecycle.LifecycleState
    ) -> lifecycle.TransitionCallbackReturn:
        """
        Handles a transition error
        A custom on error method runs before the component on error
        When a transition returns TransitionCallbackReturn.FAILURE or TransitionCallbackReturn.ERROR.
        """
        self.get_logger().error(
            f"Transition error for node {self.get_name()} - {state}"
        )
        self.custom_on_error()
        self.health_status.set_fail_component(component_names=[self.get_name()])

        # Trigger fallbacks manually if the fallback check timer is not already active
        if not hasattr(self, "__fallbacks_check_timer"):
            self._fallbacks_check_callback()

        # Wait before trying to retrigger failed transition
        self.get_logger().warning(
            f"Retriggering failed transition in {self.config._lifecycle_state_transition_timeout} sec..."
        )
        time.sleep(self.config._lifecycle_state_transition_timeout)

        # Attempt retriggering the transition
        if self.lifecycle_state == LifecycleStateMsg.TRANSITION_STATE_CONFIGURING:
            return self.on_configure(state)
        elif self.lifecycle_state == LifecycleStateMsg.TRANSITION_STATE_ACTIVATING:
            return self.on_activate(state)
        elif self.lifecycle_state == LifecycleStateMsg.TRANSITION_STATE_DEACTIVATING:
            return self.on_deactivate(state)
        elif self.lifecycle_state == LifecycleStateMsg.TRANSITION_STATE_CLEANINGUP:
            return self.on_cleanup(state)
        elif self.lifecycle_state == LifecycleStateMsg.TRANSITION_STATE_SHUTTINGDOWN:
            return self.on_shutdown(state)
        elif self.lifecycle_state == LifecycleStateMsg.TRANSITION_STATE_ERRORPROCESSING:
            return self.on_shutdown(state)

        return super().on_error(state)

    def custom_on_configure(self) -> None:
        """
        Method called on configure to overwrite with custom configuration
        """
        pass

    def custom_on_activate(self) -> None:
        """
        Method called on activation to overwrite with custom activation
        """

        pass

    def custom_on_deactivate(self) -> None:
        """
        Method called on deactivation to overwrite with custom deactivation
        """
        pass

    def custom_on_shutdown(self) -> None:
        """
        Method called on shutdown to overwrite with custom shutdown
        """
        pass

    def custom_on_error(self) -> None:
        """
        Method called on transition error to overwrite with custom transition error handling
        """
        pass

    def custom_on_cleanup(self) -> None:
        """
        Method called on cleanup to overwrite with custom cleanup
        """
        pass

    # NOTE: The following two methods added to add the fix from https://github.com/ros2/rclpy/pull/1319 merged into rolling on Dec 13, 2024. To be removed once backported to iron/humble/jazzy
    def __execute_transition_callback(
        self, current_state_id: int, previous_state: LifecycleState
    ) -> TransitionCallbackReturn:
        cb = self._callbacks.get(current_state_id, None)
        if cb is None:
            return TransitionCallbackReturn.SUCCESS
        try:
            ret = cb(previous_state)
            return ret
        except Exception as e:
            self.get_logger().error(f"Error executing state transition callback: {e}")
            return TransitionCallbackReturn.ERROR

    def _LifecycleNodeMixin__on_change_state(self, req, resp):
        """
        Overrides LifecycleNode ___on_change_state to avoid raising rcl_lifecycle error when 'change_state" service fails which otherwise would kill the process

        :param req: Lifecycle change state request
        :type req: lifecycle_msgs.srv.ChangeState.Request
        :param resp: Lifecycle change state response
        :type resp: lifecycle_msgs.srv.ChangeState.Response

        """
        # Check if node is initialized
        if not self._state_machine.initialized:
            self.get_logger().error(
                "Internal error: got service request while lifecycle state machine is not initialized."
            )
            resp.success = False
            return resp

        transition_id = req.transition.id

        # modification
        available_transition_ids = [
            t[0] for t in self._state_machine.available_transitions
        ]
        self.get_logger().debug(
            f"Available transitions for {self.node_name}: {available_transition_ids}, Requested {transition_id}"
        )

        if transition_id not in available_transition_ids:
            self.get_logger().warn(
                f"Invalid transition requested for for node {self.node_name}."
            )
            resp.success = False
            return resp

        initial_state = self._state_machine.current_state
        initial_state = LifecycleState(
            state_id=initial_state[0], label=initial_state[1]
        )
        self._state_machine.trigger_transition_by_id(transition_id, True)

        cb_return_code = self.__execute_transition_callback(
            self._state_machine.current_state[0], initial_state
        )
        self._state_machine.trigger_transition_by_label(cb_return_code.to_label(), True)

        if cb_return_code == TransitionCallbackReturn.ERROR:
            # Now we're in the errorprocessing state, trigger the on_error callback
            # and transition again based on the return code.
            error_cb_ret_code = self.__execute_transition_callback(
                self._state_machine.current_state[0], initial_state
            )
            self._state_machine.trigger_transition_by_label(
                error_cb_ret_code.to_label(), True
            )
        resp.success = cb_return_code == TransitionCallbackReturn.SUCCESS
        return resp
