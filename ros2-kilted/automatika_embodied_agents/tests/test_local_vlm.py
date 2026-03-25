"""Tests for LocalVLM wrapper — no ROS needed."""

import sys
import pytest
import numpy as np
from unittest.mock import MagicMock, patch


@pytest.fixture
def mock_deps():
    """Mock llama_cpp and its chat format module before importing LocalVLM."""
    mock_llama = MagicMock()
    mock_chat_format = MagicMock()
    mock_llama.llama_chat_format = mock_chat_format
    with patch.dict(
        sys.modules,
        {
            "llama_cpp": mock_llama,
            "llama_cpp.llama_chat_format": mock_chat_format,
        },
    ):
        yield mock_llama, mock_chat_format


@pytest.fixture
def local_vlm(mock_deps):
    mock_llama, mock_chat_format = mock_deps
    from agents.utils.local_vlm import LocalVLM

    vlm = LocalVLM.__new__(LocalVLM)
    vlm.llm = MagicMock()
    vlm.device = "cpu"
    vlm.ncpu = 1
    return vlm


def _mock_vlm_response(content="A cat"):
    return {
        "choices": [
            {
                "index": 0,
                "message": {"role": "assistant", "content": content},
                "finish_reason": "stop",
            }
        ],
    }


class TestLocalVLMCall:
    def test_with_numpy_image(self, local_vlm):
        img = np.zeros((480, 640, 3), dtype=np.uint8)
        local_vlm.llm.create_chat_completion.return_value = _mock_vlm_response("A cat")

        result = local_vlm({
            "query": [{"role": "user", "content": "What is this?"}],
            "images": [img],
        })
        assert result["output"] == "A cat"
        local_vlm.llm.create_chat_completion.assert_called_once()

        # Verify the message has multimodal format
        call_kwargs = local_vlm.llm.create_chat_completion.call_args[1]
        messages = call_kwargs["messages"]
        assert len(messages) == 1
        content = messages[0]["content"]
        assert any(item["type"] == "image_url" for item in content)
        assert any(item["type"] == "text" for item in content)

    def test_no_images(self, local_vlm):
        result = local_vlm({
            "query": [{"role": "user", "content": "What is this?"}],
            "images": [],
        })
        assert result["output"] == "No image provided."

    def test_extracts_last_user_query(self, local_vlm):
        img = np.zeros((100, 100, 3), dtype=np.uint8)
        local_vlm.llm.create_chat_completion.return_value = _mock_vlm_response("ok")

        local_vlm({
            "query": [
                {"role": "user", "content": "First question"},
                {"role": "assistant", "content": "Answer"},
                {"role": "user", "content": "Second question"},
            ],
            "images": [img],
        })

        call_kwargs = local_vlm.llm.create_chat_completion.call_args[1]
        messages = call_kwargs["messages"]
        # The text part should be the last user message
        text_items = [
            item for item in messages[0]["content"] if item["type"] == "text"
        ]
        assert text_items[0]["text"] == "Second question"

    def test_image_data_uri_format(self, local_vlm):
        img = np.zeros((10, 10, 3), dtype=np.uint8)
        local_vlm.llm.create_chat_completion.return_value = _mock_vlm_response("ok")

        local_vlm({
            "query": [{"role": "user", "content": "test"}],
            "images": [img],
        })

        call_kwargs = local_vlm.llm.create_chat_completion.call_args[1]
        content = call_kwargs["messages"][0]["content"]
        image_item = next(item for item in content if item["type"] == "image_url")
        url = image_item["image_url"]["url"]
        assert url.startswith("data:image/png;base64,")
