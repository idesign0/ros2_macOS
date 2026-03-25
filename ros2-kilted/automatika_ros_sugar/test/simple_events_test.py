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
from automatika_ros_sugar.msg import ComponentStatus
from ros_sugar.actions import Action, LogInfo

# Threading Events
on_any_py_event = threadingEvent()
on_equal_py_event = threadingEvent()
on_diff_py_event = threadingEvent()
on_greater_py_event = threadingEvent()
on_less_py_event = threadingEvent()
on_contains_any_py_event = threadingEvent()
on_contains_all_py_event = threadingEvent()
on_change_py_event = threadingEvent()
on_change_eq_py_event = threadingEvent()


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

    publisher_component = ChildComponent(
        component_name="publisher_component",
        outputs=[float_array_topic],
        change_data=True,
    )

    event_on_any = Event(
        status_topic,
    )

    event_on_equal = Event(
        status_topic.msg.status == ComponentStatus.STATUS_HEALTHY,
    )

    event_on_diff = Event(
        status_topic.msg.status != ComponentStatus.STATUS_FAILURE_ALGORITHM_LEVEL,
    )

    event_on_greater = Event(
        status_topic.msg.status > -1,
    )

    event_on_less = Event(
        status_topic.msg.status < ComponentStatus.STATUS_FAILURE_COMPONENT_LEVEL,
    )

    event_on_contains_any = Event(
        float_array_topic.msg.data.contains_any([1.0, 10.0]),
    )

    event_on_contains_all = Event(
        float_array_topic.msg.data.contains_all([1.0, 2.0, 3.0]),
    )

    event_on_change = Event(
        float_array_topic,
        on_change=True,
    )

    event_on_change_eq = Event(
        float_array_topic.msg.data == [30.0, 40.0],
        on_change=True,
    )

    def trigger_event(on_event: Event, **_):
        on_event.set()

    launcher = Launcher()

    launcher.add_pkg(
        components=[publisher_component],
        events_actions={
            event_on_any: [
                LogInfo(msg="Got OnAny Event"),
                Action(method=partial(trigger_event, on_any_py_event)),
            ],
            event_on_equal: [
                LogInfo(msg="Got OnEqual Event"),
                Action(method=partial(trigger_event, on_equal_py_event)),
            ],
            event_on_diff: [
                LogInfo(msg="Got OnDifferent Event"),
                Action(method=partial(trigger_event, on_diff_py_event)),
            ],
            event_on_less: [
                LogInfo(msg="Got OnLess Event"),
                Action(method=partial(trigger_event, on_less_py_event)),
            ],
            event_on_greater: [
                LogInfo(msg="Got OnGreater Event"),
                Action(method=partial(trigger_event, on_greater_py_event)),
            ],
            event_on_contains_any: [
                LogInfo(msg="Got OnContainsAny Event"),
                Action(method=partial(trigger_event, on_contains_any_py_event)),
            ],
            event_on_contains_all: [
                LogInfo(msg="Got OnContainsAll Event"),
                Action(method=partial(trigger_event, on_contains_all_py_event)),
            ],
            event_on_change: [
                LogInfo(msg="Got OnChange Event"),
                Action(method=partial(trigger_event, on_change_py_event)),
            ],
            event_on_change_eq: [
                LogInfo(msg="Got OnChangeEqual Event"),
                Action(method=partial(trigger_event, on_change_eq_py_event)),
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

    def test_on_any(cls):
        global on_any_py_event
        assert on_any_py_event.wait(cls.wait_time), "Failed to raise OnAny event"

    def test_on_equal(cls):
        global on_equal_py_event
        assert on_equal_py_event.wait(cls.wait_time), "Failed to raise OnEqual event"

    def test_on_different(cls):
        global on_diff_py_event
        assert on_diff_py_event.wait(cls.wait_time), "Failed to raise OnDifferent event"

    def test_on_less(cls):
        global on_less_py_event
        assert on_less_py_event.wait(cls.wait_time), "Failed to raise OnLess event"

    def test_on_greater(cls):
        global on_greater_py_event
        assert on_greater_py_event.wait(
            cls.wait_time
        ), "Failed to raise OnGreater event"

    def test_contains_with_publisher_node(cls):
        global on_contains_any_py_event, on_contains_all_py_event
        assert on_contains_any_py_event.wait(
            cls.wait_time
        ), "Failed to raise OnContainsAny event"
        assert on_contains_all_py_event.wait(
            cls.wait_time
        ), "Failed to raise OnContainsAll event"

    def test_change_with_publisher_node(cls):
        global on_change_py_event, on_change_eq_py_event
        assert on_change_py_event.wait(cls.wait_time), "Failed to raise OnChange event"
        assert on_change_eq_py_event.wait(
            cls.wait_time
        ), "Failed to raise OnChangeEqual event"
