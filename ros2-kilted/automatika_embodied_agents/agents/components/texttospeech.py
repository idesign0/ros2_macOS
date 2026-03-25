import queue
import socket
import threading
from io import BytesIO
from typing import Any, Union, Optional, List, Dict, Tuple
import numpy as np
import base64
import time

from ..clients.model_base import ModelClient
from ..clients import RoboMLWSClient, RoboMLRESPClient
from ..config import TextToSpeechConfig
from ..ros import Audio, String, Topic, StreamingString, component_action
from ..utils import validate_func_args, load_model_repo
from .model_component import ModelComponent
from .component_base import ComponentRunType


class TextToSpeech(ModelComponent):
    """
    This component takes in text input and outputs an audio representation of the text using TTS models (e.g. SpeechT5). The generated audio can be played using any audio playback device available on the agent.

    :param inputs: The input topics for the TTS.
        This should be a list of Topic objects, limited to String type.
    :type inputs: list[Topic]
    :param outputs: Optional output topics for the TTS.
        This should be a list of Topic objects, Audio type is handled automatically.
    :type outputs: list[Topic]
    :param model_client: The model client for the TTS.
        This should be an instance of ModelClient. Optional if ``enable_local_model`` is set to True in the config.
    :type model_client: Optional[ModelClient]
    :param config: The configuration for the TTS.
        This should be an instance of TextToSpeechConfig. If not provided, it defaults to TextToSpeechConfig()
    :type config: Optional[TextToSpeechConfig]
    :param trigger: The trigger value or topic for the TTS.
        This can be a single Topic object or a list of Topic objects.
    :type trigger: Union[Topic, list[Topic]]
    :param component_name: The name of the TTS component. This should be a string.
    :type component_name: str

    Example usage:
    ```python
    text_topic = Topic(name="text", msg_type="String")
    audio_topic = Topic(name="audio", msg_type="Audio")
    config = TextToSpeechConfig(play_on_device=True)
    model_client = ModelClient(model=SpeechT5(name="speecht5"))
    tts_component = TextToSpeech(
        inputs=[text_topic],
        outputs=[audio_topic],
        model_client=model_client,
        config=config,
        component_name='tts_component'
    )
    ```

    Example usage with local model:
    ```python
    text_topic = Topic(name="text", msg_type="String")
    config = TextToSpeechConfig(enable_local_model=True, play_on_device=True)
    tts_component = TextToSpeech(
        inputs=[text_topic],
        config=config,
        trigger=text_topic,
        component_name='local_tts'
    )
    ```
    """

    @validate_func_args
    def __init__(
        self,
        *,
        inputs: List[Topic],
        outputs: Optional[List[Topic]] = None,
        model_client: Optional[ModelClient] = None,
        config: Optional[TextToSpeechConfig] = None,
        trigger: Union[Topic, List[Topic]],
        component_name: str,
        **kwargs,
    ):
        self.config: TextToSpeechConfig = config or TextToSpeechConfig()
        self.allowed_inputs = {"Required": [[String, StreamingString]]}
        self.handled_outputs = [Audio]

        if isinstance(trigger, float):
            raise TypeError(
                "TextToSpeech component cannot be started as a timed component"
            )

        if not model_client and not self.config.enable_local_model:
            raise TypeError(
                "TextToSpeech component requires a model_client or enable_local_model=True in TextToSpeechConfig."
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
        # deploy local TTS if enabled
        if not self.model_client and self.config.enable_local_model:
            self._deploy_local_model()
        # Configure component
        super().custom_on_configure()

        # If play_on_device is enabled, start a playing stream on a separate thread
        if self.config.play_on_device:
            self._stream_queue = queue.Queue(maxsize=self.config.buffer_size)
            self._incoming_queue = queue.Queue()
            self._stop_event = threading.Event()
            self._playback_thread: Optional[threading.Thread] = None
            self._thread_lock = threading.Lock()

            if self.config.stream_to_ip and self.config.stream_to_port:
                self.get_logger().info(
                    f"""UDP streaming configured for target {self.config.stream_to_ip}:{self.config.stream_to_port}. Stream can be played at the target machine with gstreamer or ffmpeg, audio format example with gstreamer is shown below:
        gst-launch-1.0 -v udpsrc port={self.config.stream_to_port} caps="audio/x-raw,format=F32LE,channels=1,rate=16000" ! queue ! audioconvert ! audioresample ! alsasink
        """
                )

        # Get bytes as output from server if using appropriate client
        if isinstance(self.model_client, (RoboMLWSClient, RoboMLRESPClient)):
            self.inference_params["get_bytes"] = True

    def custom_on_deactivate(self):
        if self.config.play_on_device:
            # If play_on_device is enabled, stop the playing stream thread
            self._stop_event.set()

        # Deactivate component
        super().custom_on_deactivate()

    def _deploy_local_model(self):
        """Deploy local TTS model on demand."""
        if self.local_model is not None:
            return  # already deployed
        from ..utils.local_tts import LocalTTS

        self.local_model = LocalTTS(
            model_path=load_model_repo(
                "local_tts", self.config.local_model_path
            ),
            device=self.config.device_local_model,
            ncpu=self.config.ncpu_local_model,
        )
        # Local TTS does not support streaming
        if self.config.stream:
            self.config.stream = False
            self.inference_params = self.config.get_inference_params()

    def __stream_callback(
        self, _: bytes, frames: int, time_info: Dict, status: int
    ) -> Tuple[bytes, int]:
        """
        Stream callback for PyAudio, consuming NumPy arrays from the queue.
        """
        try:
            import pyaudio
        except ModuleNotFoundError as e:
            raise ModuleNotFoundError(
                "play_on_device device configuration for TextToSpeech component requires soundfile and pyaudio modules to be installed. Please install them with `pip install soundfile pyaudio`"
            ) from e

        assert frames == self.config.block_size
        if status:
            if pyaudio.paOutputUnderflow:
                self.get_logger().warn(
                    "Output underflow: Try to increase the blocksize. Default is 1024"
                )
            else:
                self.get_logger().warn(f"PyAudio stream status flags: {status}")

        # Bytes PyAudio expects = requested_frames * channels * bytes_per_sample
        expected_bytes_len = (
            frames * self._current_channels * 4  # float32 is 4 bytes per sample
        )
        try:
            # get numpy chunk from soundfile
            data = self._stream_queue.get_nowait()
        except queue.Empty:
            # Send silence until new data is received
            return (b"\x00" * expected_bytes_len, pyaudio.paContinue)

        # If chunk is smaller than the full block then pad
        if data.shape[0] < frames:
            # create padding array of zeros
            padding_frames = frames - data.shape[0]
            padding_np = np.zeros(
                (padding_frames, self._current_channels), dtype=data.dtype
            )
            # concatenate the actual data with padding
            final_data_np = np.concatenate((data, padding_np), axis=0)
            out_data_bytes = final_data_np.tobytes()
            return out_data_bytes, pyaudio.paContinue
        else:
            out_data_bytes = data.tobytes()
            return out_data_bytes, pyaudio.paContinue

    def __get_stream(
        self, stream, audio_interface, new_framerate: int, new_channels: int
    ):
        """If the stream doesn't exist or if the audio format has changed close the old stream and create a new one"""
        if (
            stream is None
            or new_framerate != self._current_framerate
            or new_channels != self._current_channels
        ):
            if stream is not None:
                self.get_logger().debug("Audio format changed. Re-creating stream.")
                stream.stop_stream()
                stream.close()

            # create stream
            stream = audio_interface.open(
                format=self._pyaudio_format,
                channels=new_channels,
                rate=new_framerate,
                output=True,
                frames_per_buffer=self.config.block_size,
                stream_callback=self.__stream_callback,  # type: ignore[attr-defined]
                output_device_index=self.config.device,
            )
            stream.start_stream()
            self._current_framerate = new_framerate
            self._current_channels = new_channels
            self.get_logger().debug(
                "PyAudio stream started. Feeding data using SoundFile blocks..."
            )

        return stream

    def __pre_fill_stream_queue(self, blocks):
        """Pre-fill stream queue to buffer length"""
        for _ in range(self.config.buffer_size):
            try:
                data = next(blocks)
                if not len(data):
                    return
                self._stream_queue.put_nowait(data)
            except queue.Full:
                self.get_logger().warn("Queue already full, skipping prefill.")
                break
            except Exception:
                break

    def __feed_data(self, stream, blocks, timeout: float):
        """Feed blocks to playback stream"""
        for data in blocks:
            try:
                self._stream_queue.put(data, block=True, timeout=timeout)
            except queue.Full:
                self.get_logger().warn(
                    f"Queue full while feeding stream. Timeout was set to {timeout:2.f}. Try to increate the buffer_size in config. Default is 20 (blocks)"
                )
                continue
            if self._stop_event.is_set():
                self.get_logger().debug("Event set, stopping data feed.")
                return

        # Wait until playback is finished after last chunk
        wait_start_time = time.monotonic()
        estimated_remaining_blocks = self._stream_queue.qsize()
        max_wait_timeout = estimated_remaining_blocks * (
            self.config.block_size / self._current_framerate
        )
        max_wait_timeout = max(max_wait_timeout, 0.5)
        max_wait_timeout = min(max_wait_timeout, 2)  # Cap timeout

        while stream and stream.is_active():
            if self._stop_event.is_set():
                return
            if time.monotonic() - wait_start_time > max_wait_timeout:
                self.get_logger().debug(
                    f"Timeout ({max_wait_timeout:.2f}s) waiting for stream to finish."
                )
                return
            time.sleep(0.05)

    def __get_audio_bytes(self, chunk: Union[bytes, str]) -> bytes:
        """Get audio bytes"""
        if isinstance(chunk, str):
            try:
                return base64.b64decode(chunk)
            except Exception as e:
                self.get_logger().error(f"Failed to decode base64 string: {e}")
                return b""
        else:
            return chunk

    def _stream_audio_udp(self):
        """Streams audio chunks to a target IP and port over UDP."""
        # import package
        try:
            from soundfile import SoundFile
        except ModuleNotFoundError as e:
            raise ModuleNotFoundError(
                "UDP streaming for TextToSpeech component requires the soundfile module to be installed. Please install it with `pip install soundfile`"
            ) from e

        sock = None
        try:
            # Create socket
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

            while not self._stop_event.is_set():
                try:
                    # Wait for a new chunk with a timeout.
                    chunk = self._incoming_queue.get(
                        timeout=self.config.thread_shutdown_timeout
                    )
                    if chunk is None:
                        break
                except queue.Empty:
                    self.get_logger().debug(
                        f"No new audio for {self.config.thread_shutdown_timeout}s. "
                        "Gracefully shutting down UDP streaming thread."
                    )
                    break

                # change str to bytes if output is str
                output_bytes = self.__get_audio_bytes(chunk)
                if not output_bytes:
                    continue

                try:
                    with SoundFile(BytesIO(output_bytes)) as f:
                        # request float32 from soundfile
                        blocks = f.blocks(
                            self.config.block_size, dtype="float32", always_2d=True
                        )
                        for block in blocks:
                            if self._stop_event.is_set():
                                break
                            # Convert to raw bytes (float32 LE)
                            byte_block = block.tobytes()
                            sock.sendto(
                                byte_block,
                                (self.config.stream_to_ip, self.config.stream_to_port),
                            )
                except Exception as e:
                    self.get_logger().error(
                        f"Error processing of streaming audio chunk: {e}"
                    )
                    continue
        finally:
            self.get_logger().debug("Cleaning up UDP streaming resources.")
            if sock:
                sock.close()
            self.get_logger().debug("UDP streaming thread has finished.")

    def _playback_audio_local(self):
        """Creates a stream to play audio on local device"""
        # import packages
        try:
            from soundfile import SoundFile
            import pyaudio
        except ModuleNotFoundError as e:
            raise ModuleNotFoundError(
                "play_on_device local device configuration for TextToSpeech component requires soundfile and pyaudio modules to be installed. Please install them with `pip install soundfile pyaudio`"
            ) from e

        # Create pyaudio interface and define stream params
        audio_interface = pyaudio.PyAudio()
        stream: Optional[pyaudio.Stream] = None
        self._current_framerate: int = 0
        self._current_channels: int = 0
        self._pyaudio_format = (
            pyaudio.paFloat32  # Use float32 format for SoundFile compatibility
        )

        try:
            while not self._stop_event.is_set():
                try:
                    # Wait for a new chunk with a timeout.
                    chunk = self._incoming_queue.get(
                        timeout=self.config.thread_shutdown_timeout
                    )
                    # None in the queue is a sentinel to tell the thread to stop.
                    if chunk is None:
                        break
                except queue.Empty:
                    # Queue was empty for the duration of the timeout.
                    self.get_logger().debug(
                        f"No new audio for {self.config.thread_shutdown_timeout}s. "
                        "Gracefully shutting down playback thread."
                    )
                    break  # Exit the loop

                # change str to bytes if output is str
                output_bytes = self.__get_audio_bytes(chunk)

                try:
                    with SoundFile(BytesIO(output_bytes)) as f:
                        new_framerate = f.samplerate
                        new_channels = f.channels

                        # make chunk generator
                        # request float32 from soundfile, as we have pyAudio paFloat32
                        blocks = f.blocks(
                            self.config.block_size, dtype="float32", always_2d=True
                        )
                        # calculate timeout
                        timeout = (
                            self.config.block_size
                            * self.config.buffer_size
                            / f.samplerate
                        )

                        # pre-fill queue if empty
                        if not stream or not stream.is_active():
                            self.__pre_fill_stream_queue(blocks)

                        # Get stream
                        stream = self.__get_stream(
                            stream, audio_interface, new_framerate, new_channels
                        )
                        # Feed data to the device stream
                        self.__feed_data(stream, blocks, timeout)

                except Exception as e:
                    self.get_logger().error(f"Error processing audio chunk: {e}")
                    # Continue to the next chunk
                    continue
        finally:
            # Cleanup: This block will run when the loop exits for any reason.
            self.get_logger().debug("Cleaning up playback resources.")
            if stream:
                try:
                    stream.stop_stream()
                    stream.close()
                except Exception as e:
                    self.get_logger().error(f"Error closing PyAudio stream: {e}")
            if audio_interface:
                try:
                    audio_interface.terminate()
                except Exception as e:
                    self.get_logger().error(f"Error terminating PyAudio instance: {e}")
            # Clear streaming queue
            with self._stream_queue.mutex:
                self._stream_queue.queue.clear()

            stream = None
            audio_interface = None
            self.get_logger().debug("Playback thread has finished.")

    def _play(self, audio_chunk: Union[bytes, str]):
        """
        Adds a chunk of audio data to the playback queue. If the playback thread is not running, it will be started automatically.
        """
        self._incoming_queue.put(audio_chunk)

        # If the playback thread doesn't exist or isn't alive, start it.
        with self._thread_lock:
            if self._playback_thread is None or not self._playback_thread.is_alive():
                self.get_logger().debug(
                    "Playback thread is not active. Starting a new one."
                )
                self._stop_event.clear()
                # Decide which worker to start based on config
                is_streaming_configured = (
                    self.config.stream_to_ip and self.config.stream_to_port
                )
                if is_streaming_configured:
                    target_func = self._stream_audio_udp
                    thread_name = "TTS-UDP-Streamer"
                    self.get_logger().debug(
                        "Playback thread is not active. Starting a new UDP streaming thread."
                    )
                else:
                    target_func = self._playback_audio_local
                    thread_name = "TTS-Local-Playback"
                    self.get_logger().debug(
                        "Playback thread is not active. Starting a new local playback thread."
                    )

                # Create and start thread
                self._playback_thread = threading.Thread(
                    target=target_func, name=thread_name, daemon=True
                )
                self._playback_thread.start()

    @component_action
    def stop_playback(self, wait_for_thread: bool = True):
        """
        Stops the playback thread and clears any pending audio.
        Can be used to interrupt the audio playback through an event.
        """
        self.get_logger().info("Stop requested. Signaling thread and clearing queue.")
        self._stop_event.set()

        # Clear the queue to ensure the thread doesn't process more items
        with self._incoming_queue.mutex:
            self._incoming_queue.queue.clear()

        # Add a sentinel value to unblock a waiting queue.get()
        self._incoming_queue.put(None)

        if wait_for_thread and self._playback_thread:
            self.get_logger().debug("Waiting for playback thread to terminate...")
            self._playback_thread.join()
            self.get_logger().debug("Thread terminated.")

    @component_action
    def say(self, text: str):
        """
            Say the input text.

            This method converts the input text to speech and plays the speech on device if play_on_device is set to True and publishes to Audio topics if any publishers have been provided to the component. Any current playback is stopped.

            The method can be invoked as an action consequence of an event. For example, the robot can say 'I am low on battery" when a low battery event gets triggered.

        :param text: The text to be spoken out loud.
        :type text: str
        """
        self.stop_playback()
        self._execution_step(text=text)

    def _handle_websocket_streaming(self) -> Optional[List]:
        """Handle streaming output from a websocket client"""
        try:
            tokens = self.resp_queue.get(block=True)
            if self.config.play_on_device:
                self._play(tokens)
            self._publish({"output": tokens})
        except Exception as e:
            self.get_logger().error(str(e))
            # raise a fallback trigger via health status
            self.health_status.set_fail_component()

    def _create_input(self, *_, **kwargs) -> Optional[Dict[str, Any]]:
        """Create inference input for TextToSpeech models
        :param args:
        :param kwargs:
        :rtype: dict[str, Any]
        """

        # set query as trigger
        if trigger := kwargs.get("topic"):
            query = self.trig_callbacks[trigger.name].get_output()
            if not query:
                return None
        elif text := kwargs.get("text"):
            query = text
        else:
            self.get_logger().error(
                "Trigger topic not found. TextToSpeech component needs to be given a valid trigger topic."
            )
            return None
        return {"query": query, **self.inference_params}

    def _execution_step(self, *args, **kwargs):
        """_execution_step.

        :param args:
        :param kwargs:
        """
        if self.run_type is ComponentRunType.EVENT and (trigger := kwargs.get("topic")):
            self.get_logger().debug(f"Received trigger on topic {trigger.name}")
        elif text := kwargs.get("text"):
            self.get_logger().debug(f"Received text: {text}")
        else:
            return None

        # create inference input
        inference_input = self._create_input(*args, **kwargs)
        # call model inference
        if not inference_input:
            self.get_logger().warning("Input not received, not calling model inference")
            return

        # conduct inference
        result = self._call_inference(inference_input)
        if result:
            if self.config.play_on_device:
                self._play(result["output"])

            # publish result
            self._publish(result)

    def _warmup(self):
        """Warm up and stat check"""
        import time

        inference_input = {
            "query": "Add the sum to the product of these three.",
            **self.inference_params,
        }

        # Run inference once to warm up and once to measure time
        if self.model_client:
            self.model_client.inference(inference_input)
        elif hasattr(self, "local_model"):
            self.local_model(inference_input)

        start_time = time.time()
        if self.model_client:
            self.model_client.inference(inference_input)
        elif hasattr(self, "local_model"):
            self.local_model(inference_input)
        elapsed_time = time.time() - start_time

        self.get_logger().warning(f"Approximate Inference time: {elapsed_time} seconds")
        self.get_logger().warning(
            f"RTF: {elapsed_time / 2}"  # approx audio length, 2 seconds
        )
