"""Tests for ModelComponent._call_inference dispatch — requires rclpy."""

import pytest
from unittest.mock import MagicMock

from agents.config import LLMConfig
from agents.ros import Topic
from agents.components.llm import LLM
from tests.conftest import mock_component_internals


@pytest.fixture
def llm_with_client(rclpy_init, mock_model_client):
    """Create an LLM with a mock model client."""
    comp = LLM(
        inputs=[Topic(name="in", msg_type="String")],
        outputs=[Topic(name="out", msg_type="String")],
        model_client=mock_model_client,
        config=LLMConfig(),
        component_name="test_llm",
    )
    mock_component_internals(comp)
    return comp


@pytest.fixture
def llm_with_local(rclpy_init):
    """Create an LLM configured for local model, with a mock local_model."""
    comp = LLM(
        inputs=[Topic(name="in", msg_type="String")],
        outputs=[Topic(name="out", msg_type="String")],
        config=LLMConfig(enable_local_model=True),
        component_name="test_local_llm",
    )
    mock_component_internals(comp)
    comp.local_model = MagicMock(return_value={"output": "local_response"})
    return comp


class TestCallInferenceHTTPClient:
    def test_calls_model_client(self, llm_with_client, mock_model_client):
        result = llm_with_client._call_inference({"query": "test"})
        mock_model_client.inference.assert_called_once_with({"query": "test"})
        assert result == {"output": "test"}

    def test_returns_none_sets_fail(self, llm_with_client, mock_model_client):
        mock_model_client.inference.return_value = None
        llm_with_client._call_inference({"query": "test"})
        llm_with_client.health_status.set_fail_algorithm.assert_called_once()


class TestCallInferenceLocalModel:
    def test_calls_local_model(self, llm_with_local):
        result = llm_with_local._call_inference({"query": "test"})
        llm_with_local.local_model.assert_called_once()
        assert result["output"] == "local_response"

    def test_returns_none_sets_fail(self, llm_with_local):
        llm_with_local.local_model.return_value = None
        llm_with_local._call_inference({"query": "test"})
        llm_with_local.health_status.set_fail_algorithm.assert_called_once()


class TestCallInferenceNoBackend:
    def test_no_client_no_local_sets_fail(self, rclpy_init, mock_model_client):
        comp = LLM(
            inputs=[Topic(name="in", msg_type="String")],
            outputs=[Topic(name="out", msg_type="String")],
            model_client=mock_model_client,
            config=LLMConfig(),
            component_name="test_llm_none",
        )
        mock_component_internals(comp)
        # Remove both backends
        comp.model_client = None
        comp.local_model = None
        comp._call_inference({"query": "test"})
        comp.health_status.set_fail_component.assert_called_once()
