"""Tests for SpeechToText component — requires rclpy."""

import pytest
from unittest.mock import MagicMock

from agents.config import SpeechToTextConfig
from agents.ros import Topic
from agents.components.speechtotext import SpeechToText
from tests.conftest import mock_component_internals


class TestSTTConstruction:
    def test_with_model_client(self, rclpy_init, mock_model_client):
        audio = Topic(name="audio", msg_type="Audio")
        text = Topic(name="text", msg_type="String")
        comp = SpeechToText(
            inputs=[audio],
            outputs=[text],
            model_client=mock_model_client,
            config=SpeechToTextConfig(),
            trigger=audio,
            component_name="test_stt",
        )
        assert comp.model_client is mock_model_client

    def test_with_local_model(self, rclpy_init):
        audio = Topic(name="audio", msg_type="Audio")
        text = Topic(name="text", msg_type="String")
        comp = SpeechToText(
            inputs=[audio],
            outputs=[text],
            config=SpeechToTextConfig(enable_local_model=True),
            trigger=audio,
            component_name="test_stt_local",
        )
        assert comp.config.enable_local_model is True

    def test_no_client_no_local_raises(self, rclpy_init):
        audio = Topic(name="audio", msg_type="Audio")
        text = Topic(name="text", msg_type="String")
        with pytest.raises(TypeError):
            SpeechToText(
                inputs=[audio],
                outputs=[text],
                config=SpeechToTextConfig(),
                trigger=audio,
                component_name="test_stt_fail",
            )

    def test_float_trigger_raises(self, rclpy_init, mock_model_client):
        audio = Topic(name="audio", msg_type="Audio")
        text = Topic(name="text", msg_type="String")
        with pytest.raises(TypeError):
            SpeechToText(
                inputs=[audio],
                outputs=[text],
                model_client=mock_model_client,
                config=SpeechToTextConfig(),
                trigger=1.0,
                component_name="test_stt_timed",
            )

    def test_stream_without_ws_raises(self, rclpy_init, mock_model_client):
        audio = Topic(name="audio", msg_type="Audio")
        text = Topic(name="text", msg_type="String")
        with pytest.raises(TypeError):
            SpeechToText(
                inputs=[audio],
                outputs=[text],
                model_client=mock_model_client,
                config=SpeechToTextConfig(stream=True, enable_vad=True),
                trigger=audio,
                component_name="test_stt_stream",
            )


class TestSTTCreateInput:
    @pytest.fixture
    def stt(self, rclpy_init, mock_model_client):
        audio = Topic(name="audio", msg_type="Audio")
        text = Topic(name="text", msg_type="String")
        comp = SpeechToText(
            inputs=[audio],
            outputs=[text],
            model_client=mock_model_client,
            config=SpeechToTextConfig(),
            trigger=audio,
            component_name="test_stt_input",
        )
        mock_component_internals(comp)
        return comp

    def test_from_vad_speech(self, stt):
        stt.config.enable_vad = True
        result = stt._create_input(speech=[b"aaa", b"bbb"])
        assert result is not None
        assert result["query"] == b"aaabbb"

    def test_from_trigger(self, stt):
        trigger = Topic(name="audio", msg_type="Audio")
        mock_cb = MagicMock()
        mock_cb.get_output.return_value = b"audio_bytes"
        stt.trig_callbacks = {"audio": mock_cb}

        result = stt._create_input(topic=trigger)
        assert result is not None
        assert result["query"] == b"audio_bytes"

    def test_empty_returns_none(self, stt):
        trigger = Topic(name="audio", msg_type="Audio")
        mock_cb = MagicMock()
        mock_cb.get_output.return_value = None
        stt.trig_callbacks = {"audio": mock_cb}

        result = stt._create_input(topic=trigger)
        assert result is None
