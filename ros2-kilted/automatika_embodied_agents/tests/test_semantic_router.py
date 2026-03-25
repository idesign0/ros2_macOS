"""Tests for SemanticRouter component — requires rclpy."""

import pytest
from unittest.mock import MagicMock, PropertyMock

from agents.config import SemanticRouterConfig, LLMConfig
from agents.ros import Topic, Route
from agents.components.semantic_router import SemanticRouter, RouterMode
from agents.clients.model_base import ModelClient


@pytest.fixture
def routes():
    """Create sample routes."""
    return [
        Route(
            routes_to=Topic(name="nav", msg_type="String"),
            samples=["go to", "navigate to", "move to"],
        ),
        Route(
            routes_to=Topic(name="chat", msg_type="String"),
            samples=["hello", "how are you", "tell me a joke"],
        ),
    ]


class TestRouterConstruction:
    def test_vector_mode(self, rclpy_init, mock_db_client, routes):
        router = SemanticRouter(
            inputs=[Topic(name="in", msg_type="String")],
            routes=routes,
            db_client=mock_db_client,
            config=SemanticRouterConfig(router_name="test_router"),
            component_name="test_vector_router",
        )
        assert router.routing_mode == RouterMode.VECTOR

    def test_llm_mode_with_client(self, rclpy_init, routes):
        client = MagicMock(spec=ModelClient)
        type(client).supports_tool_calls = PropertyMock(return_value=True)
        type(client).inference_timeout = PropertyMock(return_value=30)
        client.inference.return_value = {"output": "test"}
        client.check_connection.return_value = None
        client.initialize.return_value = None
        client.deinitialize.return_value = None

        router = SemanticRouter(
            inputs=[Topic(name="in", msg_type="String")],
            routes=routes,
            model_client=client,
            component_name="test_llm_router",
        )
        assert router.routing_mode == RouterMode.LLM

    def test_llm_mode_with_local(self, rclpy_init, routes):
        router = SemanticRouter(
            inputs=[Topic(name="in", msg_type="String")],
            routes=routes,
            config=LLMConfig(enable_local_model=True),
            component_name="test_local_router",
        )
        assert router.routing_mode == RouterMode.LLM

    def test_no_client_no_db_no_local_raises(self, rclpy_init, routes):
        with pytest.raises(ValueError):
            SemanticRouter(
                inputs=[Topic(name="in", msg_type="String")],
                routes=routes,
                component_name="test_fail_router",
            )

    def test_no_tool_support_raises(self, rclpy_init, routes):
        client = MagicMock(spec=ModelClient)
        type(client).supports_tool_calls = PropertyMock(return_value=False)

        with pytest.raises(TypeError):
            SemanticRouter(
                inputs=[Topic(name="in", msg_type="String")],
                routes=routes,
                model_client=client,
                component_name="test_notool_router",
            )
