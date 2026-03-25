"""Tests for TextToSpeech component — requires rclpy."""

import pytest
from unittest.mock import MagicMock

from agents.config import TextToSpeechConfig
from agents.ros import Topic
from agents.components.texttospeech import TextToSpeech
from tests.conftest import mock_component_internals


class TestTTSConstruction:
    def test_with_model_client(self, rclpy_init, mock_model_client):
        text = Topic(name="text", msg_type="String")
        audio = Topic(name="audio", msg_type="Audio")
        comp = TextToSpeech(
            inputs=[text],
            outputs=[audio],
            model_client=mock_model_client,
            config=TextToSpeechConfig(),
            trigger=text,
            component_name="test_tts",
        )
        assert comp.model_client is mock_model_client

    def test_with_local_model(self, rclpy_init):
        text = Topic(name="text", msg_type="String")
        comp = TextToSpeech(
            inputs=[text],
            config=TextToSpeechConfig(enable_local_model=True, stream=False),
            trigger=text,
            component_name="test_tts_local",
        )
        assert comp.config.enable_local_model is True

    def test_no_client_no_local_raises(self, rclpy_init):
        text = Topic(name="text", msg_type="String")
        with pytest.raises(TypeError):
            TextToSpeech(
                inputs=[text],
                config=TextToSpeechConfig(stream=False),
                trigger=text,
                component_name="test_tts_fail",
            )


class TestTTSCreateInput:
    @pytest.fixture
    def tts(self, rclpy_init, mock_model_client):
        text = Topic(name="text", msg_type="String")
        audio = Topic(name="audio", msg_type="Audio")
        comp = TextToSpeech(
            inputs=[text],
            outputs=[audio],
            model_client=mock_model_client,
            config=TextToSpeechConfig(),
            trigger=text,
            component_name="test_tts_input",
        )
        mock_component_internals(comp)
        return comp

    def test_from_trigger(self, tts):
        trigger = Topic(name="text", msg_type="String")
        mock_cb = MagicMock()
        mock_cb.get_output.return_value = "Hello world"
        tts.trig_callbacks = {"text": mock_cb}

        result = tts._create_input(topic=trigger)
        assert result is not None
        assert result["query"] == "Hello world"

    def test_from_text_kwarg(self, tts):
        result = tts._create_input(text="Direct text")
        assert result is not None
        assert result["query"] == "Direct text"
