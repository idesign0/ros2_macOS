"""Local STT wrapper using sherpa-onnx."""

from glob import glob
from typing import Dict

import numpy as np


class LocalSTT:
    """Local Speech-to-Text inference using sherpa-onnx.

    Uses Whisper models by default. The model directory should contain
    encoder, decoder, and tokens files (auto-detected via glob).
    Int8 quantized variants are preferred when available.

    :param model_path: Path to the model directory containing ONNX files
    :param device: Device to run on ('cpu' or 'cuda')
    :param ncpu: Number of CPU threads
    """

    def __init__(self, model_path: str, device: str = "cuda", ncpu: int = 1, sample_rate=16000):
        try:
            import sherpa_onnx
        except ImportError as e:
            raise ImportError(
                "Local STT model deployment requires sherpa-onnx. "
                "Install it with: pip install sherpa-onnx"
            ) from e

        self.device = device
        self.ncpu = ncpu
        self._sample_rate = sample_rate

        # Auto-detect model files — prefer int8 variants for edge
        encoders = sorted(glob(f"{model_path}/*encoder*.onnx"))
        decoders = sorted(glob(f"{model_path}/*decoder*.onnx"))
        tokens_files = glob(f"{model_path}/*tokens*.txt")

        if not encoders or not decoders or not tokens_files:
            raise FileNotFoundError(
                f"Could not find required model files in {model_path}. "
                "Expected *encoder*.onnx, *decoder*.onnx, and *tokens*.txt"
            )

        encoder = next((f for f in encoders if "int8" in f), encoders[0])
        decoder = next((f for f in decoders if "int8" in f), decoders[0])
        tokens = tokens_files[0]

        self._recognizer = sherpa_onnx.OfflineRecognizer.from_whisper(
            encoder=encoder,
            decoder=decoder,
            tokens=tokens,
            num_threads=ncpu,
            provider="cuda" if device == "cuda" else "cpu",
        )

    def __call__(self, inference_input: Dict) -> Dict:
        """Run STT inference.

        :param inference_input: Dict with 'query' (audio bytes or numpy array)
        :returns: Dict with 'output' (transcribed text)
        """
        audio_data = inference_input["query"]

        # Convert bytes to float32 numpy array
        if isinstance(audio_data, (bytes, bytearray)):
            audio_np = (
                np.frombuffer(audio_data, dtype=np.int16).astype(np.float32) / 32768.0
            )
        elif isinstance(audio_data, np.ndarray):
            audio_np = audio_data.astype(np.float32)
        else:
            return {"output": ""}

        # Ensure 1D array at 16kHz
        if audio_np.ndim > 1:
            audio_np = audio_np.flatten()

        stream = self._recognizer.create_stream()
        stream.accept_waveform(self._sample_rate, audio_np)
        self._recognizer.decode_stream(stream)

        return {"output": stream.result.text.strip()}
