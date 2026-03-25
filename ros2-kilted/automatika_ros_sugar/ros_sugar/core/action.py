"""Actions"""

import inspect
import json
from rclpy.lifecycle import Node as LifecycleNode
from launch.actions import (
    OpaqueCoroutine as ROSOpaqueCoroutine,
    OpaqueFunction as ROSOpaqueFunction,
)
from functools import wraps
from typing import (
    Any,
    Callable,
    Dict,
    Optional,
    Union,
    List,
    Type,
    Tuple,
    Iterable,
    Text,
    Awaitable,
)

from launch import LaunchContext
import launch
from launch.actions import LogInfo as LogInfoROSAction

from ..launch import logger
from ..condition import MsgConditionBuilder
from ..io import Topic, get_msg_type
from ..io.supported_types import SupportedType
from ..utils import InvalidAction


def _create_auto_topic_parser(input_msg_type: Type, target_type: Type) -> Optional[Callable]:
    """
    Factory function to create an automatic parser from a ROS message type
    to a target type (either another ROS message or a Python primitive/structure).

    :param input_msg_type: The source ROS message class.
    :param target_type: The destination type (ROS msg class, int, float, list, etc.).
    :return: A callable parser function: (msg=...) -> target_type
    """
    if not hasattr(input_msg_type, "get_fields_and_field_types"):
        raise ValueError(
            f"Cannot create an automatic parser. 'input_msg_type' should be a valid ROS2 message type, but got {input_msg_type}"
        )
    # Check if target is a ROS Message (has field types method)
    return _create_auto_ros_msg_parser(input_msg_type, target_type)


def _get_ros_field_type_map(msg_cls: Type) -> Dict[str, str]:
    """Helper to safely get field types map from a ROS message class."""
    if hasattr(msg_cls, "get_fields_and_field_types"):
        return msg_cls.get_fields_and_field_types()
    return {}


def _create_auto_ros_msg_parser(
    input_msg_type: Type, target_msg_type: Type
) -> Optional[Callable]:
    """
    Creates a parser to map one ROS message type to another ROS message type.
    Logic:
    1. Exact Match.
    2. Duck Typing (Same field names & types).
    3. Type Matching (Different names, same types - heuristic).
    """
    msg_dummy = input_msg_type()

    # Case 1: Direct Type Match
    if isinstance(msg_dummy, target_msg_type):
        return lambda *, msg, **_: msg

    target_fields = _get_ros_field_type_map(target_msg_type)
    input_fields = _get_ros_field_type_map(input_msg_type)

    # Case 2: Duck Typing (Name & Type Match)
    # Check if all fields in target exist in input with same type string
    common_fields = []
    for key, f_type in target_fields.items():
        if key in input_fields and input_fields[key] == f_type:
            common_fields.append(key)

    # If we found matches for ALL target fields, this is a strong match subset
    # Or if we found matches for ALL input fields
    # If target has fields, we need to fill them strictly.
    # If target is fully covered by input:
    if (len(common_fields) == len(target_fields) and len(target_fields) > 0) or (
        len(common_fields) == len(input_fields)
        and len(input_fields) < len(target_fields)
    ):

        def duck_parser(msg: Any, **_) -> Any:
            out = target_msg_type()
            for key in common_fields:
                setattr(out, key, getattr(msg, key))
            return out

        return duck_parser

    # Case 3: Type-based matching (Heuristic)
    # If names don't match, but types do uniquely
    # e.g. Input(x: float), Target(data: float)
    matching_map = {}  # target_field -> input_field
    used_inputs = set()

    for inp_key, inp_type in input_fields.items():
        found_source = None
        for tgt_key, tgt_type in target_fields.items():
            if inp_key in used_inputs:
                continue
            if inp_type == tgt_type:
                found_source = tgt_key
                break

        if found_source:
            matching_map[inp_key] = found_source
            used_inputs.add(inp_key)

    if (len(matching_map) == len(target_fields) and len(target_fields) > 0) or (
        len(matching_map) == len(input_fields)
        and len(input_fields) < len(target_fields)
    ):
        logger.warning(
            f"Added type-based parser (heuristic) from '{input_msg_type.__name__}' to '{target_msg_type.__name__}' "
            f"mapping: {matching_map}. Verify this logic."
        )

        def type_parser(msg: Any, **_) -> Any:
            out = target_msg_type()
            for i_key, t_key in matching_map.items():
                setattr(out, t_key, getattr(msg, i_key))
            return out

        return type_parser

    # Case 4: No Match
    return None


class OpaqueFunction(ROSOpaqueFunction):
    """
    Action that executes a Python function.

    The signature of the function should be:

    .. code-block:: python

        def function(
            context: LaunchContext,
            *args,
            **kwargs
        ) -> Optional[List[LaunchDescriptionEntity]]:
            ...

    """

    def __init__(
        self,
        *,
        function: Callable,
        args: Optional[Iterable[Any]] = None,
        kwargs: Optional[Dict[Text, Any]] = None,
        **left_over_kwargs,
    ) -> None:
        self.function = function
        self.args = args
        self.kwargs = kwargs
        super().__init__(
            function=function,
            args=args,
            kwargs=kwargs,
            **left_over_kwargs,
        )


class OpaqueCoroutine(ROSOpaqueCoroutine):
    """
    Action that adds a Python coroutine function to the launch run loop.

    The signature of the coroutine function should be:

    .. code-block:: python

        async def coroutine_func(
            context: LaunchContext,
            *args,
            **kwargs
        ):
            ...

    if ignore_context is False on construction (currently the default), or

    .. code-block:: python

        async def coroutine_func(
            *args,
            **kwargs
        ):
            ...

    if ignore_context is True on construction.
    """

    def __init__(
        self,
        *,
        coroutine: Callable[..., Awaitable[None]],
        args: Optional[Iterable[Any]] = None,
        kwargs: Optional[Dict[Text, Any]] = None,
        ignore_context: bool = False,
        **left_over_kwargs,
    ) -> None:
        self.coroutine = coroutine
        self.args = args
        self.kwargs = kwargs
        super().__init__(
            coroutine=coroutine,
            args=args,
            kwargs=kwargs,
            ignore_context=ignore_context,
            **left_over_kwargs,
        )


class Action:
    """
    Actions are used by Components and by the Launcher to execute specific methods.

    Actions can either be:
    - Actions paired with Events: in this case the Action is executed when an event is detected. This can be done by a Component if the action is a component method or a Launcher when the action is a system level action or an arbitrary method in the recipe
    - Actions paired with Fallbacks: in this case the Action is executed by a Component when a failure is detected

    Actions are defined with:
    - method (Callable)
    - args: Arguments to be passed to the method when executing the action
    - kwargs: Keyword arguments to be passed to the method when executing the action

    ## Usage Example:
    ```python
        from ros_sugar.component import BaseComponent
        from ros_sugar.config import BaseComponentConfig

        def function():
            print("I am executing action!")

        my_component = BaseComponent(node_name='test_component')
        new_config = BaseComponentConfig(loop_rate=50.0)
        action1 = Action(method=my_component.start)
        action2 = Action(method=my_component.reconfigure, args=(new_config, True)),)
        action3 = Action(method=function)
    ```
    """

    def __init__(
        self,
        method: Callable,
        args: Optional[Union[Tuple, List, Any]] = None,
        kwargs: Optional[Dict] = None,
    ) -> None:
        """
        Action

        :param method: Action function
        :type method: callable
        :param args: function arguments, defaults to ()
        :type args: tuple, optional
        :param kwargs: function keyword arguments, defaults to {}
        :type kwargs: dict, optional
        """
        self.__parent_component: Optional[str] = None
        self.__action_keyname: Optional[str] = (
            None  # contains the name of the component action as a string
        )
        self._function = method
        self._is_monitor_action = False
        self._is_lifecycle_action = False

        # List of registered conversions to execute before the main method
        # keeps track of mapping (argument_name -> output_type)
        self.__event_topic_conversions: Dict[str, str] = {}
        self.__prepared_events_conversions: Dict[str, Callable] = {}

        self.__verify_args_kwargs(args, kwargs)

        # Check if it is a component action and update parent and keyname
        if hasattr(self._function, "__self__"):
            action_object = self._function.__self__

            if hasattr(action_object, "node_name") and isinstance(
                action_object, LifecycleNode
            ):
                self.parent_component = action_object.node_name
                self.action_name = self._function.__name__

    def __verify_args_kwargs(self, args, kwargs):
        """
        Verify that args and kwargs are correct for the action executable.
        If MsgConditionBuilder objects are found (expressions like topic.msg.data),
        automatically create event parsers for them.
        """
        _args = []
        self._kwargs = {}

        function_parameters = inspect.signature(self.executable).parameters
        # Dict of: arg_index or kwarg name -> input topic message builder
        self.__input_topics: Dict[Union[str, int], MsgConditionBuilder] = {}

        # 1. Check & Parse Positional Args
        if args:
            args = args if isinstance(args, (Tuple, List)) else (args,)
            if len(args) > len(function_parameters):
                raise InvalidAction(
                    f"Too many arguments provided for action '{self.action_name}': Method expected maximum {len(function_parameters)} arguments, but got {len(args)}"
                )

            for idx, value in enumerate(args):
                if isinstance(value, MsgConditionBuilder):
                    self.__input_topics[f"arg_{idx}"] = value
                else:
                    _args.append(value)

        self._args = tuple(_args)
        # 2. Check & Parse Keyword Args
        if kwargs:
            for key, value in kwargs.items():
                # Make sure this keyword argument exists in the function
                if isinstance(value, MsgConditionBuilder):
                    self.__input_topics[f"kwarg_{key}"] = value
                else:
                    self._kwargs[key] = value

    def __call__(self, **kwargs):
        """
        Execute the action.
        Iterates through all parsers to prepare dynamic arguments based on the event (kwargs).
        """
        # This now supports topic inputs to be the same as the event topic
        # Create mutable copies of args and kwargs for this specific execution
        call_args = list(self._args)
        call_kwargs = self._kwargs.copy()

        # If the action is executed by an 'Event' the event will pass the triggering message
        topics = kwargs.get("topics", None)
        if topics and self.__input_topics:
            # Collect the required inputs
            for key, topic_condition in self.__input_topics.items():
                if related_message := topics.get(topic_condition.name, None):
                    output = topic_condition.get_value(object_value=related_message)
                else:
                    # Related message is not sent -> skip
                    continue
                if key.startswith("arg_"):
                    idx = int(key.removeprefix("arg_"))
                    call_args.insert(idx, output)
                elif key.startswith("kwarg_"):
                    call_kwargs[key.removeprefix("kwarg_")] = output

        for key, (name, conv_func) in self.__prepared_events_conversions.items():
            if msg := topics.get(name, None):
                call_kwargs[key] = conv_func(msg)
        try:
            return self.executable(*call_args, **call_kwargs)
        except Exception as e:
            logger.error(f"Error executing action: {e}")

    def _setup_conversions(self, event_topic_name, event_msg_type):
        """Method will be invoked from the associated event to setup any required automatic converters

        :param event_msg: _description_
        :type event_msg: _type_
        """
        self.__prepared_events_conversions = {}
        for key, msg_type in self.__event_topic_conversions.items():
            func: Optional[Callable] = _create_auto_topic_parser(
                event_msg_type, msg_type
            )
            # If failed to find an automatic conversion -> Do not register
            if func is not None:
                self.__prepared_events_conversions[key] = (event_topic_name, func)
            else:
                logger.warning(
                    f"Failed to find automatic conversion from event topic '{event_msg_type}' and the required Action argument type {msg_type}"
                )

    def set_required_runtime_arguments(
        self, kwargs: Dict[str, Union[str, SupportedType, Type]]
    ):
        """Used to set the missing (dynamic) argument type in derived pre-built actions"""
        for key, arg in kwargs.items():
            self.__event_topic_conversions[key] = get_msg_type(arg)

    def get_required_topics(self) -> List[Topic]:
        """Get all the required input topic to execute this action

        :return: Required topics
        :rtype: List[Topic]
        """
        return [
            msg_condition_builder.topic
            for msg_condition_builder in self.__input_topics.values()
        ]

    def replace_input_topic(self, old_topic: Topic, new_topic: Topic):
        """Replaces an input topic with a new topic if the old topic exists in the required input topics

        :param old_topic: Old topic
        :type old_topic: Topic
        :param new_topic: New topic
        :type new_topic: Topic
        """
        target_key = None
        for key, msg_condition_builder in self.__input_topics.items():
            if (
                old_topic.name == msg_condition_builder.topic.name
                and old_topic.msg_type == msg_condition_builder.topic.msg_type
            ):
                target_key = key
                break
        if target_key:
            self.__input_topics[target_key].topic = new_topic

    @property
    def executable(self):
        """
        Get the action callable

        :return: _description_
        :rtype: _type_
        """
        return self._function

    @executable.setter
    def executable(self, value: Callable):
        """
        Getter of action executable

        :param value: _description_
        :type value: Callable
        """
        self._function = value

    def _reset_args_kwargs(self, args, kwargs, input_topics):
        """
        Reset the arguments

        :return: _description_
        :rtype: _type_
        """
        self.__verify_args_kwargs(args, kwargs)
        self.__input_topics = input_topics

    @property
    def parent_component(self):
        """
        Getter of parent component class name if it is a component action, else None

        :return: _description_
        :rtype: str | None
        """
        return self.__parent_component

    @parent_component.setter
    def parent_component(self, component_name: str):
        """
        Setter of parent component name

        :param component_name: _description_
        :type component_name: str
        """
        self.__parent_component = component_name

    @property
    def action_name(self) -> str:
        """
        Getter of the action name
        Equals exact executable name if it is not a component action
        Equals method name in the component if it is a component action

        :return: _description_
        :rtype: str
        """
        if self.__action_keyname:
            return self.__action_keyname
        if hasattr(self._function, "__name__"):
            return self._function.__name__
        elif hasattr(self._function, "func") and hasattr(
            self._function.func, "__name__"
        ):
            # For partial methods
            return self._function.func.__name__
        return ""

    @action_name.setter
    def action_name(self, value: str) -> None:
        """
        Getter of action name

        :param value: _description_
        :type value: str
        """
        self.__action_keyname = value

    @property
    def component_action(self) -> bool:
        """component_action.

        :rtype: bool
        """
        return self.__parent_component is not None

    @property
    def dictionary(self) -> Dict:
        """
        Property to get/set the event using a dictionary

        :return: Event description dictionary
        :rtype: Dict
        """
        dict_value = {
            "action_name": self.action_name,
            "parent_name": self.parent_component,
            "args": self._args,
            "kwargs": self._kwargs,
            "input_topics": {
                key: value.to_json() for key, value in self.__input_topics.items()
            },
        }
        if self.__event_topic_conversions:
            dict_value["event_conversions"] = {
                key: value.__class__.__name__
                for key, value in self.__event_topic_conversions
            }
        return dict_value

    @classmethod
    def deserialize_action(
        cls,
        serialized_action_dict: Dict,
        deserialized_method: Callable,
    ) -> "Action":
        """Reconstruct Action from serialized action data

        :param serialized_action: Serialized action data
        :type serialized_action: Union[str, bytearray, bytes]
        :param deserialized_method:Deserialized action method
        :type deserialized_method: Callable
        :return: Reconstructed Action Object
        :rtype: Action
        """
        # Reconstruct the Action: executable and names
        reconstructed_action = Action(method=deserialized_method)
        reconstructed_action.action_name = serialized_action_dict["action_name"]
        reconstructed_action.parent_component = serialized_action_dict["parent_name"]

        # Deserialize the required input topics
        input_topics: Dict[Union[str, int], MsgConditionBuilder] = {}
        serialized_input_topics_dict = serialized_action_dict["input_topics"]
        for key, value in serialized_input_topics_dict.items():
            deserialized_condition_builder = json.loads(value)
            deserialized_topic = json.loads(deserialized_condition_builder["topic"])
            input_topics[key] = MsgConditionBuilder(
                topic=Topic(**deserialized_topic),
                path=deserialized_condition_builder["path"],
            )

        # Set the args/kwargs/input_topics
        reconstructed_action._reset_args_kwargs(
            serialized_action_dict["args"],
            serialized_action_dict["kwargs"],
            input_topics,
        )

        if serialized_action_dict.get("event_conversions", None):
            reconstructed_action.set_required_runtime_arguments(
                serialized_action_dict["event_conversions"]
            )
        return reconstructed_action

    @property
    def json(self) -> str:
        """
        Property to get/set the event using a json

        :return: Event description dictionary as json
        :rtype: str
        """
        json_dict = self.dictionary
        return json.dumps(json_dict)

    def launch_action(
        self, monitor_node=None
    ) -> Union[OpaqueCoroutine, OpaqueFunction]:
        """
        Get the ros launch action

        :return: _description_
        :rtype: OpaqueCoroutine | OpaqueFunction
        """
        # Check if it is a stack action and update the executable from the monitor node
        if self._is_monitor_action and monitor_node:
            if not hasattr(monitor_node, self.action_name):
                raise ValueError(f"Unknown stack action: {self.action_name}")
            # Get executable from monitor
            self.executable = getattr(monitor_node, self.action_name)

        elif self._is_monitor_action and not monitor_node:
            raise ValueError("Monitor node should be provided to parse stack action")

        function_parameters = inspect.signature(self.executable).parameters

        # Wrap the function to add LaunchContext attribute to it required by ROS Launch
        @wraps(self.executable)
        def new_function(_: LaunchContext, *args, **kwargs):
            """
            Create new_function - Add context + No return from original function

            :param context: ROS Launch Context
            :type context: LaunchContext
            """
            self(*args, **kwargs)

        # HACK: Update function signature - as ROS Launch uses inspect to check signature
        new_parameters = list(function_parameters.values())
        new_parameters.insert(
            0,
            inspect.Parameter(
                "context",
                kind=inspect.Parameter.POSITIONAL_ONLY,
                annotation=LaunchContext,
            ),
        )
        new_function.__signature__ = inspect.signature(self.executable).replace(
            parameters=new_parameters
        )

        if inspect.iscoroutine(self.executable):
            return OpaqueCoroutine(
                coroutine=new_function, args=self._args, kwargs=self._kwargs
            )
        else:
            return OpaqueFunction(
                function=new_function, args=self._args, kwargs=self._kwargs
            )


class LogInfo(LogInfoROSAction):
    """Overrides the LogInfo Action for ros2 launch to change the hard-codded logger name

    :param LogInfoROSAction: Action that logs a message when executed
    :type LogInfoROSAction: LogInfoROSAction
    """

    def __init__(self, *, msg: str, logger_name: Optional[str] = None, **kwargs):
        """Setup the LogInfo Action

        :param msg: Logged message
        :type msg: str
        :param logger_name: Logger name, defaults to None. If not provided the message is logged with the package logger name 'Launcher'
        :type logger_name: Optional[str], optional
        """
        super().__init__(msg=msg, **kwargs)
        if logger_name:
            self.__logger = launch.logging.get_logger(logger_name)
        else:
            self.__logger = logger

    def execute(self, context: LaunchContext) -> None:
        """Execute the action."""
        self.__logger.info(
            "".join([context.perform_substitution(sub) for sub in self.msg])
        )
        return None
