"""Local TTS wrapper using sherpa-onnx."""

import io
import wave
from typing import Dict

import numpy as np


class LocalTTS:
    """Local Text-to-Speech inference using sherpa-onnx.

    Uses Kokoro models by default. The model directory should contain
    model.onnx, voices.bin, tokens.txt, and espeak-ng-data/.

    :param model_path: Path to the model directory containing ONNX and data files
    :param device: Device to run on ('cpu' or 'cuda')
    :param ncpu: Number of CPU threads
    """

    def __init__(self, model_path: str, device: str = "cuda", ncpu: int = 1):
        try:
            import sherpa_onnx
        except ImportError as e:
            raise ImportError(
                "Local TTS model deployment requires sherpa-onnx. "
                "Install it with: pip install sherpa-onnx"
            ) from e

        self.device = device
        self.ncpu = ncpu

        tts_config = sherpa_onnx.OfflineTtsConfig(
            model=sherpa_onnx.OfflineTtsModelConfig(
                kokoro=sherpa_onnx.OfflineTtsKokoroModelConfig(
                    model=f"{model_path}/model.onnx",
                    voices=f"{model_path}/voices.bin",
                    tokens=f"{model_path}/tokens.txt",
                    data_dir=f"{model_path}/espeak-ng-data",
                ),
                num_threads=ncpu,
                provider="cuda" if device == "cuda" else "cpu",
            ),
        )
        self._tts = sherpa_onnx.OfflineTts(tts_config)

    def __call__(self, inference_input: Dict) -> Dict:
        """Run TTS inference.

        :param inference_input: Dict with 'query' (text string)
        :returns: Dict with 'output' (WAV bytes)
        """
        text = inference_input["query"]
        if not text:
            return {"output": b""}

        audio = self._tts.generate(text, sid=0, speed=1.0)
        wav_bytes = self._samples_to_wav(audio.samples, audio.sample_rate)

        return {"output": wav_bytes}

    @staticmethod
    def _samples_to_wav(samples: np.ndarray, sample_rate: int) -> bytes:
        """Convert float32 samples to WAV bytes using stdlib only.

        :param samples: Float32 audio samples in [-1, 1]
        :param sample_rate: Sample rate in Hz
        :returns: WAV file bytes
        """
        samples = np.array(samples, dtype=np.float32)
        int16_data = (samples * 32767).astype(np.int16)
        buf = io.BytesIO()
        with wave.open(buf, "wb") as wf:
            wf.setnchannels(1)
            wf.setsampwidth(2)
            wf.setframerate(sample_rate)
            wf.writeframes(int16_data.tobytes())
        return buf.getvalue()
