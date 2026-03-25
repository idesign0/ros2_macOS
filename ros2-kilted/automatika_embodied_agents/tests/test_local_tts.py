"""Tests for LocalTTS wrapper — no ROS needed."""

import sys
import pytest
import numpy as np
from unittest.mock import MagicMock, patch


@pytest.fixture
def mock_sherpa():
    """Mock sherpa_onnx before importing LocalTTS."""
    mock = MagicMock()
    with patch.dict(sys.modules, {"sherpa_onnx": mock}):
        yield mock


@pytest.fixture
def local_tts(mock_sherpa):
    from agents.utils.local_tts import LocalTTS

    tts = LocalTTS.__new__(LocalTTS)
    tts._tts = MagicMock()
    tts.device = "cpu"
    tts.ncpu = 1
    return tts


class TestLocalTTSCall:
    def test_with_text(self, local_tts):
        samples = np.zeros(16000, dtype=np.float32)
        mock_audio = MagicMock()
        mock_audio.samples = samples
        mock_audio.sample_rate = 24000
        local_tts._tts.generate.return_value = mock_audio

        result = local_tts({"query": "Hello world"})
        assert isinstance(result["output"], bytes)
        assert len(result["output"]) > 0
        # Verify WAV header
        assert result["output"][:4] == b"RIFF"
        local_tts._tts.generate.assert_called_once_with(
            "Hello world", sid=0, speed=1.0
        )

    def test_empty_text(self, local_tts):
        result = local_tts({"query": ""})
        assert result["output"] == b""
