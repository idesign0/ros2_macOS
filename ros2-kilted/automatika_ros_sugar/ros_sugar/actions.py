"""Actions

Pre-defined component and system level actions.

## Component-level Actions:
These actions operate directly on Sugarcoat `BaseComponent` instances to manage lifecycle, configuration, or intrinsic interfaces.
- start
- stop
- restart
- reconfigure
- update_parameter
- update_parameters
- trigger_component_service
- send_component_service_request
- trigger_component_action_server
- send_component_action_server_goal

## System-level Actions:
These actions interact with standard ROS2 interfaces (Topics, Services, Actions) and system utilities.
- log
- publish_message
- send_srv_request
- trigger_service
- send_action_goal
- trigger_action_server

## Usage Example:
```python
    from ros_sugar.actions import start, log, trigger_service

    # Component Lifecycle Action
    my_component = BaseComponent(node_name='test_component')
    action_start = start(component=my_component)

    # System/Logic Action
    action_log = log(msg="I am executing a cool action!")

    # ROS2 Interface Action
    action_srv = trigger_service(
        srv_name='/add_two_ints',
        srv_type=AddTwoInts
    )
```

"""

from functools import wraps
from typing import Any, Callable, List, Optional, Union

from .core.action import Action, LogInfo
from .core.component import BaseComponent
from .utils import InvalidAction
from .io.topic import Topic

__all__ = [
    "Action",
    "start",
    "stop",
    "restart",
    "reconfigure",
    "update_parameter",
    "update_parameters",
    "log",
    "publish_message",
    "send_srv_request",
    "trigger_service",
    "send_component_service_request",
    "trigger_component_service",
    "send_action_goal",
    "trigger_action_server",
    "send_component_action_server_goal",
    "trigger_component_action_server",
]


def _validate_component_action(function: Callable):
    """
    Decorator for to validate that a given action is a supported component action

    :param function:
    :type function: Callable
    """

    # NOTE: Although the validator is used as a decorated for methods taking only keyword arguments, *args is added to get cls/self arguments
    @wraps(function)
    def _wrapper(*args, **kwargs):
        """_summary_

        :param component: _description_
        :type component: BaseComponent
        :raises TypeError: _description_
        :return: _description_
        :rtype: _type_
        """
        if not kwargs.get("component"):
            raise InvalidAction(
                f"Component should be provided to use component action '{function.__name__}'"
            )
        component: Optional[BaseComponent] = kwargs.get("component")

        if component and not hasattr(component, function.__name__):
            raise InvalidAction(
                f"Component '{component.node_name}' does not support '{function.__name__}' action"
            )

        return function(*args, **kwargs)

    return _wrapper


def send_srv_request(*, srv_name: str, srv_type: type, srv_request_msg: Any) -> Action:
    """Action to send a ROS2 service request to a given service name/type

    :param srv_name: Service name
    :type srv_name: str
    :param srv_type: Service type (ROS2 service)
    :type srv_type: type
    :param srv_request_msg: Service request message
    :type srv_request_msg: Any

    :return: Sending request action
    :rtype: Action
    """
    # Combine positional arguments and keyword arguments
    kwargs = {
        "srv_name": srv_name,
        "srv_type": srv_type,
        "srv_request_msg": srv_request_msg,
    }
    # Action with an empty callable
    stack_action = Action(method=lambda *args, **kwargs: None, kwargs=kwargs)
    stack_action.action_name = "send_srv_request"
    stack_action._is_monitor_action = True
    return stack_action


def trigger_service(*, srv_name: str, srv_type: type) -> Action:
    """Action to trigger a ROS2 service.
    The action will try to create the service request out of the incoming event topic,
    if failed it will trigger the server with a default request value

    :param srv_name: Service name
    :type srv_name: str
    :param srv_type: Service type (ROS2 service)
    :type srv_type: type

    :return: Sending request action
    :rtype: Action
    """
    # Combine positional arguments and keyword arguments
    kwargs = {
        "srv_name": srv_name,
        "srv_type": srv_type,
    }

    # Action with an empty callable
    stack_action = Action(method=lambda *args, **kwargs: None, kwargs=kwargs)
    stack_action.action_name = "send_srv_request"
    stack_action._is_monitor_action = True
    # Set the type of the required argument (service request)
    # This is done to enable the action to find an automatic parser
    stack_action.set_required_runtime_arguments({"srv_request_msg": srv_type.Request})
    return stack_action


def send_component_service_request(
    *, component: BaseComponent, srv_request_msg: Any
) -> Action:
    """Action to send a ROS2 service request to a component's main service

    :param component: Sugarcoat Component
    :type component: BaseComponent
    :param srv_request_msg: Service request message
    :type srv_request_msg: Any

    :return: Sending request action
    :rtype: Action
    """
    if not component.main_srv_name or not component.service_type:
        raise NotImplementedError(
            f"Cannot use the action 'trigger_component_service' on component '{component.node_name}'. Component {component.node_name} does not have a main service implemented."
        )
    # Combine positional arguments and keyword arguments
    kwargs = {
        "srv_name": component.main_srv_name,
        "srv_type": component.service_type,
        "srv_request_msg": srv_request_msg,
    }
    # Action with an empty callable
    stack_action = Action(method=lambda *args, **kwargs: None, kwargs=kwargs)
    stack_action.action_name = "send_srv_request"
    stack_action._is_monitor_action = True
    return stack_action


def trigger_component_service(*, component: BaseComponent) -> Action:
    """Action to trigger a component's main service
    The action will try to create the service request out of the incoming event topic,
    if failed it will trigger the server with a default request value

    :param component: Sugarcoat Component
    :type component: BaseComponent

    :return: Sending request action
    :rtype: Action
    """
    if not component.main_srv_name or not component.service_type:
        raise NotImplementedError(
            f"Cannot use the action 'trigger_component_service' on component '{component.node_name}'. Component {component.node_name} does not have a main service implemented."
        )
    # Combine positional arguments and keyword arguments
    kwargs = {
        "srv_name": component.main_srv_name,
        "srv_type": component.service_type,
    }

    stack_action = Action(method=lambda *args, **kwargs: None, kwargs=kwargs)
    stack_action.action_name = "send_srv_request"
    stack_action._is_monitor_action = True
    # Set the type of the required argument (service request)
    stack_action.set_required_runtime_arguments({
        "srv_request_msg": component.service_type.Request
    })
    return stack_action


def send_action_goal(
    *, server_name: str, server_type: type, request_msg: Any
) -> Action:
    """Action to send a ROS2 action goal to a given ROS2 action server name and type

    :param action_name: ROS2 action name
    :type action_name: str
    :param action_type: ROS2 action type
    :type action_type: type
    :param action_request_msg: ROS2 action goal message
    :type action_request_msg: Any

    :return: Sending goal action
    :rtype: Action
    """
    # Combine positional arguments and keyword arguments
    kwargs = {
        "action_name": server_name,
        "action_type": server_type,
        "action_request_msg": request_msg,
    }

    stack_action = Action(method=lambda *args, **kwargs: None, kwargs=kwargs)
    stack_action.action_name = "send_action_goal"
    stack_action._is_monitor_action = True
    return stack_action


def trigger_action_server(*, server_name: str, server_type: type) -> Action:
    """Action to trigger a ROS2 action server with given name and type
    The action will try to create the service request out of the incoming event topic,
    if failed it will trigger the server with a default request value

    :param action_name: ROS2 action name
    :type action_name: str
    :param action_type: ROS2 action type
    :type action_type: type

    :return: Sending goal action
    :rtype: Action
    """
    # Combine positional arguments and keyword arguments
    kwargs = {
        "action_name": server_name,
        "action_type": server_type,
    }

    stack_action = Action(method=lambda *args, **kwargs: None, kwargs=kwargs)
    stack_action.action_name = "send_action_goal"
    stack_action._is_monitor_action = True
    # Set the type of the required argument (service request)
    stack_action.set_required_runtime_arguments({
        "action_request_msg": server_type.Goal
    })
    return stack_action


def send_component_action_server_goal(
    *, component: BaseComponent, request_msg: Any
) -> Action:
    """Action to send a ROS2 action to a component's main action server

    :param component: Sugarcoat Component
    :type component: BaseComponent
    :param action_request_msg: ROS2 action goal message
    :type action_request_msg: Any

    :return: Sending goal action
    :rtype: Action
    """
    # Combine positional arguments and keyword arguments
    kwargs = {
        "action_name": component.main_action_name,
        "action_type": component.action_type,
        "action_request_msg": request_msg,
    }

    stack_action = Action(method=lambda *args, **kwargs: None, kwargs=kwargs)
    stack_action.action_name = "send_action_goal"
    stack_action._is_monitor_action = True
    return stack_action


def trigger_component_action_server(*, component: BaseComponent) -> Action:
    """Action to trigger a component's action server
    The action will try to create the service request out of the incoming event topic,
    if failed it will trigger the server with a default request value

    :param component: Sugarcoat Component
    :type component: BaseComponent

    :return: Sending goal action
    :rtype: Action
    """
    if not component.main_action_name or not component.action_type:
        raise NotImplementedError(
            f"Cannot use the action 'trigger_main_action_server' on component '{component.node_name}'. Component {component.node_name} does not have a main action server implemented."
        )
    # Combine positional arguments and keyword arguments
    kwargs = {
        "action_name": component.main_action_name,
        "action_type": component.action_type,
    }

    stack_action = Action(method=lambda *args, **kwargs: None, kwargs=kwargs)
    stack_action.action_name = "send_action_goal"
    stack_action._is_monitor_action = True
    # Set the type of the required argument (action request)
    stack_action.set_required_runtime_arguments({
        "action_request_msg": component.action_type.Goal
    })
    return stack_action


def publish_message(
    *,
    topic: Topic,
    msg: Any,
    publish_rate: Optional[float] = None,
    publish_period: Optional[float] = None,
) -> Action:
    """Action to publish a ROS2 message on a given topic.
    If both publish_rate and publish_period are not provided, the message will be published once

    :param topic: Topic to publish
    :type topic: Topic
    :param msg: Message to publish
    :type msg: Any
    :param publish_rate: Publishing rate (Hz), defaults to None
    :type publish_rate: Optional[float], optional
    :param publish_period: Publishing period (s), defaults to None
    :type publish_period: Optional[float], optional

    :return: Publish message action
    :rtype: Action
    """
    # Combine positional arguments and keyword arguments
    kwargs = {
        "topic": topic,
        "msg": msg,
        "publish_rate": publish_rate,
        "publish_period": publish_period,
    }

    stack_action = Action(method=lambda *args, **kwargs: None, kwargs=kwargs)
    stack_action.action_name = "publish_message"
    stack_action._is_monitor_action = True
    return stack_action


@_validate_component_action
def start(*, component: BaseComponent) -> Action:
    """Action to start a given component

    :param component: Component
    :type component: BaseComponent

    :return: Component start action
    :rtype: Action
    """
    # Action with an empty callable
    stack_action = Action(method=component.start)
    stack_action._is_lifecycle_action = True
    return stack_action


@_validate_component_action
def stop(*, component: BaseComponent) -> Action:
    """Action to stop a given component

    :param component: Component
    :type component: BaseComponent

    :return: Component stop action
    :rtype: Action
    """
    # Action with an empty callable
    stack_action = Action(method=component.stop)
    stack_action._is_lifecycle_action = True
    return stack_action


@_validate_component_action
def restart(*, component: BaseComponent, wait_time: Optional[float] = None) -> Action:
    """Action to restart a given component

    :param component: Component
    :type component: BaseComponent
    :param wait_time: Optional wait time n seconds between stop and start, defaults to None (i.e. no wait)
    :type wait_time: Optional[float]

    :return: Component restart action
    :rtype: Action
    """
    # Action with an empty callable
    stack_action = Action(method=component.restart, kwargs={"wait_time": wait_time})
    stack_action._is_lifecycle_action = True
    return stack_action


def reconfigure(
    *,
    component: BaseComponent,
    new_config: Union[str, object],
    keep_alive: bool = False,
) -> Action:
    """Action to reconfigure a given component

    :param component: Component
    :type component: BaseComponent
    :param new_config: Component config class or path to config file
    :type new_config: Union[str, object]
    :param keep_alive: To keep the component running when reconfiguring, defaults to False
    :type keep_alive: bool, optional

    :return: Component reconfigure action
    :rtype: Action
    """
    kwargs = {
        "component": component,
        "new_config": new_config,
        "keep_alive": keep_alive,
    }

    if not isinstance(new_config, str) and not isinstance(
        new_config, component.config.__class__
    ):
        raise TypeError(
            f"Incompatible config type '{type(new_config)}'. Cannot reconfigure {component.node_name}. config should be either a '{component.config.__class__}' instance or 'str' with path to valid config file (yaml, json, toml)"
        )
    # Action with an empty callable
    stack_action = Action(method=lambda *args, **kwargs: None, kwargs=kwargs)
    stack_action.action_name = "configure_component"
    stack_action._is_monitor_action = True
    return stack_action


def update_parameter(
    *,
    component: BaseComponent,
    param_name: str,
    new_value: Any,
    keep_alive: bool = True,
) -> Action:
    """Action to update (change) the value of a component config parameter

    :param component: Component
    :type component: BaseComponent
    :param param_name: Parameter name
    :type param_name: str
    :param new_value: Parameter value
    :type new_value: Any
    :param keep_alive: To keep the component running when updating the value, defaults to True
    :type keep_alive: bool, optional

    :return: Component parameter update action
    :rtype: Action
    """
    kwargs = {
        "component": component,
        "param_name": param_name,
        "new_value": new_value,
        "keep_alive": keep_alive,
    }
    # Action with an empty callable
    stack_action = Action(method=lambda *args, **kwargs: None, kwargs=kwargs)
    stack_action.action_name = "update_parameter"
    stack_action._is_monitor_action = True
    return stack_action


def update_parameters(
    *,
    component: BaseComponent,
    params_names: List[str],
    new_values: List,
    keep_alive: bool = True,
) -> Action:
    """Action to update (change) the values of a list of component config parameters

    :param component: Component
    :type component: BaseComponent
    :param params_names: Parameters names
    :type params_names: List[str]
    :param new_values: Parameters values
    :type new_values: List
    :param keep_alive: To keep the component running when updating the value, defaults to True
    :type keep_alive: bool, optional

    :return: Component parameter update action
    :rtype: Action
    """
    kwargs = {
        "component": component,
        "params_names": params_names,
        "new_values": new_values,
        "keep_alive": keep_alive,
    }

    # Action with an empty callable
    stack_action = Action(method=lambda *args, **kwargs: None, kwargs=kwargs)
    # Setup Monitor action
    stack_action.action_name = "update_parameters"
    stack_action._is_monitor_action = True
    return stack_action


def log(*, msg: str, logger_name: Optional[str] = None) -> LogInfo:
    """Action to log a message.

    :param msg:
    :type msg: str
    :param logger_name:
    :type logger_name: Optional[str]
    :rtype: LogInfo
    """
    return LogInfo(msg=msg, logger_name=logger_name)
