"""Fallbacks"""

from typing import List, Optional, Union, Dict, Any
import json
from attrs import define, field

from automatika_ros_sugar.msg import ComponentStatus

from .action import Action
from .event import EventBlackboardEntry
from ..io import Topic


@define
class Fallback:
    """Fallback action and execution tracking"""

    action: Union[List[Action], Action] = field()
    max_retries: Optional[int] = field(default=None)

    # Internal values to keep track of retry attempts and the unique action index within a set of actions
    action_idx: int = field(default=0, init=False)
    retry_idx: int = field(default=0, init=False)

    def reset_current_idx(self):
        """Reset the current action index to zero"""
        self.action_idx = 0

    def reset_retries(self):
        """Reset the current retries index to zero"""
        self.retry_idx = 0

    def reset(self):
        """Reset the current action and the retries index to zero"""
        self.reset_current_idx()
        self.reset_retries()

    @property
    def dictionary(self) -> Dict:
        """Getter of fallback as a dictionary for serialization

        :return: Fallback dict
        :rtype: Dict
        """
        action_list = (
            [self.action] if not isinstance(self.action, list) else self.action
        )

        return {
            "action_list": [action.dictionary for action in action_list],
            "max_retries": self.max_retries,
        }

    def _get_required_topics(self) -> List[Topic]:
        actions = self.action if isinstance(self.action, list) else [self.action]
        topics = []
        for action in actions:
            topics.extend(action.get_required_topics())
        return topics


class ComponentFallbacks:
    """
    Components Fallbacks contain the set of Actions to be executed when a failure status is detected (See Status class). Fallbacks are defines with an Action or a list of Actions to be executed in order. Each Action in the list is retried until max_retries is reached.

    # Default behavior in a Component:
    The default behavior in a Component is for the component to broadcast the status on any detected failure. By default the component sets 'on_any_fail' Fallback to 'Action(self.broadcast_status)' with 'max_retries=None'.

    Action(s) set for 'on_any_fail' is executed for *any* failure for which no fallback action is defined.

    # Usage in a Component:
    ```python
        from ros_sugar.component import BaseComponent
        from ros_sugar.action import Action

        my_component = BaseComponent(node_name='test_component')

        # Set fallback for component failure to restart the component
        my_component.on_component_fail(fallback=Action(my_component.restart))

        # Change fallback for any failure
        my_component.on_fail(fallback=Action(my_component.restart))

        # First broadcast status, if another failure happens -> restart
        my_component.on_fail(fallback=[Action(my_component.broadcast_status), Action(my_component.restart)])
    ```
    """

    def __init__(
        self,
        on_any_fail: Optional[Fallback] = None,
        on_component_fail: Optional[Fallback] = None,
        on_algorithm_fail: Optional[Fallback] = None,
        on_system_fail: Optional[Fallback] = None,
        on_giveup: Optional[Fallback] = None,
    ) -> None:
        """
        Setup component fallbacks

        :param max_retries: Maximum number of attempts when running a fallback action, defaults to 1
        :type max_retries: int, optional
        :param on_any_fail: Actions to be executed on any failure if a specific failure policy is not defined, defaults to None
        :type on_any_fail: Optional[Union[List[Action], Action]], optional
        :param on_component_fail:  Actions to be executed on component level fail, defaults to None
        :type on_component_fail: Optional[Union[List[Action], Action]], optional
        :param on_algorithm_fail: Actions to be executed on algorithm level fail, defaults to None
        :type on_algorithm_fail: Optional[Union[List[Action], Action]], optional
        :param on_system_fail: Actions to be executed on system level fail, defaults to None
        :type on_system_fail: Optional[Union[List[Action], Action]], optional
        :param on_giveup: Action to be executed when all fallbacks have failed, defaults to None
        :type on_giveup: Optional[Action], optional
        """
        self.on_any_fail = on_any_fail

        self.on_component_fail = on_component_fail
        self.on_algorithm_fail = on_algorithm_fail

        self.on_system_fail = on_system_fail

        self.on_giveup = on_giveup

        # Flag to indicate that all fallbacks failed and no more fallbacks are available
        self.__giveup: bool = False
        self.__latest_state_value = ComponentStatus.STATUS_HEALTHY

        # Registry of the latest topics message values
        self.__topics_blackboard: Dict[str, Any] = {}

    @property
    def giveup(self) -> bool:
        """
        Getter of component giveup, True if no more fallbacks are available for a type of failure

        :return: Giveup
        :rtype: bool
        """
        return self.__giveup

    @property
    def latest_status(self) -> int:
        """Get the latest health status updated by the fallback execution

        :return: Health status code
        :rtype: int
        """
        return self.__latest_state_value

    @property
    def json(self) -> Union[str, bytes, bytearray]:
        """Getter of serialized component fallbacks for component multi-process execution serialization/deserialization

        :return: Serialized ComponentFallbacks
        :rtype: Union[str, bytes, bytearray]
        """
        fallbacks_dict = {
            "on_any_fail": self.on_any_fail.dictionary if self.on_any_fail else None,
            "on_component_fail": self.on_component_fail.dictionary
            if self.on_component_fail
            else None,
            "on_algorithm_fail": self.on_algorithm_fail.dictionary
            if self.on_algorithm_fail
            else None,
            "on_system_fail": self.on_system_fail.dictionary
            if self.on_system_fail
            else None,
            "on_giveup": self.on_giveup.dictionary if self.on_giveup else None,
        }
        return json.dumps(fallbacks_dict)

    @property
    def required_topics(self) -> List[Topic]:
        on_any = self.on_any_fail._get_required_topics() if self.on_any_fail else []
        on_component = (
            self.on_component_fail._get_required_topics()
            if self.on_component_fail
            else []
        )
        on_algorithm = (
            self.on_algorithm_fail._get_required_topics()
            if self.on_algorithm_fail
            else []
        )
        on_system = (
            self.on_system_fail._get_required_topics() if self.on_system_fail else []
        )
        on_giveup = self.on_giveup._get_required_topics() if self.on_giveup else []
        all_topics = on_any + on_algorithm + on_component + on_system + on_giveup
        # Get unique topics
        unique_topics_dict = {}
        for topic in all_topics:
            unique_topics_dict[topic.name] = topic
        return list(unique_topics_dict.values())

    def update_topics_blackboard(self, topics_board: Dict[str, EventBlackboardEntry]):
        self.__topics_blackboard = {
            key: value.msg for key, value in topics_board.items()
        }

    def reset(self) -> None:
        """Reset all fallback execution tracking indices to 0 and the retries tracking indices to 0"""
        self.reset_execution_indices()
        self.reset_retries()

    def reset_execution_indices(self) -> None:
        """Reset all fallback execution tracking indices to 0"""
        # Indices to keep track of the executed fallback

        if self.on_component_fail:
            self.on_component_fail.reset_current_idx()
        if self.on_algorithm_fail:
            self.on_algorithm_fail.reset_current_idx()
        if self.on_system_fail:
            self.on_system_fail.reset_current_idx()

    def reset_retries(self):
        """Reset all fallback retries tracking indices to 0"""
        if self.on_component_fail:
            self.on_component_fail.reset_retries()
        if self.on_algorithm_fail:
            self.on_algorithm_fail.reset_retries()
        if self.on_system_fail:
            self.on_system_fail.reset_retries()

    def _execute_fallback(self, fallback: Fallback) -> None:
        """
        Execute a fallback from given fallbacks methods

        :param idx: Index of last executed fallback from the list, sat to '-1' if no fallback was ever executed
        :type idx: int
        :param fallback_list: List of fallback methods
        :type fallback_list: List[Callable]

        :raises ValueError: If the list of fallback methods is None
        """
        if not isinstance(fallback.action, List):
            # Only one fallback action is available

            # None max_retries == Never give up, or max_retries not reached yet
            if (
                fallback.max_retries is None
                or fallback.retry_idx < fallback.max_retries
            ):
                try:
                    success = fallback.action(topics=self.__topics_blackboard)
                except Exception:
                    success = False
                if success:
                    # Fallback ran successfully -> reset the status to healthy
                    self.__latest_state_value = ComponentStatus.STATUS_HEALTHY
                fallback.retry_idx += 1
                self.__giveup = False
            else:
                self.__giveup = True
            return

        # Fallback with a list of actions cannot have None max_retries as it will remain stuck on first action
        if fallback.max_retries is None:
            fallback.max_retries = 1

        if fallback.retry_idx < fallback.max_retries:
            # If all retries are not consumed increase retry index
            fallback.retry_idx += 1
        else:
            # If all retries are consumed -> reset retries and execute the next fallback
            fallback.reset_retries()
            fallback.action_idx += 1

        if fallback.action_idx < len(fallback.action):
            try:
                success = fallback.action[fallback.action_idx](
                    topics=self.__topics_blackboard
                )
            except Exception:
                success = False
            if success:
                # Fallback ran successfully -> reset the status to healthy
                self.__latest_state_value = ComponentStatus.STATUS_HEALTHY
            self.__giveup = False
        else:
            self.__giveup = True

    def execute_giveup(self):
        """
        Execute the component giveup method when all fallbacks fail
        """
        self._execute_fallback(self.on_giveup)

    def execute_component_fallback(self) -> bool:
        """
        Execute the next component fallback method for component failure

        :return: Giveup: If no more fallbacks are available to be executed
        :rtype: bool
        """
        self.__latest_state_value = ComponentStatus.STATUS_FAILURE_COMPONENT_LEVEL
        self._execute_fallback(self.on_component_fail)
        return self.__giveup

    def execute_algorithm_fallback(self) -> bool:
        """
        Execute the next algorithm fallback method for algorithm failure

        :return: Giveup: If no more fallbacks are available to be executed
        :rtype: bool
        """
        self.__latest_state_value = ComponentStatus.STATUS_FAILURE_ALGORITHM_LEVEL
        self._execute_fallback(self.on_algorithm_fail)
        return self.__giveup

    def execute_system_fallback(self) -> bool:
        """
        Execute the next system fallback method for system failure

        :return: Giveup: If no more fallbacks are available to be executed
        :rtype: bool
        """
        self.__latest_state_value = ComponentStatus.STATUS_FAILURE_SYSTEM_LEVEL
        self._execute_fallback(self.on_system_fail)
        return self.__giveup

    def execute_generic_fallback(self) -> bool:
        """
        Execute the next algorithm fallback method for any failure

        :return: Giveup: If no more fallbacks are available to be executed
        :rtype: bool
        """
        self.__latest_state_value = ComponentStatus.STATUS_GENERAL_FAILURE
        self._execute_fallback(self.on_any_fail)
        return self.__giveup
