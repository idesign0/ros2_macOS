"""Tests for LLM component — requires rclpy."""

import pytest
from unittest.mock import MagicMock

from agents.config import LLMConfig
from agents.ros import Topic
from agents.components.llm import LLM
from tests.conftest import mock_component_internals


@pytest.fixture
def llm(rclpy_init, mock_model_client):
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


class TestLLMConstruction:
    def test_with_model_client(self, rclpy_init, mock_model_client):
        comp = LLM(
            inputs=[Topic(name="in", msg_type="String")],
            outputs=[Topic(name="out", msg_type="String")],
            model_client=mock_model_client,
            config=LLMConfig(),
            component_name="test_llm_client",
        )
        assert comp.model_client is mock_model_client

    def test_with_local_model(self, rclpy_init):
        comp = LLM(
            inputs=[Topic(name="in", msg_type="String")],
            outputs=[Topic(name="out", msg_type="String")],
            config=LLMConfig(enable_local_model=True),
            component_name="test_llm_local",
        )
        assert comp.config.enable_local_model is True

    def test_no_client_no_local_raises(self, rclpy_init):
        with pytest.raises(RuntimeError):
            LLM(
                inputs=[Topic(name="in", msg_type="String")],
                outputs=[Topic(name="out", msg_type="String")],
                config=LLMConfig(),
                component_name="test_llm_fail",
            )

    def test_system_prompt_in_messages(self, rclpy_init, mock_model_client):
        comp = LLM(
            inputs=[Topic(name="in", msg_type="String")],
            outputs=[Topic(name="out", msg_type="String")],
            model_client=mock_model_client,
            config=LLMConfig(_system_prompt="You are a robot"),
            component_name="test_llm_sp",
        )
        assert comp.messages[0] == {"role": "system", "content": "You are a robot"}

    def test_no_system_prompt_empty_messages(self, rclpy_init, mock_model_client):
        comp = LLM(
            inputs=[Topic(name="in", msg_type="String")],
            outputs=[Topic(name="out", msg_type="String")],
            model_client=mock_model_client,
            config=LLMConfig(),
            component_name="test_llm_nosp",
        )
        assert comp.messages == []


class TestLLMCreateInput:
    def test_from_trigger_topic(self, llm):
        trigger = Topic(name="in", msg_type="String")
        mock_cb = MagicMock()
        mock_cb.get_output.return_value = "hello"
        llm.trig_callbacks = {"in": mock_cb}
        llm.callbacks = {}

        result = llm._create_input(topic=trigger)
        assert result is not None
        assert result["query"][-1]["content"] == "hello"

    def test_returns_none_no_query(self, llm):
        trigger = Topic(name="in", msg_type="String")
        mock_cb = MagicMock()
        mock_cb.get_output.return_value = None
        llm.trig_callbacks = {"in": mock_cb}
        llm.callbacks = {}

        result = llm._create_input(topic=trigger)
        assert result is None

    def test_chat_reset(self, llm):
        llm.config.chat_history = True
        llm.messages = [{"role": "user", "content": "old"}]
        trigger = Topic(name="in", msg_type="String")
        mock_cb = MagicMock()
        mock_cb.get_output.return_value = "chat reset"
        llm.trig_callbacks = {"in": mock_cb}
        llm.callbacks = {}

        result = llm._create_input(topic=trigger)
        assert result is None
        assert llm.messages == []

    def test_includes_tools(self, llm):
        llm.config._tool_descriptions = [{"function": {"name": "test_tool"}}]
        trigger = Topic(name="in", msg_type="String")
        mock_cb = MagicMock()
        mock_cb.get_output.return_value = "hello"
        llm.trig_callbacks = {"in": mock_cb}
        llm.callbacks = {}

        result = llm._create_input(topic=trigger)
        assert "tools" in result


class TestLLMExecutionStep:
    def test_non_streaming(self, llm, mock_model_client):
        trigger = Topic(name="in", msg_type="String")
        mock_cb = MagicMock()
        mock_cb.get_output.return_value = "hello"
        llm.trig_callbacks = {"in": mock_cb}
        llm.callbacks = {}

        llm._execution_step(topic=trigger)
        mock_model_client.inference.assert_called_once()
        llm.publishers_dict["out"].publish.assert_called_once()

    def test_no_input_skips_inference(self, llm, mock_model_client):
        trigger = Topic(name="in", msg_type="String")
        mock_cb = MagicMock()
        mock_cb.get_output.return_value = None
        llm.trig_callbacks = {"in": mock_cb}
        llm.callbacks = {}

        llm._execution_step(topic=trigger)
        mock_model_client.inference.assert_not_called()

    def test_with_tool_calls(self, llm, mock_model_client):
        llm.config._tool_descriptions = [{"function": {"name": "test_tool"}}]
        mock_model_client.inference.return_value = {
            "output": "calling tool",
            "tool_calls": [
                {"function": {"name": "test_tool", "arguments": {"x": "1"}}}
            ],
        }

        # Register a mock tool
        mock_tool = MagicMock(return_value="tool_result")
        llm._external_processors = {
            "test_tool": ([mock_tool], "tool"),
        }
        llm.config._tool_response_flags = {"test_tool": False}

        trigger = Topic(name="in", msg_type="String")
        mock_cb = MagicMock()
        mock_cb.get_output.return_value = "do something"
        llm.trig_callbacks = {"in": mock_cb}
        llm.callbacks = {}

        llm._execution_step(topic=trigger)
        mock_tool.assert_called_once()


class TestLLMWarmup:
    def test_with_model_client(self, llm, mock_model_client):
        llm._warmup()
        assert mock_model_client.inference.call_count == 2

    def test_with_local_model(self, rclpy_init):
        comp = LLM(
            inputs=[Topic(name="in", msg_type="String")],
            outputs=[Topic(name="out", msg_type="String")],
            config=LLMConfig(enable_local_model=True),
            component_name="test_warmup_local",
        )
        mock_component_internals(comp)
        comp.model_client = None
        comp.local_model = MagicMock(return_value={"output": "ok"})
        comp._warmup()
        assert comp.local_model.call_count == 2
