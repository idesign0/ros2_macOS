"""Component Status"""

from typing import List, Optional

from automatika_ros_sugar.msg import ComponentStatus

_component_status = {
    0: "Running - Healthy",
    1: "Failure: Algorithm Level",
    2: "Failure: Component Level",
    3: "Failure: System Level",
    4: "Failure: General",
}


class Status:
    """
    Component health status
    Updated by the component during runtime to declare any detected failure in the component, or in the component algorithm, or in the external system

    Status is configured in the base component along with a corresponding publisher if the health broadcasting is enabled.

    To use health status in your component (inheriting from BaseComponent):

    - To indicate a malfunction in general without details on the failure level / source, use:
        ```python
        self.health_status.set_failure()
        ```

    - It can indicate a mal-function/ failure in the algorithm (algorithm) used by the component. In this case it is possible to indicate the failed algorithm name in the message.
        ```python
        self.health_status.set_fail_algorithm(algorithm_names=['algorithm_1', 'algorithm_2'])
        ```

    - It can indicate a mal-function/ failure in the component itself or another component. Component fail by default refers to a failure in the component itself, however it is possible to specify a failure detected in a specific component by passing the name(s)
        ```python
        # To indicate failure in the component itself
        self.health_status.set_fail_component()

        # To indicate failure in another component
        self.health_status.set_fail_component(component_names=['other_component_name'])

        # To indicate failure in multiple components
        self.health_status.set_fail_component(component_names=[self.node_name, 'other_component_name'])
        ```

    - It can indicate a mal-function/ failure in the external system. It is possible to add the failure source using component names or topic names (in case a required topic is not available, for example)
        ```python
        # For failure related to a specific topic
        self.health_status.set_fail_system(topic_names=['some_topic_name'])
        ```

    NOTE: TIMED components publish the status periodically (if broadcasting is enabled). If the component is not TIMED, then the child component should implement the publishing, using:
    ```python
        self.health_status_publisher.publish(self.health_status)
    ```

    """

    def __init__(self, msg: Optional[ComponentStatus] = None) -> None:
        if msg:
            self._msg = msg
        else:
            # Init with healthy status
            self._msg = ComponentStatus()
            self.set_healthy()

    def __call__(self) -> ComponentStatus:
        """
        Returns the ROS message for publishing

        :return: ROS message
        :rtype: ComponentStatus
        """
        return self._msg

    @property
    def value(self):
        """
        Status health value

        :return: Current status value
        :rtype: ComponentStatus
        """
        return self._msg.status

    @value.setter
    def value(self, key: int):
        """
        Set status to given key value

        :param value: Status key value from [ComponentStatus.STATUS_HEALTHY, ComponentStatus.STATUS_FAILURE_ALGORITHM_LEVEL, ComponentStatus.STATUS_FAILURE_COMPONENT_LEVEL, ComponentStatus.STATUS_FAILURE_SYSTEM_LEVEL]
        :type value: int

        :raises ValueError: If given key value if not supported
        :raises TypeError: If given key value if not of type int
        """
        if type(key) is int:
            if key in _component_status.keys():
                self._set_status_from_dict(key=key)
            else:
                raise ValueError(
                    f"Unsupported status value. Status can only be set to one of the following keys: {_component_status}"
                )
        else:
            raise TypeError(
                f"Can only set using integer values in the following: {_component_status}"
            )

    def _set_status_from_dict(self, key: int):
        """
        Set StatusMsg from given key

        :param key: Status key value
        :type key: int
        """
        self._msg.status = key
        self._msg.msg = _component_status[key]

    def set_healthy(self):
        """
        Set status to Running - Healthy
        """
        self._set_status_from_dict(key=0)

    def set_failure(self):
        """
        Set status to Non Healthy - General (Any) Failure
        """
        self._set_status_from_dict(key=4)

    def set_fail_algorithm(self, algorithm_names: Optional[List[str]] = None):
        """
        Set status to Error: Algorithmic Level
        """
        self._set_status_from_dict(key=1)
        if algorithm_names:
            self._msg.src_algorithms = algorithm_names

    def set_fail_component(self, component_names: Optional[List[str]] = None):
        """
        Set status to Error: Component Level
        """
        self._set_status_from_dict(key=2)
        if component_names:
            self._msg.src_components = component_names

    def set_fail_system(
        self,
        component_names: Optional[List[str]] = None,
        topic_names: Optional[List[str]] = None,
    ):
        """
        Set status to Error: System Level
        """
        self._set_status_from_dict(key=3)
        if component_names:
            self._msg.src_components = component_names
        if topic_names:
            self._msg.src_topics = topic_names

    @property
    def is_healthy(self) -> bool:
        """
        Property to check if the current health status is healthy

        :return: If status is ComponentStatus.STATUS_HEALTHY
        :rtype: bool
        """
        return self.value == ComponentStatus.STATUS_HEALTHY

    @property
    def is_component_fail(self) -> bool:
        """
        Property to check if there is a component fail status

        :return: If status is ComponentStatus.STATUS_FAILURE_COMPONENT_LEVEL
        :rtype: bool
        """
        return self.value == ComponentStatus.STATUS_FAILURE_COMPONENT_LEVEL

    @property
    def is_algorithm_fail(self) -> bool:
        """
        Property to check if there is a algorithm fail status

        :return: If status is ComponentStatus.STATUS_FAILURE_ALGORITHM_LEVEL
        :rtype: bool
        """
        return self.value == ComponentStatus.STATUS_FAILURE_ALGORITHM_LEVEL

    @property
    def is_system_fail(self) -> bool:
        """
        Property to check if there is a system "external" fail status

        :return: If status is ComponentStatus.STATUS_FAILURE_SYSTEM_LEVEL
        :rtype: bool
        """
        return self.value == ComponentStatus.STATUS_FAILURE_SYSTEM_LEVEL

    @property
    def is_general_fail(self) -> bool:
        return self.value == ComponentStatus.STATUS_GENERAL_FAILURE
