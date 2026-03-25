import unittest
from functools import partial
from threading import Event as threadingEvent
import launch_testing
import launch_testing.actions
import launch_testing.markers
import pytest
import numpy as np

from ros_sugar.io import Topic
from ros_sugar.core import BaseComponent, Event
from ros_sugar import Launcher
from ros_sugar.actions import Action, LogInfo

# Threading Events
on_any_and_on_equal_py_event = threadingEvent()
on_true_or_on_contains_py_event = threadingEvent()
on_not_false_py_event = threadingEvent()


class ChildComponent(BaseComponent):
    """Child component to publish an array of data for testing"""

    def __init__(
        self,
        component_name,
        inputs=None,
        outputs=None,
        change_data: bool = False,
        **kwargs,
    ):
        super().__init__(
            component_name,
            inputs,
            outputs,
            **kwargs,
        )
        self._data = np.array([1.0, 2.0, 3.0, 4.0])
        self._change_data = change_data
        self._counter = 0.0

    def _execution_step(self):
        self._counter += 1
        if self._change_data:
            if self._counter % 2 == 0:
                self._data = np.array([30.0, 40.0])
            else:
                self._data = np.array([1.0, 2.0, 3.0, 4.0])
        # Publish data
        if self.publishers_dict.get("float_array"):
            self.publishers_dict["float_array"].publish(self._data)
        if self.publishers_dict.get("bool_topic"):
            self.publishers_dict["bool_topic"].publish(True)


@pytest.mark.launch_test
@launch_testing.markers.keep_alive
def generate_test_description():
    """
    Generate launch test description to test all event classes using the component health status topic
    Health status is published automatically after start and should be equal to 'STATUS_HEALTHY'
    """

    # health status topic
    status_topic = Topic(name="publisher_component/status", msg_type="ComponentStatus")

    # float array topic
    float_array_topic = Topic(name="float_array", msg_type="Float64MultiArray")
    bool_topic = Topic(name="bool_topic", msg_type="Bool")

    publisher_component = ChildComponent(
        component_name="publisher_component",
        outputs=[float_array_topic, bool_topic],
        change_data=True,
    )

    event_on_any_and_on_equal = Event(
        status_topic & (float_array_topic.msg.data == [1.0, 2.0, 3.0, 4.0]),
    )

    event_on_true_or_on_contains = Event(
        bool_topic.msg.data.is_true()
        | float_array_topic.msg.data.contains_all([
            2.0,
            3.0,
        ]),
    )

    event_on_not_false = Event(~bool_topic.msg.data.is_false())

    def trigger_event(on_event: Event, **_):
        on_event.set()

    launcher = Launcher()

    launcher.add_pkg(
        components=[publisher_component],
        events_actions={
            event_on_any_and_on_equal: [
                LogInfo(msg="Got event_on_any_and_on_equal"),
                Action(method=partial(trigger_event, on_any_and_on_equal_py_event)),
            ],
            event_on_true_or_on_contains: [
                LogInfo(msg="Got event_on_true_or_on_contains"),
                Action(method=partial(trigger_event, on_true_or_on_contains_py_event)),
            ],
            event_on_not_false: [
                LogInfo(msg="Got event_on_not_false"),
                Action(method=partial(trigger_event, on_not_false_py_event)),
            ],
        },
    )

    # Setup launch description without bringup for testing
    launcher.setup_launch_description()

    # Add ready for test action
    launcher._description.add_action(launch_testing.actions.ReadyToTest())

    # Return the launcher description for launch_testing
    return launcher._description


class TestEvents(unittest.TestCase):
    """Tests that all event types are raised and caught correctly"""

    wait_time = 10.0  # seconds

    def test_on_any_and_on_equal(cls):
        global on_any_and_on_equal_py_event
        assert on_any_and_on_equal_py_event.wait(
            cls.wait_time
        ), "Failed to raise on_any_and_on_equal event"

    def test_on_true_or_on_contains(cls):
        global on_true_or_on_contains_py_event
        assert on_true_or_on_contains_py_event.wait(
            cls.wait_time
        ), "Failed to raise on_true_or_on_contains event"

    def test_on_not_false(cls):
        global on_not_false_py_event
        assert on_not_false_py_event.wait(
            cls.wait_time
        ), "Failed to raise on_not_false event"
