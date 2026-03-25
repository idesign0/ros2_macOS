import pytest
from unittest.mock import MagicMock, PropertyMock

from agents.ros import Topic
from agents.clients.model_base import ModelClient
from agents.clients.db_base import DBClient
from agents.components.component_base import ComponentRunType


@pytest.fixture(scope="session")
def rclpy_init():
    """Initialize rclpy for the test session."""
    import rclpy

    rclpy.init()
    yield
    rclpy.shutdown()


@pytest.fixture
def mock_model_client():
    """Create a mock ModelClient."""
    client = MagicMock(spec=ModelClient)
    client.inference.return_value = {"output": "test"}
    client.check_connection.return_value = None
    client.initialize.return_value = None
    client.deinitialize.return_value = None
    type(client).supports_tool_calls = PropertyMock(return_value=True)
    type(client).inference_timeout = PropertyMock(return_value=30)
    return client


@pytest.fixture
def mock_db_client():
    """Create a mock DBClient."""
    client = MagicMock(spec=DBClient)
    client.query.return_value = {"output": "route_result"}
    client.check_connection.return_value = None
    client.initialize.return_value = None
    client.deinitialize.return_value = None
    client.add.return_value = None
    return client


@pytest.fixture
def string_topics():
    """Create a pair of String topics."""
    return (
        Topic(name="in", msg_type="String"),
        Topic(name="out", msg_type="String"),
    )


@pytest.fixture
def image_topic():
    """Create an Image topic."""
    return Topic(name="img", msg_type="Image")


@pytest.fixture
def audio_topic():
    """Create an Audio topic."""
    return Topic(name="audio", msg_type="Audio")


def mock_component_internals(component):
    """Patch internal attributes on an already-constructed component
    so that method-level tests can run without ROS lifecycle calls."""
    component.get_logger = MagicMock()
    component.health_status = MagicMock()
    component.health_status.set_fail_component = MagicMock()
    component.health_status.set_fail_algorithm = MagicMock()
    component.inference_params = component.config._get_inference_params()
    component.run_type = ComponentRunType.EVENT
    component.get_ros_time = MagicMock(
        return_value=MagicMock(sec=0, nanosec=0)
    )

    # Setup publishers_dict
    mock_pub = MagicMock()
    mock_pub.publish = MagicMock()
    mock_pub.output_topic = MagicMock()
    component.publishers_dict = {"out": mock_pub}

    return component
