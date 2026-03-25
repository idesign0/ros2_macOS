# Testing Guide

This document covers how to test Sugarcoat components and systems, including unit testing with `pytest`, integration testing with `launch_testing`, and running the full test suite.

## Prerequisites

Ensure your workspace is built and sourced:

```bash
cd ~/ros_sugar_ws
colcon build
source install/setup.bash
```

Install test dependencies:

```bash
pip install pytest pytest-cov launch_testing_ros
```

## Running the Test Suite

### Full suite with colcon

Run all tests across the package:

```bash
colcon test --packages-select automatika_ros_sugar
colcon test-result --verbose
```

### Individual tests with pytest

For faster iteration during development, run pytest directly:

```bash
cd ~/ros_sugar_ws/src/sugarcoat
python -m pytest tests/ -v
```

## Unit Testing Components

### Testing a BaseComponent subclass

Create a test that instantiates your component, configures it, and verifies behavior:

```python
import pytest
import rclpy
from rclpy.lifecycle import TransitionCallbackReturn

from my_package.my_component import MyComponent
from ros_sugar.io import Topic
from ros_sugar.io.supported_types import Float32


@pytest.fixture(scope="module")
def rclpy_init():
    rclpy.init()
    yield
    rclpy.shutdown()


@pytest.fixture
def component(rclpy_init):
    input_topic = Topic(name="/sensor", msg_type=Float32)
    comp = MyComponent(
        component_name="test_component",
        inputs=[input_topic],
    )
    yield comp
    comp.destroy_node()


def test_configure(component):
    result = component.trigger_configure()
    assert result == TransitionCallbackReturn.SUCCESS


def test_activate(component):
    component.trigger_configure()
    result = component.trigger_activate()
    assert result == TransitionCallbackReturn.SUCCESS
```

### Testing configuration

Verify that `BaseComponentConfig` attributes are validated correctly:

```python
from my_package.config import MyComponentConfig

def test_config_defaults():
    config = MyComponentConfig()
    assert config.loop_rate > 0

def test_config_from_file(tmp_path):
    config_file = tmp_path / "config.yaml"
    config_file.write_text("loop_rate: 50.0\n")
    config = MyComponentConfig.from_yaml(str(config_file))
    assert config.loop_rate == 50.0
```

## Testing Events and Actions

### Testing event conditions

Verify that conditions evaluate correctly against mock message data:

```python
from ros_sugar.core import Event
from ros_sugar.io import Topic
from ros_sugar.io.supported_types import Float32
from ros_sugar.core.event import EventBlackboardEntry
import time


def test_event_triggers():
    sensor = Topic(name="/battery", msg_type=Float32)
    event = Event(sensor.msg.data < 10.0)

    # Create a mock ROS message
    from std_msgs.msg import Float32 as ROSFloat32
    msg = ROSFloat32()
    msg.data = 5.0

    # Simulate the blackboard
    blackboard = {
        "/battery": EventBlackboardEntry(msg=msg, timestamp=time.time())
    }

    event.check_condition(blackboard)
    assert event.trigger is True


def test_event_does_not_trigger():
    sensor = Topic(name="/battery", msg_type=Float32)
    event = Event(sensor.msg.data < 10.0)

    from std_msgs.msg import Float32 as ROSFloat32
    msg = ROSFloat32()
    msg.data = 95.0

    blackboard = {
        "/battery": EventBlackboardEntry(msg=msg, timestamp=time.time())
    }

    event.check_condition(blackboard)
    assert event.trigger is False
```

### Testing actions

Test that actions execute correctly when called:

```python
from ros_sugar.core import Action


def test_action_execution():
    result = {}

    def my_handler(topics=None):
        result["called"] = True
        return True

    action = Action(my_handler)
    action(topics={})
    assert result["called"] is True
```

### Testing fallbacks

Verify the fallback execution chain:

```python
from ros_sugar.core import Action, ComponentFallbacks, Fallback


def test_fallback_retry():
    call_count = {"n": 0}

    def failing_action(topics=None):
        call_count["n"] += 1
        return False  # Simulate failure

    fallbacks = ComponentFallbacks(
        on_component_fail=Fallback(
            action=Action(failing_action),
            max_retries=3,
        )
    )

    for _ in range(3):
        giveup = fallbacks.execute_component_fallback()
        assert giveup is False

    # Fourth attempt should trigger giveup
    giveup = fallbacks.execute_component_fallback()
    assert giveup is True
```

## Integration Testing with launch_testing

For tests that require a running ROS graph, use `launch_testing`:

```python
import unittest
import launch
import launch_testing
import launch_testing.actions
from launch_ros.actions import LifecycleNode


def generate_test_description():
    node = LifecycleNode(
        package="my_package",
        executable="my_component_node",
        name="test_node",
        output="screen",
    )

    return launch.LaunchDescription([
        node,
        launch_testing.actions.ReadyToTest(),
    ]), {"node": node}


class TestComponentIntegration(unittest.TestCase):
    def test_node_starts(self, proc_info):
        """Verify the node process starts without crashing."""
        proc_info.assertWaitForStartup(timeout=10)


@launch_testing.post_shutdown_test()
class TestShutdown(unittest.TestCase):
    def test_clean_exit(self, proc_info):
        launch_testing.asserts.assertExitCodes(proc_info)
```

Place integration tests in a `test/` directory and ensure they are registered in `CMakeLists.txt` or `setup.cfg` so that `colcon test` discovers them.

## Testing the Event Serialization Round-Trip

Events must survive JSON serialization for multi-process execution. Test this explicitly:

```python
from ros_sugar.core import Event
from ros_sugar.io import Topic
from ros_sugar.io.supported_types import Float32


def test_event_serialization():
    sensor = Topic(name="/temp", msg_type=Float32)
    original = Event(sensor.msg.data > 100.0)

    json_str = original.to_json()
    restored = Event.from_json(json_str)

    assert restored._condition.topic_name == "/temp"
    assert restored._condition.ref_value == 100.0
```

## Tips

- Always call `rclpy.init()` before creating nodes and `rclpy.shutdown()` after destroying them. Use `pytest` fixtures with appropriate scope to manage this.
- For tests that only exercise pure Python logic (conditions, serialization, configs), you do not need a running ROS system.
- Use `colcon test --event-handlers console_direct+` for real-time test output.
- To run a single test file: `colcon test --packages-select automatika_ros_sugar --pytest-args tests/test_events.py`.
