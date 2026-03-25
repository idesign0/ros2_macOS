import inspect
from enum import IntEnum as BaseIntEnum
from functools import wraps
from typing import Callable, List, Union, TypeVar

from rclpy.utilities import ok as rclpy_is_ok
from rclpy.lifecycle import Node as LifecycleNode
from launch import LaunchContext
from launch.actions import OpaqueFunction
import os
import logging


# Get ROS distro
__installed_distro = os.environ.get("ROS_DISTRO", "").lower()

try:
    if __installed_distro in ["humble", "galactic", "foxy"]:
        # Get some_action_type for older distributions
        from launch.some_actions_type import SomeActionsType as SomeEntitiesType
    else:
        from launch.some_entities_type import SomeEntitiesType
except ModuleNotFoundError as e:
    raise ModuleNotFoundError(
        "Could not determine correct ROS version. Make sure ROS_DISTRO variable is set and ROS_DISTRO is >= `humble`"
    ) from e


# logger for utils
logger = logging.getLogger("Sugarcoat")


# Define a generic type variable for topic message types
MsgT = TypeVar("MsgT")


class IncompatibleSetup(Exception):
    """Exception raised when a component is configured with incompatible parameter values"""

    pass


class InvalidAction(Exception):
    """Exception raised when an Action is invalid or configured with incompatible values"""

    pass


class IntEnum(BaseIntEnum):
    """
    Extends enum.IntEnum class with methods to get all integer values and get enum value corresponding to given int value
    """

    @classmethod
    def get_enum(cls, __value: int) -> Union[int, None]:
        """
        Get Enum members equal to given values

        :param __value: Reference value
        :type __value: int
        :return: Enum value
        :rtype: int | None
        """
        for enum_member in cls:
            if enum_member.value == __value:
                return enum_member
        return None

    @classmethod
    def values(cls):
        """
        Get all enum values
        """
        return [member.value for member in cls]


def action_handler(function: Callable):
    """
    Decorator for components action handlers
    Verifies that the return type is a ros launch some_entities_type required for event handling

    :param function:
    :type function: Callable
    """

    @wraps(function)
    def _wrapper(*args, **kwargs):
        """_wrapper.
        :param a:
        :param kw:
        """
        return_type = inspect.signature(function).return_annotation
        if return_type is not SomeEntitiesType and return_type != "SomeEntitiesType":
            raise TypeError(
                f"Action handlers must return launch event handlers 'launch.some_entities_type.SomeEntitiesType'. Method '{function.__name__}' cannot have '@action_handler' decorator"
            )

        return function(*args, **kwargs)

    return _wrapper


def component_action(function: Callable, active: bool = False):
    """
    Decorator for components actions
    Verifies that the function is a valid Component method, returns a boolean or None, and that the Component is active

    :param function:
    :type function: Callable
    """

    @wraps(function)
    def _wrapper(*args, **kwargs):
        """_wrapper.
        :param a:
        :param kw:
        """
        if not args:
            raise TypeError(f"'{function.__name__}' is not a valid Component method")

        self = args[0]
        if not isinstance(self, LifecycleNode):
            raise TypeError(f"'{function.__name__}' is not a valid Component method")

        # Check return type
        return_type = inspect.signature(function).return_annotation
        if return_type is not bool and return_type:
            raise TypeError(
                f"Action methods must return boolean or None. Method '{function.__name__}' cannot have '@component_action' decorator"
            )

        # Check Component is active
        if rclpy_is_ok() and hasattr(self, "_state_machine"):
            # check for active flag and if the flag is True, check lifecycle_state is 3 i.e. active
            if not active or self._state_machine.current_state[1] == "active":
                return function(*args, **kwargs)
            else:
                logger.error(
                    f"Cannot use component action method '{function.__name__}' without activating the Component"
                )
                return None
        else:
            logger.error(
                f"Cannot use component action method '{function.__name__}' without initializing rclpy and the Component"
            )

    _wrapper.__name__ = function.__name__

    return _wrapper


def component_fallback(function: Callable):
    """
    Decorator for components fallback methods
    Verifies that rcply is initialized and component is configured or active

    :param function:
    :type function: Callable
    """

    @wraps(function)
    def _wrapper(*args, **kwargs):
        """_wrapper.
        :param a:
        :param kw:
        """
        if not args:
            raise TypeError(f"'{function.__name__}' is not a valid Component method")

        self = args[0]
        if not isinstance(self, LifecycleNode):
            raise TypeError(f"'{function.__name__}' is not a valid Component method")

        # Check Component is active
        if rclpy_is_ok() and hasattr(self, "_state_machine"):
            if self._state_machine.current_state[1] in [
                "active",
                "inactive",
                "activating",
            ]:
                return function(*args, **kwargs)
            else:
                logger.error(
                    f"{self._state_machine.current_state[1]} Cannot use component fallback method '{function.__name__}' without activating or configuring the Component"
                )
                return None
        else:
            logger.error(
                f"Cannot use component action method '{function.__name__}' without initializing rclpy and the Component"
            )

    _wrapper.__name__ = function.__name__

    return _wrapper


def launch_action(function: Callable):
    """
    Decorator to add LaunchCotext to a method to be used as a ros launch action

    :param function:
    :type function: Callable
    """

    @wraps(function)
    def _wrapper(*args, **kwargs):
        """_wrapper.
        :param a:
        :param kw:
        """

        def new_function(_: LaunchContext, *args, **kwargs):
            return function(*args, **kwargs)

        return OpaqueFunction(function=new_function, args=args, kwargs=kwargs)

    function_parameters = inspect.signature(function).parameters
    new_parameters = list(function_parameters.values())
    new_parameters.insert(
        0,
        inspect.Parameter(
            "context",
            kind=inspect.Parameter.POSITIONAL_ONLY,
            annotation=LaunchContext,
        ),
    )
    _wrapper.__signature__ = inspect.signature(function).replace(
        parameters=new_parameters
    )
    return _wrapper


def log_srv(srv_callback: Callable):
    """
    Decorator for components service callback methods to log request/response of the service call

    :param srv_callback:
    :type srv_callback: Callable
    """

    @wraps(srv_callback)
    def _wrapper(*args, **kwargs):
        """_wrapper.
        :param a:
        :param kw:
        """
        self = args[0]
        parameters = inspect.signature(srv_callback).parameters
        self.get_logger().info(
            f"Got New Service Request: {parameters['request'].annotation}"
        )
        response = srv_callback(*args, **kwargs)
        self.get_logger().info(f"Service returned response: {response}")
        return response

    return _wrapper


def has_decorator(method: Callable, decorator_name: str):
    """Helper method to check if a callable is decorated with given decorator
    :param method:
    :type method: Callable
    :param decorator_name:
    :type decorator_name: str

    :rtype: bool
    """
    if decorator_name.startswith("@"):
        decorator_name = decorator_name[1:]

    decorators = [
        i.strip()
        for i in inspect.getsource(method).split("\n")
        if i.strip().startswith("@")
    ]
    return f"@{decorator_name}" in decorators


def get_methods_with_decorator(obj, decorator_name: str) -> List[str]:
    """Helper method to get all object method names decorated with given decorator
    :param obj:
    :type obj: Any
    :param decorator_name:
    :type decorator_name: str

    :rtype: List[str]
    """
    method_names = []
    for name, method in inspect.getmembers(obj.__class__, predicate=inspect.isfunction):
        if has_decorator(method, decorator_name):
            method_names.append(name)
    return method_names


def camel_to_snake_case(text: str) -> str:
    """
    Turns given string from camel case to snake case
    used to automatically assign names to ros services and actions from type names

    :param text: _description_
    :type text: str
    :return: _description_
    :rtype: str
    """
    result = ""
    for char in text:
        if char.isupper():
            result += "_" + char.lower()
        else:
            result += char
    return result.lstrip("_")
