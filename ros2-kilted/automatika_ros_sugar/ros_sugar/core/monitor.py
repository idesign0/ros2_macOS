"""Monitor"""

import os
from functools import partial
import time
from typing import Any, Callable, Dict, List, Optional, Union
from rclpy.node import Node
from rclpy.publisher import Publisher
from rclpy.callback_groups import MutuallyExclusiveCallbackGroup, ReentrantCallbackGroup
from automatika_ros_sugar.msg import ComponentStatus
from automatika_ros_sugar.srv import (
    ChangeParameter,
    ChangeParameters,
    ConfigureFromFile,
    ReplaceTopic,
)

from .. import base_clients
from .component import BaseComponent
from ..config import BaseConfig
from ..io.topic import Topic
from .event import Event, EventBlackboardEntry
from .action import Action
from ..launch import logger


class Monitor(Node):
    """
    Monitor is a ROS2 Node (not Lifecycle) responsible of monitoring the status of the stack (rest of the running nodes) and managing requests/responses from the Orchestrator.

    :::{note} When launching the stack using the Launcher, the user is not required to configure the Monitor. The Launcher will configure and launch its own Monitor internally.
    :::

    ## Main Functionalities:
    - Creates Subscribers to registered Events. The Monitor is configured to declare an InternalEvent back to the Launcher so the corresponding Action can be executed (see source implementation in launch_actions.py)
    - Creates Subscribers to all registered Components health status topics
    - Creates clients for all components main services and main action servers
    - Creates service clients to components reconfiguration services to handle actions sent from the Launcher
    """

    def __init__(
        self,
        components_names: List[str],
        events_actions: Optional[Dict[str, List[Action]]] = None,
        events_to_emit: Optional[List[Event]] = None,
        config: Optional[BaseConfig] = None,
        services_components: Optional[List[BaseComponent]] = None,
        action_servers_components: Optional[List[BaseComponent]] = None,
        activate_on_start: Optional[List[str]] = None,
        activation_timeout: Optional[float] = None,
        activation_attempt_time: float = 1.0,
        component_name: str = "monitor",
        **_,
    ):
        """
        Setup the Monitor node

        :param components_names: List containing the ROS2 Node names of the components to be monitored
        :type components_names: List[str]
        :param events: List of Events to be monitored, defaults to None
        :type events: Optional[List[Event]], optional
        :param actions: Dictionary that associates each Event to a valid Action. Should be provided in case events is not None. An error is thrown if an invalid event name , defaults to None
        :type actions: Optional[Dict[str, Action]], optional
        :param config: Basic node configuration, defaults to None
        :type config: Optional[BaseConfig], optional
        :param services_components: List of components running as Servers, defaults to None
        :type services_components: Optional[List[Component]], optional
        :param action_servers_components: List of components running as Action Servers, defaults to None
        :type action_servers_components: Optional[List[Component]], optional
        :param activate_on_start: List of Lifecycle components names to activate on start, defaults to None
        :type activate_on_start: Optional[List[str]], optional
        :param start_on_init: To activate provided components on start, defaults to False
        :type start_on_init: bool, optional
        :param component_name: Name of the ROS2 node, defaults to "monitor"
        :type component_name: str, optional
        :param callback_group: Callback group, defaults to None
        :type callback_group:  Optional[Union[MutuallyExclusiveCallbackGroup, ReentrantCallbackGroup]], optional
        """
        self._events_actions = events_actions
        self._internal_events = events_to_emit
        self._components_to_monitor = components_names
        self._service_components = services_components
        self._action_components = action_servers_components
        self.node_name = f"{component_name}_{os.getpid()}"
        self.config = config or BaseConfig()

        # Server nodes handlers
        self._update_parameter_srv_client: Dict[
            str, base_clients.ServiceClientHandler
        ] = {}
        self._update_parameters_srv_client: Dict[
            str, base_clients.ServiceClientHandler
        ] = {}
        self._topic_change_srv_client: Dict[str, base_clients.ServiceClientHandler] = {}
        self._configure_from_file_srv_client: Dict[
            str, base_clients.ServiceClientHandler
        ] = {}
        self._main_srv_clients: Dict[str, base_clients.ServiceClientHandler] = {}
        self._main_action_clients: Dict[str, base_clients.ActionClientHandler] = {}

        self._components_to_activate_on_start: List[str] = activate_on_start or []

        self.__components_activation_event: Optional[Callable] = None

        # Handle timeout when waiting for looking for the components to activate
        self.__activation_timeout = activation_timeout
        self.__activation_attempt_time = activation_attempt_time

        # Emit exit all to the launcher
        self._emit_exit_to_launcher: Optional[Callable] = None

    def rclpy_init_node(self, *args, **kwargs):
        """
        To init the node with rclpy and activate default services
        """
        Node.__init__(self, self.node_name, *args, **kwargs)
        self.get_logger().info(f"NODE {self.get_name()} STARTED")

    def add_components_activation_event(self, method) -> None:
        """
        Adds a method to be executed when components are activated

        :param method: Method to be executed on components activation
        :type method: Callable
        """
        self.__components_activation_event = method

    def activate(self):
        """Activate all subscribers/publishers/etc..."""
        # Create a timer for components activation
        if self._components_to_activate_on_start:
            callback_group = MutuallyExclusiveCallbackGroup()
            self.__activation_wait_time: float = 0.0
            self.__components_monitor_timer = self.create_timer(
                timer_period_sec=self.__activation_attempt_time,
                callback=self._check_and_activate_components,
                callback_group=callback_group,
            )

        # Create health status subscribers
        if self._components_to_monitor:
            self._create_status_subscribers()
            for component_name in self._components_to_monitor:
                self._turn_on_component_management(component_name)

        # Activate event monitoring
        self._activate_event_monitoring()

        # Create main services clients
        if self._service_components is not None:
            for component in self._service_components:
                self.get_logger().info(
                    f"Creating Main Service Client for {component.node_name}"
                )
                self._main_srv_clients[component.node_name] = (
                    base_clients.ServiceClientHandler(
                        client_node=self,
                        srv_type=component.service_type,
                        srv_name=component.main_srv_name,
                    )
                )

        # Create main action clients
        if self._action_components is not None:
            for component in self._action_components:
                self.get_logger().info(
                    f"Creating Main Action Client for {component.node_name} with name {component.main_action_name} and type {component.action_type}"
                )
                self._main_action_clients[component.node_name] = (
                    base_clients.ActionClientHandler(
                        client_node=self,
                        action_type=component.action_type,
                        action_name=component.main_action_name,
                    )
                )

    def _check_and_activate_components(self) -> None:
        """
        Checks and activates requested components
        """
        self.__activation_wait_time += self.__activation_attempt_time
        node_names = self.get_node_names()
        __notfound: Optional[set[str]] = None
        if set(self._components_to_activate_on_start).issubset(set(node_names)):
            logger.info(
                f"NODES '{self._components_to_activate_on_start}' ARE UP ... ACTIVATING"
            )
            if self.__components_activation_event:
                self.__components_activation_event()
            self.destroy_timer(self.__components_monitor_timer)
        else:
            __notfound = set(self._components_to_activate_on_start).difference(
                set(node_names)
            )
            logger.info(f"Waiting for Nodes '{__notfound}' to come up to activate ...")
        if (
            self.__activation_timeout
            and self.__activation_wait_time > self.__activation_timeout
        ):
            if self._emit_exit_to_launcher:
                self._emit_exit_to_launcher()

            raise LookupError(
                f"Timeout while Waiting for nodes '{__notfound}' to come up to activate. A process might have died. If all processes are starting without errors, then this might be a ROS2 discovery problem. Run 'ros2 node list' to see if nodes with the same name already exist or old nodes are not killed properly. Alternatively, try to restart ROS2 daemon."
            )

    def _turn_on_component_management(self, component_name: str) -> None:
        """
        Created clients for all main services in a given component
        - Change a component parameter
        - Change a set of component parameters
        - Replace a topic
        - Reconfigure component from file

        :param component_name: Name of the component (ROS node name)
        :type component_name: str
        """
        self._update_parameter_srv_client[component_name] = (
            base_clients.ServiceClientHandler(
                client_node=self,
                srv_type=ChangeParameter,
                srv_name=f"/{component_name}/update_config_parameter",
            )
        )

        self._update_parameters_srv_client[component_name] = (
            base_clients.ServiceClientHandler(
                client_node=self,
                srv_type=ChangeParameters,
                srv_name=f"{component_name}/update_config_parameters",
            )
        )

        # Input/Output update services

        self._topic_change_srv_client[component_name] = (
            base_clients.ServiceClientHandler(
                client_node=self,
                srv_type=ReplaceTopic,
                srv_name=f"{component_name}/change_topic",
            )
        )

        self._configure_from_file_srv_client[component_name] = (
            base_clients.ServiceClientHandler(
                client_node=self,
                srv_type=ConfigureFromFile,
                srv_name=f"{component_name}/configure_from_file",
            )
        )

    def configure_component(
        self,
        component: BaseComponent,
        new_config: Union[object, str],
        keep_alive: bool,
    ) -> None:
        """
        Configure a given component from config instance or config file
        Creates and send the request to the component service

        :param component: Component to configure
        :type component: BaseComponent
        :param config: Config instance or path to config file
        :type config: object | str
        :param keep_alive: To keep the component running while configuring
        :type keep_alive: bool
        :param executor: Used to spin the monitor node until the service response is received, defaults to None
        :type executor: ROS Executor, optional
        """
        try:
            # For config instance prepare change parameters request
            if isinstance(new_config, component.config.__class__):
                request_msg: ChangeParameters.Request = (
                    component.get_change_parameters_msg_from_config(new_config)
                )
                request_msg.keep_alive = keep_alive
                self._update_parameters_srv_client[component.node_name].send_request(
                    request_msg, executor=self.executor
                )
            else:
                # For string send a configure from file request
                request_msg_file = ConfigureFromFile.Request()
                request_msg_file.path_to_file = new_config
                self._configure_from_file_srv_client[component.node_name].send_request(
                    request_msg_file, executor=self.executor
                )
        except Exception as e:
            self.get_logger().error(
                f"Unable to configure component {component.node_name}: {e}"
            )

    def update_parameter(
        self,
        component: BaseComponent,
        param_name: str,
        new_value: Any,
        keep_alive: bool = True,
    ) -> None:
        """Sends a ChangeParameter service request to given component

        :param component: _description_
        :type component: BaseComponent
        :param param_name: _description_
        :type param_name: str
        :param new_value: _description_
        :type new_value: Any
        :param keep_alive: _description_, defaults to True
        :type keep_alive: bool, optional
        """
        srv_client: base_clients.ServiceClientHandler = (
            self._update_parameter_srv_client[component.node_name]
        )
        srv_request = ChangeParameter.Request()
        srv_request.name = param_name
        srv_request.value = str(new_value)
        srv_request.keep_alive = keep_alive
        srv_client.send_request(req_msg=srv_request, executor=self.executor)

    def update_parameters(
        self,
        component: BaseComponent,
        params_names: List[str],
        new_values: List,
        keep_alive: bool = True,
        **_,
    ) -> None:
        """Sends a ChangeParameters service request to given component

        :param component: _description_
        :type component: BaseComponent
        :param params_names: _description_
        :type params_names: List[str]
        :param new_values: _description_
        :type new_values: List
        :param keep_alive: _description_, defaults to True
        :type keep_alive: bool, optional
        """
        srv_client: base_clients.ServiceClientHandler = (
            self._update_parameters_srv_client[component.node_name]
        )
        srv_request = ChangeParameters.Request()
        srv_request.names = params_names
        srv_request.values = str(new_values)
        srv_request.keep_alive = keep_alive
        srv_client.send_request(req_msg=srv_request, executor=self.executor)

    def __get_srv_client(
        self, srv_name: str, srv_type: type
    ) -> base_clients.ServiceClientHandler:
        """Helper method to get a service client handler for the provided service name/type

        :param srv_name: Service name
        :type srv_name: str
        :param srv_type: Service type (ROS2 service)
        :type srv_type: type

        :return: Service client handler
        :rtype: base_clients.ServiceClientHandler
        """
        # Check if the client is already created (clients are created for main component services)
        for main_srv_client in self._main_srv_clients.values():
            if (
                main_srv_client.config.name == srv_name
                and main_srv_client.config.srv_type == srv_type
            ):
                return main_srv_client
        # If no return -> service client does not exist -> create it
        return base_clients.ServiceClientHandler(
            client_node=self, srv_name=srv_name, srv_type=srv_type
        )

    def __get_action_client(
        self, action_name: str, action_type: type
    ) -> base_clients.ActionClientHandler:
        """Helper method to get a ros action client handler for the provided service name/type

        :param action_name: Action name
        :type action_name: str
        :param action_type: Action type (ROS2 action)
        :type action_type: type

        :return: Action client handler
        :rtype: base_clients.ActionClientHandler
        """
        # Check if the client is already created (clients are created for main component services)
        for main_action_client in self._main_action_clients.values():
            if (
                main_action_client.config.name == action_name
                and main_action_client.config.action_type == action_type
            ):
                return main_action_client
        # If no return -> service client does not exist -> create it
        return base_clients.ActionClientHandler(
            client_node=self, action_name=action_name, action_type=action_type
        )

    def send_srv_request(
        self,
        srv_request_msg: Any = None,
        srv_name: Optional[str] = None,
        srv_type: Optional[type] = None,
        **_,
    ) -> None:
        """Action to send a ROS2 service request during runtime

        :param srv_name: Service name
        :type srv_name: str
        :param srv_type: Service type (ROS2 service)
        :type srv_type: type
        :param srv_request_msg: Service request message
        :type srv_request_msg: Any
        """
        if not srv_name or not srv_type:
            self.get_logger().error(
                f"Cannot send service request to unknown ROS2 service with name: {srv_name} and type {srv_type}"
            )
            return
        if not srv_request_msg:
            # If request is not provided create an empty one
            srv_request_msg = srv_type.Request()
        srv_client = self.__get_srv_client(srv_name, srv_type)
        srv_client.send_request(srv_request_msg, executor=self.executor)

    def send_action_goal(
        self,
        action_request_msg: Any = None,
        action_name: Optional[str] = None,
        action_type: Optional[type] = None,
        **_,
    ) -> None:
        """Action to send a ROS2 action goal during runtime

        :param action_name: ROS2 action name
        :type action_name: str
        :param action_type: ROS2 action type
        :type action_type: type
        :param action_request_msg: ROS2 action goal message
        :type action_request_msg: Any
        """
        if not action_name or not action_type:
            self.get_logger().error(
                f"Cannot send service request to unknown ROS2 service with name: {action_name} and type {action_type}"
            )
            return
        if not action_request_msg:
            # If request is not provided create an empty one
            action_request_msg = action_type.Goal()
        action_client = self.__get_action_client(action_name, action_type)
        action_client.send_request(action_request_msg)

    def get_secs_time(self) -> float:
        ros_time = self.get_clock().now().to_msg()
        return float(ros_time.sec + (1e-9 * ros_time.nanosec))

    def publish_message(
        self,
        topic: Topic,
        msg: Any,
        publish_rate: Optional[float] = None,
        publish_period: Optional[float] = None,
        **_,
    ) -> None:
        """Action to publish a message to a given topic

        :param topic: Published topic
        :type topic: Topic
        :param msg: Published message
        :type msg: Any
        :param publish_rate: Publishing rate, if None the message is published once, defaults to None
        :type publish_rate: Optional[float], optional
        :param publish_period: Publishing period, if none and rate is given the message is published forever, defaults to None
        :type publish_period: Optional[float], optional
        """
        publisher: Publisher = self.create_publisher(
            msg_type=topic.ros_msg_type,
            topic=topic.name,
            qos_profile=topic.qos_profile.to_ros(),
        )
        # Publish once
        if not publish_rate and not publish_period:
            publisher.publish(msg)
            self.destroy_publisher(publisher)
        elif publish_rate and not publish_period:
            # Publish forever
            self.create_timer(
                timer_period_sec=1 / publish_rate,
                callback=partial(publisher.publish, msg),
            )
        elif publish_rate and publish_period:
            # Publish with rate for given period
            max_time: float = self.get_secs_time() + publish_period
            timer_name: str = f"timer_{topic.name}_"
            callback = partial(
                self._timer_publish_msg_loop,
                timer_name=timer_name,
                max_time=max_time,
                publisher=publisher,
                msg=msg,
            )
            setattr(
                self,
                timer_name,
                self.create_timer(timer_period_sec=1 / publish_rate, callback=callback),
            )

    def _timer_publish_msg_loop(
        self, timer_name: str, max_time: float, publisher: Publisher, msg: Any
    ) -> None:
        """Timer callback to publish a message until max_time is reached then destroy the timer

        :param max_time: _description_
        :type max_time: float
        :param publisher: _description_
        :type publisher: Publisher
        :param msg: _description_
        :type msg: Any
        """
        time_now: float = self.get_secs_time()
        if time_now > max_time:
            self.destroy_timer(getattr(self, timer_name))
            self.destroy_publisher(publisher)
            return
        publisher.publish(msg)

    def __event_topic_callback(self, topic_name: str, msg: Any):
        """
        Central Handler:
        1. Updates Cache of all required events topics
        2. Re-evaluates all events that depend on this topic
        """
        # Update Blackboard
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

    def _activate_event_monitoring(self) -> None:
        """
        Turn on all events
        """
        self.__events = []
        if self._events_actions:
            for serialized_event, actions in self._events_actions.items():
                event = Event.from_json(serialized_event)
                for action in actions:
                    method = getattr(self, action.action_name)
                    # register action to the event
                    action.executable = partial(method, *action._args, **action._kwargs)
                    event.register_actions(action)
                self.__events.append(event)

        if self._internal_events:
            # Add internal events (to emit back to launcher)
            self.__events.extend(self._internal_events)

        # TURN ON EVENTS MANAGEMENT
        # Blackboard to store latest messages for all topics required for all event:
        # {'topic_1_name': RosMsg, 'topic_2_name': ROSMsg, ... }
        self._events_topics_blackboard: Dict[str, EventBlackboardEntry] = {}

        # Identify all unique topics required across ALL events
        unique_topics: Dict[str, Topic] = {}
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

    def _create_status_subscribers(self) -> None:
        """
        Creates subscribers to all the health status topics of the components
        """
        # Reentrant group for multi threaded monitoring
        callback_group = ReentrantCallbackGroup()
        for component_name in self._components_to_monitor:
            logger.debug(f"Creating health status subscriber for: {component_name}")
            self.create_subscription(
                ComponentStatus,
                topic=f"{component_name}/status",
                callback=partial(
                    self._status_check_callback, component_name=component_name
                ),
                qos_profile=10,
                callback_group=callback_group,
            )

    def _status_check_callback(self, msg, component_name: str):
        """
        Callback to check the health status of a component

        :param msg: Health status message
        :type msg: ComponentStatus
        :param component: Node under check
        :type component: Component
        """
        # TODO: handle status
        self.get_logger().debug(f"Form {component_name} got status {msg}")
