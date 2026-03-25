import unittest
from threading import Event as threadingEvent
import launch_testing
import launch_testing.actions
import launch_testing.markers
import pytest

from ros_sugar.core import BaseComponent, Event
from ros_sugar import Launcher
from ros_sugar.config import ComponentRunType
from ros_sugar import actions
from ros_sugar.io import Topic

# Dummy service type for testing
from nav_msgs.srv import SetMap

# Threading Events
execution_service_py_event = threadingEvent()


class ChildComponent(BaseComponent):
    """Child component to test component action"""

    def __init__(
        self,
        component_name,
        inputs=None,
        outputs=None,
        config=None,
        config_file=None,
        callback_group=None,
        fallbacks=None,
        main_action_type=None,
        main_srv_type=None,
        **kwargs,
    ):
        super().__init__(
            component_name,
            inputs,
            outputs,
            config,
            config_file,
            callback_group,
            fallbacks,
            main_action_type,
            main_srv_type,
            **kwargs,
        )
        self.service_type = SetMap

    def main_service_callback(self, _, response):
        global execution_service_py_event
        execution_service_py_event.set()
        return response


@pytest.mark.launch_test
@launch_testing.markers.keep_alive
def generate_test_description():
    # Component publishing to the event topic
    component = ChildComponent(component_name="test_component")

    component.loop_rate = 10.0  # Hz
    component.run_type = ComponentRunType.SERVER

    # health status topic
    status_topic = Topic(name="test_component/status", msg_type="ComponentStatus")

    # Dummy event to send an automatic service call to the component main service post launch
    event_on_health_status = Event(status_topic, handle_once=True)
    srv_call = actions.send_srv_request(
        srv_name="test_component/set_map",
        srv_request_msg=SetMap.Request(),
        srv_type=SetMap,
    )

    launcher = Launcher()

    launcher.add_pkg(
        components=[component],
        events_actions={event_on_health_status: srv_call},
        multiprocessing=False,
        ros_log_level="debug",
        rclpy_log_level="debug",
    )

    # Setup launch description without bringup for testing
    launcher.setup_launch_description()

    # Add ready for test action
    launcher._description.add_action(launch_testing.actions.ReadyToTest())

    # Return the launcher description for launch_testing
    return launcher._description


class TestActions(unittest.TestCase):
    """Tests that Component runtype SERVER works correctly"""

    wait_time = 10.0  # seconds

    def test_server_component(cls):
        global execution_service_py_event
        assert execution_service_py_event.wait(
            cls.wait_time
        ), "Server component did not run correctly"
