from typing import Optional, List
from typing import Callable
import logging

import numpy as np
from .utils import VADStatus, WakeWordStatus

try:
    import onnxruntime as ort
except ModuleNotFoundError as e:
    raise ModuleNotFoundError(
        """enable_vad and enable_wakeword in SpeechToText component requires onnxruntime to be installed. Please install them with `pip install onnxruntime` or `pip install onnxruntime-gpu` for cpu or gpu based deployment.

        For Jetson devices you can download the pre-built ONNX runtime wheels corresponding to your Jetpack version at https://elinux.org/Jetson_Zoo#ONNX_Runtime"""
    ) from e


def _get_onnx_providers(device: str, model: str) -> List[str]:
    """Check for available providers"""
    available = ort.get_available_providers()
    logger = logging.getLogger(model)

    if device == "cuda":
        if "CUDAExecutionProvider" not in available:
            logger.warning(
                f"CUDA is not available for {model}. Ensure the correct CUDA/cuDNN versions are installed and install ONNX Runtime with `pip install onnxruntime-gpu`. Switching to CPU runtime."
            )
        return ["CUDAExecutionProvider", "CPUExecutionProvider"]

    if device == "tensorrt":
        if "TensorrtExecutionProvider" not in available:
            logger.warning(
                f"Tensorrt is not available for {model}. Ensure the correct CUDA/cuDNN versions are installed and install ONNX Runtime with TensorRT support. Switching to CPU runtime."
            )
        return [
            "TensorrtExecutionProvider",
            "CUDAExecutionProvider",
            "CPUExecutionProvider",
        ]

    return ["CPUExecutionProvider"]


class VADIterator:
    """Adapted from https://github.com/snakers4/silero-vad/blob/master/src/silero_vad/utils_vad.py
    Check out https://github.com/snakers4/silero-vad
    Citation:
        @misc{Silero VAD,
          author = {Silero Team},
          title = {Silero VAD: pre-trained enterprise-grade Voice Activity Detector (VAD), Number Detector and Language Classifier},
          year = {2024},
          publisher = {GitHub},
          journal = {GitHub repository},
          howpublished = {https://github.com/snakers4/silero-vad},
          commit = {insert_some_commit_here},
          email = {hello@silero.ai}
        }

    """

    def __init__(
        self,
        model_path: str,
        threshold: float = 0.5,
        sample_rate: int = 16000,
        min_silence_duration_ms: int = 1000,
        speech_pad_ms: int = 30,
        ncpu: int = 1,
        device: str = "cpu",
    ):
        self.threshold = threshold

        self.sample_rate = np.array(sample_rate).astype(np.int64)

        # Initialize the ONNX model
        sessionOptions = ort.SessionOptions()
        sessionOptions.inter_op_num_threads = ncpu
        sessionOptions.intra_op_num_threads = ncpu

        providers = _get_onnx_providers(device, "VAD")
        self.model = ort.InferenceSession(
            model_path, sess_options=sessionOptions, providers=providers
        )

        # State variable required by vad model
        self._state = np.zeros((2, 1, 128)).astype("float32")

        self.min_silence_samples = sample_rate * min_silence_duration_ms / 1000
        self.speech_pad_samples = sample_rate * speech_pad_ms / 1000

        self.reset_states()

    def reset_states(self):
        self.triggered = False
        self.temp_end = 0
        self.current_sample = 0

    def __call__(self, x_np_32: np.ndarray) -> Optional[VADStatus]:
        """
        x: np.ndarray dtype:int16
            audio chunks
        """
        chunks = np.array_split(x_np_32, 2)
        speech_probs = []
        for chunk in chunks:
            window_size_samples = chunk.shape[0]
            self.current_sample += window_size_samples

            ort_inputs = {
                "input": chunk[None,] / 32768,
                "state": self._state,
                "sr": self.sample_rate,
            }

            out, self._state = self.model.run(None, ort_inputs)
            speech_probs.append(out.squeeze())

        speech_prob = np.mean(speech_probs)

        if (speech_prob >= self.threshold) and self.temp_end:
            self.temp_end = 0

        if (speech_prob >= self.threshold) and not self.triggered:
            self.triggered = True
            return VADStatus.START

        if (speech_prob < self.threshold - 0.15) and self.triggered:
            if not self.temp_end:
                self.temp_end = self.current_sample
            if self.current_sample - self.temp_end < self.min_silence_samples:
                return VADStatus.ONGOING
            else:
                self.temp_end = 0
                self.triggered = False
                return VADStatus.END

        return None


class AudioFeatures:
    """
     Adapted streaming implementation of AudioFeatures class from the wonderful [openWakeWord project](https://github.com/dscripka/openWakeWord/). This class converts raw audio to audio embeddings. It uses the following two ONNX models.

    - An ONNX implementation of Torch's melspectrogram function with fixed parameters.

    - A shared feature extraction backbone model that converts melspectrogram inputs into general-purpose speech audio embeddings. This model is provided by Google as a TFHub module under an Apache-2.0 license and manually reimplemented in openWakeWord.
    """

    def __init__(
        self,
        melspectogram_model_path: str,
        embedding_model_path: str,
        ncpu: int = 1,
        device: str = "cpu",
    ):
        # Initialize ONNX options
        sessionOptions = ort.SessionOptions()
        sessionOptions.inter_op_num_threads = ncpu
        sessionOptions.intra_op_num_threads = ncpu

        providers = _get_onnx_providers(device, "WakeWord")
        # Initialize melspectrogram model
        self.melspec_model = ort.InferenceSession(
            melspectogram_model_path, sess_options=sessionOptions, providers=providers
        )

        self.melspec_model_predict = lambda x: self.melspec_model.run(
            None, {"input": x}
        )

        # Initialize audio embedding model
        self.embedding_model = ort.InferenceSession(
            embedding_model_path, sess_options=sessionOptions, providers=providers
        )

        self.embedding_model_predict = lambda x: self.embedding_model.run(
            None, {"input_1": x}
        )[0].squeeze()

        # Buffers for storing melspectrograms and embeddings
        self.melspectrogram_buffer = np.ones((76, 32))  # n_frames x num_features
        self.embeddings_buffer = self._initialize_random_embeddings(
            np.random.randint(-1000, 1000, 16000 * 4).astype(np.float32)
        )

    def _get_melspectrogram(
        self,
        x: np.ndarray,
        melspec_transform: Callable = lambda x: x / 10 + 2,
    ):
        """
        Function to compute the mel-spectrogram of the provided audio samples.
        melspec_transform: A function to transform the computed melspectrogram.
        Defaults to a transform that makes the ONNX melspectrogram model closer to the native Tensorflow implementation from Google (https://tfhub.dev/google/speech_embedding/1).
        """
        x = x[None,] if len(x.shape) < 2 else x

        # compute melspectrogram
        outputs = self.melspec_model_predict(x)
        spec = np.squeeze(outputs[0])

        # transform melspectrogram
        spec = melspec_transform(spec)

        return spec

    def _initialize_random_embeddings(
        self, x: np.ndarray, window_size: int = 76, step_size: int = 8, **kwargs
    ):
        """Initialize random embeddings"""
        spec = self._get_melspectrogram(x, **kwargs)
        windows = []
        for i in range(0, spec.shape[0], step_size):
            window = spec[i : i + window_size]
            if window.shape[0] == window_size:  # truncate short windows
                windows.append(window)
        batch = np.expand_dims(np.array(windows), axis=-1).astype(np.float32)
        embedding = self.embedding_model_predict(batch)
        return embedding

    def get_embeddings(self, n_feature_frames: int = 16) -> np.ndarray:
        """Get computed embeddings from the buffer"""
        return self.embeddings_buffer[int(-1 * n_feature_frames) :, :][None,].astype(
            np.float32
        )

    def __call__(self, x):
        """Calclate melspetrogram and audio embeddings"""
        # calculate melspectogram
        mels = self._get_melspectrogram(x)
        self.melspectrogram_buffer = np.roll(
            self.melspectrogram_buffer, -mels.shape[0], axis=0
        )
        self.melspectrogram_buffer[-mels.shape[0] :] = mels

        # calculate audio embeddings
        x = self.melspectrogram_buffer.astype(np.float32)[None, :, :, None]

        self.embeddings_buffer = np.roll(self.embeddings_buffer, -1, axis=0)
        self.embeddings_buffer[-1:] = self.embedding_model_predict(x)


class WakeWord:
    """
    A wakeword model classification class that uses pre-trained wakeword models adapted for models presented in [openWakeWord project](https://github.com/dscripka/openWakeWord/).
    This class consumed audio embeddings and gives out the probability of detecting a wakeword.
    Simple models like 2 layer RNNs work fairly well as wakeword classification models. Pre-trained models from openWakeWord are available [here](https://github.com/dscripka/openWakeWord/tree/main?tab=readme-ov-file#pre-trained-models).
    To train a custom model, follow this simple [tutorial](https://github.com/dscripka/openWakeWord/blob/main/notebooks/automatic_model_training.ipynb) provided by openWakeWord. Please check [licensing information](https://github.com/dscripka/openWakeWord/tree/main?tab=readme-ov-file#license) for pre-trained models, before utilizing them.
    """

    def __init__(
        self,
        model_path: str,
        threshold: float = 0.6,
        ncpu=1,
        device="cpu",
    ):
        sessionOptions = ort.SessionOptions()
        sessionOptions.inter_op_num_threads = ncpu
        sessionOptions.intra_op_num_threads = ncpu

        providers = _get_onnx_providers(device, "WakeWord")
        # Create inference session
        self.model = ort.InferenceSession(
            model_path, sess_options=sessionOptions, providers=providers
        )
        self.model_input = self.model.get_inputs()[0].shape[1]
        self.model_output = self.model.get_outputs()[0].shape[1]
        self.threshold = threshold
        self.prob: float = 0.0

    def _predict(self, x) -> Optional[WakeWordStatus]:
        self.prob = self.model.run(None, {self.model.get_inputs()[0].name: x})[
            0
        ].squeeze()

    def __call__(self, x: np.ndarray):
        """Check wakeword probability in audio embeddings"""
        if self.prob < self.threshold:
            self._predict(x)
            if self.prob > self.threshold:
                return WakeWordStatus.START
            else:
                return
        elif self.prob >= self.threshold:
            self._predict(x)
            if self.prob < self.threshold:
                return WakeWordStatus.END
            else:
                return WakeWordStatus.ONGOING


class HypothesisBuffer:
    """A simplified Hypothesis buffer for collection output from a streaming speech to text model based on [whisper_stream](https://github.com/ufal/whisper_streaming). Implements LocalAgreement-n policy as used in CUNI-KIT at IWSLT 2022 etc. before.
        @inproceedings{machacek-etal-2023-turning,
        title = "Turning Whisper into Real-Time Transcription System",
        author = "Mach{\'a}{\v{c}}ek, Dominik  and
          Dabre, Raj  and
          Bojar, Ond{\v{r}}ej",
        editor = "Saha, Sriparna  and
          Sujaini, Herry",
        booktitle = "Proceedings of the 13th International Joint Conference on Natural Language Processing and the 3rd Conference of the Asia-Pacific Chapter of the Association for Computational Linguistics: System Demonstrations",
        month = nov,
        year = "2023",
        address = "Bali, Indonesia",
        publisher = "Association for Computational Linguistics",
        url = "https://aclanthology.org/2023.ijcnlp-demo.3",
        pages = "17--24",
    }
    """

    def __init__(self):
        self.commited_in_buffer = []
        self.buffer = []
        self.new = []
        self.last_commited_time = 0

    def reset(self):
        self.commited_in_buffer = []
        self.buffer = []
        self.new = []
        self.last_commited_time = 0

    def insert(self, new):
        # Add new words
        self.new = [(a, b, t) for a, b, t in new if a > self.last_commited_time - 0.1]

        # Remove up to 5 duplicates if they exist in previously commited
        if self.new and self.commited_in_buffer:
            cn = len(self.commited_in_buffer)
            nn = len(self.new)
            for i in range(1, min(min(cn, nn), 5) + 1):
                c = " ".join(
                    [self.commited_in_buffer[-j][2] for j in range(1, i + 1)][::-1]
                )
                tail = " ".join(self.new[j - 1][2] for j in range(1, i + 1))
                if c == tail:
                    [repr(self.new.pop(0)) for _ in range(i)]
                    break

    def flush(self):
        commit = []
        # loop to confirm words in transcript received in previous step
        while self.new:
            if not self.buffer:
                break
            na, nb, nt = self.new[0]
            if nt == self.buffer[0][2] and abs(na - self.buffer[0][0]) < 0.2:
                commit.append((na, nb, nt))
                self.last_commited_time = nb
                self.buffer.pop(0)
                self.new.pop(0)
            else:
                break

        self.buffer = self.new
        self.new = []
        # commit confirmed words
        self.commited_in_buffer.extend(commit)
        return commit

    def complete(self):
        # send any remaining words in buffer
        return self.buffer
