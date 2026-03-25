"""Tests for LocalLLM wrapper — no ROS needed."""

import sys
import json
import pytest
from unittest.mock import MagicMock, patch


@pytest.fixture(autouse=True)
def mock_llama_cpp():
    """Mock llama_cpp before importing LocalLLM."""
    mock_module = MagicMock()
    with patch.dict(sys.modules, {"llama_cpp": mock_module}):
        yield mock_module


@pytest.fixture
def local_llm(mock_llama_cpp):
    from agents.utils.local_llm import LocalLLM

    llm = LocalLLM.__new__(LocalLLM)
    llm.llm = MagicMock()
    llm.device = "cpu"
    llm.ncpu = 1
    return llm


def _mock_response(content="Hello there!", tool_calls=None):
    """Build a mock non-streaming chat completion response."""
    message = {"role": "assistant", "content": content}
    finish_reason = "stop"
    if tool_calls:
        message["tool_calls"] = tool_calls
        finish_reason = "tool_calls"
    return {
        "choices": [
            {
                "index": 0,
                "message": message,
                "finish_reason": finish_reason,
            }
        ],
        "usage": {"prompt_tokens": 10, "completion_tokens": 5, "total_tokens": 15},
    }


def _mock_stream_chunks(tokens):
    """Build mock streaming chunks from a list of token strings."""
    chunks = []
    # First chunk: role announcement
    chunks.append({"choices": [{"delta": {"role": "assistant"}, "finish_reason": None}]})
    # Content chunks
    for token in tokens:
        chunks.append({"choices": [{"delta": {"content": token}, "finish_reason": None}]})
    # Final chunk
    chunks.append({"choices": [{"delta": {}, "finish_reason": "stop"}]})
    return chunks


class TestCallNonStreaming:
    def test_returns_output(self, local_llm):
        local_llm.llm.create_chat_completion.return_value = _mock_response(
            "Hello there!"
        )

        result = local_llm({"query": [{"role": "user", "content": "Hi"}]})
        assert result["output"] == "Hello there!"
        local_llm.llm.create_chat_completion.assert_called_once()

    def test_passes_temperature_and_max_tokens(self, local_llm):
        local_llm.llm.create_chat_completion.return_value = _mock_response("ok")

        local_llm({
            "query": [{"role": "user", "content": "Hi"}],
            "temperature": 0.5,
            "max_new_tokens": 100,
        })

        call_kwargs = local_llm.llm.create_chat_completion.call_args[1]
        assert call_kwargs["temperature"] == 0.5
        assert call_kwargs["max_tokens"] == 100


class TestCallStreaming:
    def test_returns_generator(self, local_llm):
        chunks = _mock_stream_chunks(["Hello", " world"])
        local_llm.llm.create_chat_completion.return_value = iter(chunks)

        result = local_llm(
            {"query": [{"role": "user", "content": "Hi"}]}, stream=True
        )
        assert "output" in result
        tokens = list(result["output"])
        assert tokens == ["Hello", " world"]


class TestCallWithTools:
    def test_tools_parsed_from_response(self, local_llm):
        tool_calls = [
            {
                "id": "call_0",
                "type": "function",
                "function": {
                    "name": "route_to_nav",
                    "arguments": json.dumps({"x": 1}),
                },
            }
        ]
        local_llm.llm.create_chat_completion.return_value = _mock_response(
            content=None, tool_calls=tool_calls
        )

        tools = [{"type": "function", "function": {"name": "route_to_nav"}}]
        result = local_llm(
            {"query": [{"role": "user", "content": "Go"}], "tools": tools}
        )
        assert "tool_calls" in result
        assert result["tool_calls"][0]["function"]["name"] == "route_to_nav"
        assert result["tool_calls"][0]["function"]["arguments"] == {"x": 1}

    def test_passes_tools_to_api(self, local_llm):
        local_llm.llm.create_chat_completion.return_value = _mock_response("ok")

        tools = [{"type": "function", "function": {"name": "test_fn"}}]
        local_llm(
            {"query": [{"role": "user", "content": "Hi"}], "tools": tools}
        )

        call_kwargs = local_llm.llm.create_chat_completion.call_args[1]
        assert call_kwargs["tools"] == tools
        assert call_kwargs["tool_choice"] == "auto"
