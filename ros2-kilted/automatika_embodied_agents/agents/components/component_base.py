import json
from abc import abstractmethod
from copy import deepcopy
from types import NoneType
from typing import Optional, Sequence, Union, List, Dict, Type

from ..ros import (
    BaseComponent,
    ComponentRunType,
    FixedInput,
    SupportedType,
    Topic,
    BaseTopic,
    Event,
    Action,
)
from ..config import BaseComponentConfig
from ..utils import flatten


class Component(BaseComponent):
    """Component."""

    def __init__(
        self,
        inputs: Optional[Sequence[Union[Topic, FixedInput]]] = None,
        outputs: Optional[Sequence[Topic]] = None,
        config: Optional[BaseComponentConfig] = None,
        trigger: Union[Topic, List[Topic], float, Event, NoneType] = 1.0,
        component_name: str = "agents_component",
        **kwargs,
    ):
        self.config: BaseComponentConfig = (
            deepcopy(config) if config else BaseComponentConfig()
        )
        self.allowed_inputs: Dict[
            str, List[Union[Type[SupportedType], List[Type[SupportedType]]]]
        ]
        self.allowed_outputs: Dict[
            str, List[Union[Type[SupportedType], List[Type[SupportedType]]]]
        ]

        # setup inputs and outputs
        if inputs:
            self._validate_topics(
                inputs,
                allowed_topic_types=self.allowed_inputs,
                topics_direction="Inputs",
            )

        if outputs:
            if hasattr(self, "allowed_outputs"):
                self._validate_topics(
                    outputs,
                    allowed_topic_types=self.allowed_outputs,
                    topics_direction="Outputs",
                )

        # Initialize Parent Component
        super().__init__(
            component_name=component_name,
            inputs=inputs,
            outputs=outputs,
            config=self.config,
            callback_group=None,
            **kwargs,
        )

        # setup component run type and triggers
        self._trigger(trigger)

    def custom_on_configure(self):
        """Custom configurateion in case trigger is an event"""
        if isinstance(self.trig_source, Event):
            self.get_logger().info("ADDING TRIGGER EVENT/ACTION PAIR")
            self._add_event_action_pair(self.trig_source, Action(self._execution_step))

    def custom_on_activate(self):
        """
        Custom configuration for creating triggers.
        """
        # Setup trigger based callback or frequency based timer
        if self.run_type is ComponentRunType.EVENT:
            self.activate_all_triggers()

    def create_all_subscribers(self):
        """
        Override to handle trigger topics and fixed inputs.
        Called by parent BaseComponent
        """
        self.get_logger().info("STARTING ALL SUBSCRIBERS")
        all_callbacks = (
            list(self.callbacks.values()) + list(self.trig_callbacks.values())
            if self.run_type is ComponentRunType.EVENT
            and hasattr(
                self, "trig_callbacks"
            )  # second condition is necessary if being triggered by events
            else self.callbacks.values()
        )
        for callback in all_callbacks:
            callback.set_node_name(self.node_name)
            if not hasattr(callback.input_topic, "fixed"):
                callback.set_subscriber(self._add_ros_subscriber(callback))

    def activate_all_triggers(self) -> None:
        """
        Activates component triggers by attaching execution step to callbacks
        """
        # For topic triggers
        if hasattr(self, "trig_callbacks"):
            self.get_logger().info("ACTIVATING TRIGGER TOPICS")
            for callback in self.trig_callbacks.values():
                # Add execution step of the node as a post callback function
                callback.on_callback_execute(self._execution_step, get_processed=False)

    def destroy_all_subscribers(self) -> None:
        """
        Destroys all node subscribers
        """
        self.get_logger().info("DESTROYING ALL SUBSCRIBERS")
        all_callbacks = (
            list(self.callbacks.values()) + list(self.trig_callbacks.values())
            if self.run_type is ComponentRunType.EVENT
            else self.callbacks.values()
        )
        for callback in all_callbacks:
            if callback._subscriber:
                self.destroy_subscription(callback._subscriber)

    def _trigger(self, trigger: Union[Topic, List[Topic], float, Event, None]) -> None:
        """
        Set component trigger
        """
        if isinstance(trigger, list):
            self.run_type = ComponentRunType.EVENT
            self.trig_callbacks = {}
            for t in trigger:
                if t.name not in self.callbacks:
                    raise TypeError(
                        f"Invalid configuration for component trigger {t.name} - A trigger needs to be one of the inputs already defined in component inputs."
                    )
                self.trig_callbacks[t.name] = self.callbacks[t.name]
                # remove trigger inputs from self.callbacks
                del self.callbacks[t.name]

        elif isinstance(trigger, Topic):
            if trigger.name not in self.callbacks:
                raise TypeError(
                    f"Invalid configuration for component trigger {trigger.name} - A trigger needs to be one of the inputs already defined in component inputs."
                )
            self.run_type = ComponentRunType.EVENT
            self.trig_callbacks = {trigger.name: self.callbacks[trigger.name]}
            del self.callbacks[trigger.name]

        elif isinstance(trigger, Event):
            self.run_type = ComponentRunType.EVENT

        elif trigger is None:
            if self.run_type not in [
                ComponentRunType.ACTION_SERVER,
                ComponentRunType.SERVER,
            ]:
                raise TypeError(
                    f"Component run type is set to `{self.run_type}` but no trigger is provided. Trigger can only be set to None when component run type is `ACTION_SERVER` or `SERVER`."
                )

        else:
            self.run_type = ComponentRunType.TIMED
            # Set component loop_rate (Hz)
            self.config.loop_rate = 1 / trigger

        self.trig_source: Union[Topic, List[Topic], float, Event, None] = trigger

    def _validate_topics(
        self,
        topics: Sequence[Union[Topic, FixedInput]],
        allowed_topic_types: Optional[
            Dict[str, List[Union[Type[SupportedType], List[Type[SupportedType]]]]]
        ] = None,
        topics_direction: str = "Topics",
    ):
        """
        Verify component specific inputs or outputs using allowed topics if provided
        """
        # type validation
        correct_type = all(isinstance(i, (BaseTopic, FixedInput)) for i in topics)
        if not correct_type:
            raise TypeError(
                f"{topics_direction} to a component can only be of type Topic"
            )

        # Check that only the allowed topics (or their subtypes) have been given
        if not allowed_topic_types:
            return

        all_msg_types = {topic.msg_type for topic in topics}
        all_topic_types = list(flatten(allowed_topic_types["Required"])) + (
            list(flatten(allowed_topic_types.get("Optional")))
            if allowed_topic_types.get("Optional")
            else []
        )

        if msg_type := next(
            (
                topic
                for topic in all_msg_types
                if not any(
                    issubclass(topic, allowed_t) for allowed_t in all_topic_types
                )
            ),
            None,
        ):
            raise TypeError(
                f"{topics_direction} to the component of type {self.__class__.__name__} can only be of the allowed datatypes: {[topic.__name__ for topic in all_topic_types]} or their subclasses. A topic of type {msg_type.__name__} cannot be given to this component."
            )

        def _check_type(
            m_type: Type[SupportedType],
            allowed_type: Union[Type[SupportedType], List[Type[SupportedType]]],
        ) -> bool:
            if isinstance(allowed_type, List):
                return any(issubclass(m_type, allowed_t) for allowed_t in allowed_type)
            return issubclass(m_type, allowed_type)

        # Check that all required topics (or subtypes) have been given
        sufficient_topics = all(
            any(_check_type(m_type, allowed_type) for m_type in all_msg_types)
            for allowed_type in allowed_topic_types["Required"]
        )

        if not sufficient_topics:
            raise TypeError(
                f"{self.__class__.__name__} component {topics_direction} should have at least one topic of each datatype in the following list: {[' or '.join([t.__name__ for t in topic]) if isinstance(topic, List) else topic.__name__ for topic in allowed_topic_types['Required']]}"
            )

    @abstractmethod
    def _execution_step(self, **kwargs):
        """_execution_step.

        :param args:
        :param kwargs:
        """
        raise NotImplementedError(
            "_execution_step method needs to be implemented by child components."
        )

    def _update_cmd_args_list(self):
        """
        Update launch command arguments
        """
        super()._update_cmd_args_list()

        self.launch_cmd_args = [
            "--trigger",
            self._get_trigger_json(),
        ]

    def _get_trigger_json(self) -> Union[str, bytes, bytearray]:
        """
        Serialize component routes to json

        :return: Serialized inputs
        :rtype:  str | bytes | bytearray
        """
        serialized_trigger = {}
        if isinstance(self.trig_source, Topic):
            serialized_trigger["trigger_type"] = "Topic"
            serialized_trigger["trigger"] = self.trig_source.to_json()
        elif isinstance(self.trig_source, List):
            serialized_trigger["trigger_type"] = "List"
            serialized_trigger["trigger"] = json.dumps([
                t.to_json() for t in self.trig_source
            ])
        elif isinstance(self.trig_source, Event):
            serialized_trigger["trigger_type"] = "Event"
            serialized_trigger["trigger"] = self.trig_source.to_json()
        else:
            serialized_trigger = self.trig_source
        return json.dumps(serialized_trigger)

    def _replace_input_topic(
        self, topic_name: str, new_name: str, msg_type: str
    ) -> Optional[str]:
        """Replaces a component input topic by a new topic. Overrides parent method to handle trigger callbacks

        :param topic_name: Old Topic name
        :type topic_name: str
        :param new_name: New topic name
        :type new_name: str
        :param msg_type: New topic message type
        :type msg_type: str
        :return: Error message or None if no errors are found
        :rtype: Optional[str]
        """
        error_msg = super()._replace_input_topic(topic_name, new_name, msg_type)
        if not error_msg:
            return

        # topic to be replaced is not found in callbacks -> check in trigger callbacks
        normalized_topic_name = (
            topic_name[1:] if topic_name.startswith("/") else topic_name
        )

        if topic_name not in self.trig_callbacks:
            error_msg = f"Topic {topic_name} is not found in Component inputs"
            return error_msg

        old_callback = self.trig_callbacks[normalized_topic_name]

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
        self.trig_callbacks.pop(normalized_topic_name)
        self.trig_callbacks[new_name] = new_callback

        # update the internal lists
        old_topic = old_callback.input_topic
        self._update_inactive_input_topic(old_topic, new_topic)

        return None
