import threading
from functools import partial
from typing import List, Optional, Union

import launch
import rclpy
from launch import event_handlers
from launch.action import Action as ROSAction
from launch_ros.actions import Node as NodeLaunchAction
from rclpy.context import Context
from rclpy.executors import MultiThreadedExecutor
from rclpy.impl.logging_severity import LoggingSeverity
from rclpy.lifecycle.managed_entity import ManagedEntity
from rclpy.logging import set_logger_level

from . import logger
from ..core.event import InternalEvent
from ..core.monitor import Monitor
from ..core.component import BaseComponent


class ComponentLaunchAction(NodeLaunchAction):
    """ComponentLaunchAction."""

    def __init__(
        self,
        *,
        node: Union[BaseComponent, Monitor],
        name: str = "node_name",
        namespace: Union[str, List[launch.Substitution], None] = None,
        log_level: LoggingSeverity = LoggingSeverity.INFO,
        **kwargs,
    ) -> None:
        """Launch action to start a BaseComponent with the Launcher in a separate thread

        :param node:
        :type node: Union[BaseComponent, Monitor]
        :param name:
        :type name: Union[str , List[launch.Substitution], None]
        :param namespace:
        :type namespace: Union[str, List[launch.Substitution], None]
        :param log_level:
        :type log_level: LoggingSeverity
        :param kwargs:
        :rtype: None
        """
        self.__ros_node = node
        self.__node_name = name
        self.__ros_executor = None
        self.__log_level = log_level
        self.__logger = logger

        NodeLaunchAction.__init__(
            self,
            name=name,
            namespace=namespace,
            executable="",
            **kwargs,
        )

    @property
    def name(self) -> Union[str, List[launch.Substitution], None]:
        """
        Node name getter

        :return: _description_
        :rtype: str
        """
        return self.__node_name

    def _on_internal_event(self, event_name: str, **_):
        """
        Executed on a trigger of any internal event in the node
        The triggered event_name corresponds to the key name in the events dictionary provided to the monitor node. See kompass_ros.nodes.health_monitor
        :param event_name: Event key name to identify which events got triggered
        :type event_name: str
        """
        try:
            # Create a launch event of type InternalEvent with the event name
            event = InternalEvent(
                event_name=event_name,
                topics_value={},
            )

            def func():
                # Update values from node
                event.topics_value = {
                    key: entry.msg
                    for key, entry in self.__ros_node._events_topics_blackboard.items()
                }
                self.__context.emit_event_sync(event)

            # Emit the event to launch context
            # Safety check to make sure the asyncio loop is initialized
            if self.__context.asyncio_loop:
                self.__context.asyncio_loop.call_soon_threadsafe(func)
            else:
                func()
        except Exception as exc:
            self.__logger.error("Exception in emitting event': {}".format(exc))

    def execute(self, context: launch.LaunchContext) -> Optional[List[ROSAction]]:
        """
        Overrides execute action of launch_ros.actions.Node

        1- Adds the _on_internal_event method which emits the event to launch to every event handler in the monitor. This allows all internal events to be declared tl launch context on trigger

        2- Inits and activates a Monitor node

        3- Executes the Monitor node in a separate thread

        Delegated to :meth:`launch.actions.ExecuteProcess.execute`.
        """
        # Set the launch context
        self.__context = context

        # FOR MONITOR NODE
        # Adds event emit method to all internal events if this is the Monitor Node
        if isinstance(self.__ros_node, Monitor):
            if self.__ros_node._internal_events:
                for event in self.__ros_node._internal_events:
                    self.__logger.debug(f"Registering internal event '{event}'")
                    # Register a method to emit the event to the launch context on trigger
                    event.register_actions(partial(self._on_internal_event, event.id))
            if hasattr(self.__ros_node, "_emit_exit_to_launcher"):
                self.__ros_node._emit_exit_to_launcher = partial(
                    self._on_internal_event, "exit_all"
                )

            # Adds an emit event for components activation
            self.__logger.debug("Registering Conditional Activation Handle")
            self.__ros_node.add_components_activation_event(
                partial(self._on_internal_event, "activate_all")
            )

        # Get rclpy context and init the monitor
        self.__ros_context = Context()
        rclpy.init(context=self.__ros_context)
        set_logger_level(self.__node_name, self.__log_level)

        self.__ros_node.rclpy_init_node(context=self.__ros_context)

        if not issubclass(self.__ros_node.__class__, ManagedEntity):
            # Activate Non lifecycle nodes
            self.__ros_node.activate()

        # Get a multi-threaded executor
        self.__ros_executor = MultiThreadedExecutor(context=self.__ros_context)
        self.__is_running = True

        context.extend_globals({f"{self.name}": self.__ros_node})

        context.register_event_handler(
            event_handlers.OnShutdown(on_shutdown=lambda *_: self.shutdown())
        )

        self.__ros_executor_thread = threading.Thread(target=self._run, daemon=True)

        self.__ros_executor_thread.start()

    @property
    def executor(self):
        """
        Getter of component executor

        :return: _description_
        :rtype: _type_
        """
        return self.__ros_executor

    def _run(self):
        """
        Overrides _run method of launch_ros.actions.Node to spin using the monitor loop_rate
        """
        if not self.__ros_executor:
            raise Exception("Node executor is unknown")
        try:
            self.__ros_executor.add_node(self.__ros_node)
            while self.__is_running:
                # TODO: switch this to `spin()` when it considers
                #   asynchronously added subscriptions.
                self.__ros_executor.spin_once(
                    timeout_sec=1 / self.__ros_node.config.loop_rate
                )
        except KeyboardInterrupt:
            pass
        finally:
            self.__ros_executor.remove_node(self.__ros_node)

    def shutdown(self):
        """Shutdown Node"""
        if not self.__is_running:
            raise RuntimeError(f"Cannot shutdown - Node {self.name} is not running")
        self.__is_running = False
        self.__ros_executor_thread.join()
        self.__ros_node.destroy_node()
        rclpy.shutdown(context=self.__ros_context)
