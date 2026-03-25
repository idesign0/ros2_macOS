from typing import Any, Union, Optional, List, Dict, Tuple
import queue
import threading
import numpy as np
from collections import deque

import msgpack
from ..clients.model_base import ModelClient
from agents.clients import RoboMLWSClient
from ..config import SpeechToTextConfig
from ..ros import Audio, String, Topic
from ..utils import (
    validate_func_args,
    VADStatus,
    WakeWordStatus,
    load_model,
    load_model_repo,
)
from .model_component import ModelComponent
from .component_base import ComponentRunType


class SpeechToText(ModelComponent):
    """
    This component takes in audio input and outputs a text representation of the audio using Speech-to-Text models (e.g. Whisper).

    :param inputs: The input topics for the STT.
        This should be a list of Topic objects, limited to Audio type.
    :type inputs: list[Topic]
    :param outputs: The output topics for the STT.
        This should be a list of Topic objects, String type is handled automatically.
    :type outputs: list[Topic]
    :param model_client: The model client for the STT.
        This should be an instance of ModelClient. Optional if ``enable_local_model`` is set to True in the config.
    :type model_client: Optional[ModelClient]
    :param config: The configuration for the STT.
        This should be an instance of SpeechToTextConfig. If not provided, defaults to SpeechToTextConfig().
    :type config: Optional[SpeechToTextConfig]
    :param trigger: The trigger value or topic for the STT.
        This can be a single Topic object, a list of Topic objects.
    :type trigger: Union[Topic, list[Topic], float]
    :param component_name: The name of the STT component. This should be a string.
    :type component_name: str

    Example usage:
    ```python
    audio_topic = Topic(name="audio", msg_type="Audio")
    text_topic = Topic(name="text", msg_type="String")
    config = SpeechToTextConfig(enable_vad=True)
    model = Whisper(name="whisper")
    model_client = ModelClient(model=model)
    stt_component = SpeechToText(
        inputs=[audio_topic],
        outputs=[text_topic],
        model_client=model_client,
        config=config,
        component_name='stt_component'
    )
    ```

    Example usage with local model:
    ```python
    audio_topic = Topic(name="audio", msg_type="Audio")
    text_topic = Topic(name="text", msg_type="String")
    config = SpeechToTextConfig(enable_local_model=True, enable_vad=True)
    stt_component = SpeechToText(
        inputs=[audio_topic],
        outputs=[text_topic],
        config=config,
        trigger=audio_topic,
        component_name='local_stt'
    )
    ```
    """

    @validate_func_args
    def __init__(
        self,
        *,
        inputs: List[Topic],
        outputs: List[Topic],
        model_client: Optional[ModelClient] = None,
        config: Optional[SpeechToTextConfig] = None,
        trigger: Union[Topic, List[Topic]],
        component_name: str,
        **kwargs,
    ):
        self.config: SpeechToTextConfig = config or SpeechToTextConfig()
        self.allowed_inputs = {"Required": [Audio]}
        self.handled_outputs = [String]

        if isinstance(trigger, float):
            raise TypeError(
                "SpeechToText component cannot be started as a timed component"
            )

        if not model_client and not self.config.enable_local_model:
            raise TypeError(
                "SpeechToText component requires a model_client or enable_local_model=True in SpeechToTextConfig."
            )

        if (
            self.config.stream
            and model_client
            and not isinstance(model_client, RoboMLWSClient)
        ):
            raise TypeError(
                "SpeechToText component can only stream audio to the server when using RoboMLWebSocketClient. Please set stream to False in config or use a different client."
            )

        self.model_client = model_client

        super().__init__(
            inputs,
            outputs,
            model_client,
            self.config,
            trigger,
            component_name,
            **kwargs,
        )

    def custom_on_configure(self):
        """Custom configuration"""
        # deploy local STT if enabled
        if not self.model_client and self.config.enable_local_model:
            self._deploy_local_model()

        super().custom_on_configure()

    def custom_on_activate(self):
        """Custom activation"""
        # NOTE: Custom activate to ensure creation of separate thread if VAD is enabled
        # happens after activation as VAD starts sending received voice to execution
        # step right away

        # Activate component
        super().custom_on_activate()

        # If VAD is enabled, start a listening stream on a separate thread
        if self.config.enable_vad:
            from ..utils.voice import VADIterator

            self.event = threading.Event()
            self.queue = queue.Queue()
            self.vad_iterator = VADIterator(
                model_path=load_model("silero_vad", self.config.vad_model_path),
                threshold=self.config.vad_threshold,
                sample_rate=self.config._sample_rate,
                min_silence_duration_ms=self.config.min_silence_duration_ms,
                speech_pad_ms=self.config.speech_pad_ms,
                ncpu=self.config.ncpu_vad,
                device=self.config.device_vad,
            )
            self.speech_buffer = deque(
                maxlen=int(
                    self.config._sample_rate
                    * self.config.speech_buffer_max_len
                    / (1000 * self.config._block_size)
                )
            )
            if self.config.enable_wakeword:
                from ..utils.voice import WakeWord, AudioFeatures

                self.audio_features = AudioFeatures(
                    melspectogram_model_path=load_model(
                        "melspec", self.config.melspectrogram_model_path
                    ),
                    embedding_model_path=load_model(
                        "voice_embeddings", self.config.embedding_model_path
                    ),
                    ncpu=self.config.ncpu_wakeword,
                    device=self.config.device_wakeword,
                )
                self.wake_word = WakeWord(
                    model_path=load_model(
                        "hey_jarvis", self.config.wakeword_model_path
                    ),
                    threshold=self.config.wakeword_threshold,
                    ncpu=self.config.ncpu_wakeword,
                    device=self.config.device_wakeword,
                )
                self.wake_word_triggered = False

            # initialize response buffer used for output when streaming input
            if self.config.stream:
                from ..utils.voice import HypothesisBuffer

                self.transcript_buffer: HypothesisBuffer = HypothesisBuffer()
                self.result_partial: List = []
                self.min_chunk_size = int(
                    self.config._sample_rate
                    * self.config.min_chunk_size
                    / (1000 * self.config._block_size)
                )

            # start listening thread
            self.listening_thread = threading.Thread(
                target=self._process_audio, daemon=True
            )
            self.listening_thread.start()

    def custom_on_deactivate(self):
        # If VAD is enabled, stop the listening stream thread
        if self.config.enable_vad:
            self.event.set()
            if self.listening_thread:
                self.listening_thread.join()

        # Deactivate component
        super().custom_on_deactivate()

    def _deploy_local_model(self):
        """Deploy local STT model on demand."""
        if self.local_model is not None:
            return  # already deployed
        from ..utils.local_stt import LocalSTT

        self.local_model = LocalSTT(
            model_path=load_model_repo(
                "local_stt", self.config.local_model_path
            ),
            device=self.config.device_local_model,
            ncpu=self.config.ncpu_local_model,
            sample_rate=self.config._sample_rate
        )
        # Local STT does not support streaming
        if self.config.stream:
            self.config.stream = False

    def __stream_callback(
        self, indata: bytes, frames: int, _, status
    ) -> Tuple[bytes, int]:
        """Stream callback function for processing audio

        :param indata:
        :type indata: np.ndarray
        :param frames:
        :type frames: int
        :param _:
        :param status:
        :type status: sd.CallbackFlags
        :rtype: None
        """
        assert frames == self.config._block_size
        if status:
            self.get_logger().warn(f"Status: {status}")
        try:
            import pyaudio
        except ModuleNotFoundError as e:
            raise ModuleNotFoundError(
                "enable_vad configuration for SpeechToText component requires pyaudio module to be installed. Please install it with `pip install pyaudio`"
            ) from e
        np_frames = np.frombuffer(indata, dtype=np.int16).astype(np.float32)
        vad_output = self.vad_iterator(np_frames)

        # if wake word is enabled then store speech when its triggered
        if self.config.enable_wakeword:
            # create audio embeddings for wakeword classifier
            self.audio_features(np_frames)
            if self.wake_word_triggered:
                self.speech_buffer.append(indata)

                # Send input if streaming enabled
                if (
                    self.config.stream
                    and len(self.speech_buffer) >= self.min_chunk_size
                ):
                    self._execution_step(speech=self.speech_buffer)

        # otherwise store speech when vad is triggered
        elif self.vad_iterator.triggered:
            self.speech_buffer.append(indata)

            # Send input if streaming enabled
            if self.config.stream and len(self.speech_buffer) >= self.min_chunk_size:
                self._execution_step(speech=self.speech_buffer)

        # add vad status outputs to queue
        if vad_output:
            self.queue.put_nowait(vad_output)

        return indata, pyaudio.paContinue

    def _process_audio(self) -> None:
        """Creates a stream to process audio from device"""

        # clear event and queue
        self.queue.queue.clear()
        self.event.clear()

        try:
            import pyaudio
        except ModuleNotFoundError as e:
            raise ModuleNotFoundError(
                "enable_vad configuration for SpeechToText component requires pyaudio module to be installed. Please install it with `pip install pyaudio`"
            ) from e
        # Create an interface to PortAudio
        audio_interface = pyaudio.PyAudio()

        stream = audio_interface.open(
            format=pyaudio.paInt16,
            channels=1,
            rate=self.config._sample_rate,
            frames_per_buffer=self.config._block_size,
            input=True,
            start=True,
            input_device_index=self.config.device_audio,
            stream_callback=self.__stream_callback,  # type: ignore
        )

        while True:
            vad_output = self.queue.get()
            self.__process_vad_output(vad_output)
            if self.event.is_set():
                stream.stop_stream()
                stream.close()
                audio_interface.terminate()
                break
        self.event.wait()

    def __process_vad_output(self, vad_output: VADStatus):
        """Process VAD Status"""
        if vad_output is VADStatus.START:
            # When someone starts speaking, check for wakeword if enabled
            self.get_logger().debug("Speech started")
            if self.config.enable_wakeword:
                self.wake_word(
                    self.audio_features.get_embeddings(self.wake_word.model_input)
                )
        elif vad_output is VADStatus.ONGOING:
            self.get_logger().debug("Speech ongoing")
            if self.config.enable_wakeword:
                wake_status = self.wake_word(
                    self.audio_features.get_embeddings(self.wake_word.model_input)
                )
                if wake_status is WakeWordStatus.END:
                    self.get_logger().debug("Wakeword ended")
                    self.wake_word_triggered = True
        elif vad_output is VADStatus.END:
            # Send audio when speech finishes
            self.get_logger().debug("Speech ended")
            if self.config.enable_wakeword:
                self.wake_word_triggered = False
            if self.config.stream:
                # Send again in case last segment was shorter than min_chunk_size
                self._execution_step(speech=self.speech_buffer)
                # Send termination token
                self._execution_step(speech=[b"\r\n"])
            else:
                self._execution_step(speech=self.speech_buffer)
            self.speech_buffer.clear()

    def _handle_websocket_streaming(self) -> Optional[List]:
        """Handle streaming output from a websocket client"""
        try:
            message = self.resp_queue.get(block=True)
            tokens = msgpack.unpackb(message)

            # if termination token is not received then add to hypothesis
            if not tokens == b"\r\n":
                self.transcript_buffer.insert(tokens)
                if newly_committed_words := self.transcript_buffer.flush():
                    self.result_partial.extend(newly_committed_words)

            # publish output when termination token received
            else:
                # Get any last words not currently confirmed in hypothesis
                if remaining_words := self.transcript_buffer.complete():
                    self.result_partial.extend(remaining_words)
                complete_transcript = "".join(i[2] for i in self.result_partial)
                self.get_logger().debug(complete_transcript)
                self._publish({"output": complete_transcript})
                # reset buffers
                self.result_partial = []
                self.transcript_buffer.reset()

        except Exception as e:
            self.get_logger().error(str(e))
            # raise a fallback trigger via health status
            self.health_status.set_fail_component()

    def _create_input(self, *_, **kwargs) -> Optional[Dict[str, Any]]:
        """Create inference input for SpeechToText models
        :param args:
        :param kwargs:
        :rtype: dict[str, Any]
        """

        if self.config.enable_vad and (speech := kwargs.get("speech")) is not None:
            query = b"".join(speech)
        elif trigger := kwargs.get("topic"):
            query = self.trig_callbacks[trigger.name].get_output()
            if query is None or len(query) == 0:
                return None
        else:
            self.get_logger().error(
                "Trigger topic not found. SpeechToText component needs to be given a valid trigger topic."
            )
            return None

        return {
            "query": query,
            **self.inference_params,
        }

    def _execution_step(self, *args, **kwargs):
        """_execution_step.

        :param args:
        :param kwargs:
        """
        # Check for direct speech before checking for triggers
        if self.config.enable_vad and (speech := kwargs.get("speech")):
            self.get_logger().debug(
                f"Received speech from speech thread: {len(speech)}"
            )
        elif self.run_type is ComponentRunType.EVENT and (
            trigger := kwargs.get("topic")
        ):
            self.get_logger().debug(f"Received trigger on topic {trigger.name}")
        else:
            return None

        # create inference input
        inference_input = self._create_input(*args, **kwargs)
        if not inference_input:
            self.get_logger().warning("Input not received, not calling model inference")
            return

        # call model inference
        result = self._call_inference(inference_input)
        if result:
            # publish inference result
            self._publish(result)

    def _warmup(self):
        """Warm up and stat check"""
        import time
        from pathlib import Path

        with open(
            str(Path(__file__).parents[1] / Path("resources/test.wav")), "rb"
        ) as file:
            file_bytes = file.read()

        inference_input = {"query": file_bytes, **self.inference_params}

        # Run inference once to warm up and once to measure time
        if self.model_client:
            self.model_client.inference(inference_input)
        elif hasattr(self, "local_model"):
            self.local_model(inference_input)

        start_time = time.time()
        if self.model_client:
            result = self.model_client.inference(inference_input)
        elif hasattr(self, "local_model"):
            result = self.local_model(inference_input)
        else:
            result = None
        elapsed_time = time.time() - start_time

        if result:
            self.get_logger().warning(f"Model Output: {result['output']}")
            self.get_logger().warning(
                f"Approximate Inference time: {elapsed_time} seconds"
            )
            self.get_logger().warning(
                f"RTF: {elapsed_time / 2}"  # audio length, 2 seconds
            )
        else:
            self.get_logger().error("Model inference failed during warmup.")
