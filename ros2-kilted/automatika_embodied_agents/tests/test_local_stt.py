"""Tests for LocalSTT wrapper — no ROS needed."""

import sys
import pytest
import numpy as np
from unittest.mock import MagicMock, patch


@pytest.fixture
def mock_sherpa():
    """Mock sherpa_onnx before importing LocalSTT."""
    mock = MagicMock()
    with patch.dict(sys.modules, {"sherpa_onnx": mock}):
        yield mock


@pytest.fixture
def local_stt(mock_sherpa):
    from agents.utils.local_stt import LocalSTT

    stt = LocalSTT.__new__(LocalSTT)
    stt._recognizer = MagicMock()
    stt._sample_rate = 16000
    stt.device = "cpu"
    stt.ncpu = 1
    return stt


def _setup_mock_stream(local_stt, text):
    """Configure the mock recognizer to return a stream with the given text."""
    mock_stream = MagicMock()
    mock_stream.result.text = text
    local_stt._recognizer.create_stream.return_value = mock_stream
    return mock_stream


class TestLocalSTTCall:
    def test_with_bytes(self, local_stt):
        # Create int16 audio bytes
        audio = np.array([0, 100, -100, 32767], dtype=np.int16)
        audio_bytes = audio.tobytes()
        mock_stream = _setup_mock_stream(local_stt, "hello world")

        result = local_stt({"query": audio_bytes})
        assert result["output"] == "hello world"
        mock_stream.accept_waveform.assert_called_once()
        local_stt._recognizer.decode_stream.assert_called_once_with(mock_stream)

    def test_with_numpy(self, local_stt):
        audio = np.array([0.1, 0.2, -0.1], dtype=np.float32)
        _setup_mock_stream(local_stt, "test")

        result = local_stt({"query": audio})
        assert result["output"] == "test"

    def test_multidimensional_flattened(self, local_stt):
        audio = np.array([[0.1, 0.2], [0.3, 0.4]], dtype=np.float32)
        mock_stream = _setup_mock_stream(local_stt, "flat")

        result = local_stt({"query": audio})
        assert result["output"] == "flat"
        # Verify the array was flattened
        call_args = mock_stream.accept_waveform.call_args[0]
        assert call_args[1].ndim == 1

    def test_unsupported_type(self, local_stt):
        result = local_stt({"query": 12345})
        assert result["output"] == ""
