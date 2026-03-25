"""
The following model specification classes are meant to define a comman interface for initialization parameters for ML models across supported model serving platforms.
"""

from typing import Optional, Dict, Any, Literal, List
from attrs import define, field, validators
from .ros import BaseAttrs, base_validators
from .utils import build_lerobot_features_from_dataset_info, _LANGUAGE_CODES

__all__ = [
    "GenericLLM",
    "GenericMLLM",
    "GenericTTS",
    "GenericSTT",
    "TransformersLLM",
    "TransformersMLLM",
    "OllamaModel",
    "Whisper",
    "SpeechT5",
    "Bark",
    "MeloTTS",
    "VisionModel",
]


@define(kw_only=True)
class Model(BaseAttrs):
    """Model configuration base class"""

    name: str
    checkpoint: str
    init_timeout: Optional[int] = field(default=None)

    def get_init_params(self) -> Dict:
        """Get init params from models"""
        return self._get_init_params()

    def _get_init_params(self) -> Dict:
        raise NotImplementedError(
            "_get_init_params method needs to be implemented by model definition classes"
        )


@define(kw_only=True)
class LLM(Model):
    """LLM/MLLM model configurations base class.

    :param name: An arbitrary name given to the model.
    :type name: str
    :type quantization: str or None
    :param init_timeout: The timeout in seconds for the initialization process. Defaults to None.
    :type init_timeout: int, optional
    """

    quantization: Optional[str] = field(
        default="4bit", validator=base_validators.in_(["4bit", "8bit", None])
    )

    def _get_init_params(self) -> Dict:
        """Get init params for model initialization."""
        return {
            "checkpoint": self.checkpoint,
            "quantization": self.quantization,
        }


@define(kw_only=True)
class GenericLLM(Model):
    """
    A generic LLM configuration for OpenAI-compatible /v1/chat/completions APIs.

    This class supports any model served via an OpenAI-compatible endpoint (e.g., vLLM,
    LMDeploy, DeepSeek, Groq, or OpenAI itself).

    :param name: An arbitrary name given to the model.
    :type name: str
    :param checkpoint: The model identifier on the remote server (e.g., "gpt-4o", "meta-llama/Llama-3-70b").
               For OpenAI models, consult: https://platform.openai.com/docs/models
    :type checkpoint: str
    :param init_timeout: The timeout in seconds for the initialization process. Defaults to None.
    :type init_timeout: int, optional
    :param options: Optional dictionary to configure default inference behavior. Options that conflict with component config options such as (max_tokens and temperature) will be overridden if set in component config.
                    Supported keys match standard OpenAI API parameters:
                    - temperature (float): Sampling temperature (0-2).
                    - top_p (float): Nucleus sampling probability.
                    - max_tokens (int): Max tokens to generate.
                    - presence_penalty (float): Penalty for new tokens (-2.0 to 2.0).
                    - frequency_penalty (float): Penalty for frequent tokens (-2.0 to 2.0).
                    - stop (str or list): Stop sequences.
                    - seed (int): Random seed for deterministic sampling.
    :type options: dict, optional

    Example usage:
    ```python
    gpt4 = GenericLLM(
        name='gpt4',
        checkpoint="gpt-4o",
        options={"temperature": 0.7, "max_tokens": 500}
    )
    ```
    """

    checkpoint: str = field(default="gpt-4o")
    options: Optional[Dict[str, Any]] = field(default=None)

    @options.validator
    def _validate_options(self, _, value):
        if value is None:
            return
        allowed_keys = {
            "temperature": float,
            "top_p": float,
            "max_tokens": int,
            "presence_penalty": float,
            "frequency_penalty": float,
            "stop": List,
            "seed": int,
        }

        for key, val in value.items():
            if key not in allowed_keys:
                raise ValueError(f"Invalid key in options: {key}")
            expected_type = allowed_keys[key]
            if key == "stop":
                if not isinstance(val, list) or not all(
                    isinstance(item, str) for item in val
                ):
                    raise TypeError(f"Value for key '{key}' must be a list of strings")
            elif not isinstance(val, expected_type):
                raise TypeError(
                    f"Value for key '{key}' must be of type {expected_type.__name__}"
                )

    def _get_init_params(self) -> Dict:
        return {"checkpoint": self.checkpoint, "options": self.options}


@define(kw_only=True)
class GenericMLLM(GenericLLM):
    """
    A generic Multimodal LLM configuration for OpenAI-compatible APIs.

    Use this for models that accept image/audio inputs alongside text (e.g., GPT-4o,
    Claude 3.5 Sonnet via wrapper, Gemini via OpenAI adapter).

    :param name: An arbitrary name given to the model.
    :type name: str
    :param checkpoint: The model identifier. Consult provider documentation.
    :type checkpoint: str
    :param options: Optional dictionary for default inference parameters (see GenericLLM).
    :type options: dict, optional

    Example usage:
    ```python
    gpt4_vision = GenericMLLM(name='gpt4v', checkpoint="gpt-4o")
    ```
    """

    checkpoint: str = field(default="gpt-4o")


@define(kw_only=True)
class GenericTTS(Model):
    """
    A generic Text-to-Speech model for OpenAI-compatible /v1/audio/speech APIs.

    :param name: An arbitrary name given to the model.
    :type name: str
    :param checkpoint: The model identifier (e.g., "tts-1", "tts-1-hd").
                       For details: https://platform.openai.com/docs/models/tts
    :type checkpoint: str
    :param voice: The voice ID to use. OpenAI standard voices: 'alloy', 'echo', 'fable',
                  'onyx', 'nova', 'shimmer'. Other providers may have different IDs.
    :type voice: str
    :param speed: The speed of the generated audio. Select a value from 0.25 to 4.0. Default is 1.0.
    :type speed: float
    :param init_timeout: The timeout in seconds for the initialization process. Defaults to None.
    :type init_timeout: int, optional

    Example usage:
    ```python
    tts = GenericTTS(
        name='openai_tts',
        checkpoint="tts-1-hd",
        voice="nova",
        speed=1.2
    )
    ```
    """

    checkpoint: str = field(default="tts-1")
    voice: str = field(default="alloy")
    speed: float = field(
        default=1.0, validator=base_validators.in_range(min_value=0.25, max_value=4.0)
    )

    def _get_init_params(self) -> Dict:
        return {
            "checkpoint": self.checkpoint,
            "voice": self.voice,
            "speed": self.speed,
        }


@define(kw_only=True)
class GenericSTT(Model):
    """
    A generic Speech-to-Text model for OpenAI-compatible /v1/audio/transcriptions APIs.

    :param name: An arbitrary name given to the model.
    :type name: str
    :param checkpoint: The model identifier (e.g., "whisper-1").
                       For details: https://platform.openai.com/docs/models/whisper
    :type checkpoint: str
    :param language: The language of the input audio (ISO-639-1 format, e.g., 'en', 'fr').
                     Improves accuracy if known. Default is None (auto-detect).
    :type language: str, optional
    :param temperature: The sampling temperature (0-1). Lower values are more deterministic. Default is 0.
    :type temperature: float
    :param init_timeout: The timeout in seconds for the initialization process. Defaults to None.
    :type init_timeout: int, optional

    Example usage:
    ```python
    stt = GenericSTT(
        name='openai_stt',
        checkpoint="whisper-1",
        language="en",
        temperature=0.2
    )
    ```
    """

    checkpoint: str = field(default="whisper-1")
    temperature: float = field(default=0.0)
    language: Optional[str] = field(
        default=None,
        validator=validators.optional(base_validators.in_(_LANGUAGE_CODES)),
    )

    def _get_init_params(self) -> Dict:
        return {
            "checkpoint": self.checkpoint,
            "language": self.language,
            "temperature": self.temperature,
        }


@define(kw_only=True)
class OllamaModel(LLM):
    """An Ollama model that needs to be initialized with an ollama tag as checkpoint.

    :param name: An arbitrary name given to the model.
    :type name: str
    :param checkpoint: The name of the pre-trained model's checkpoint. For available checkpoints consult [Ollama Models](https://ollama.com/library)
    :type checkpoint: str
    :param init_timeout: The timeout in seconds for the initialization process. Defaults to None.
    :type init_timeout: int, optional
    :param options: Optional dictionary to configure generation behavior. Options that conflict with component config options such as (num_predict and temperature) will be overridden if set in component config. Only the following keys with their specified value types are allowed. For details check [Ollama api documentation](https://github.com/ollama/ollama/blob/main/docs/api.md#generate-request-with-options):
        - num_keep: int
        - seed: int
        - num_predict: int
        - top_k: int
        - top_p: float
        - min_p: float
        - typical_p: float
        - repeat_last_n: int
        - temperature: float
        - repeat_penalty: float
        - presence_penalty: float
        - frequency_penalty: float
        - penalize_newline: bool
        - stop: list of strings
        - numa: bool
        - num_ctx: int
        - num_batch: int
        - num_gpu: int
        - main_gpu: int
        - use_mmap: bool
        - num_thread: int
        - think: bool
    :type options: dict, optional

     Example usage:
    ```python
    llm = OllamaModel(
        name='ollama1',
        checkpoint="gemma2:latest",
        options={"temperature": 0.7, "num_predict": 50}
    )
    ```"""

    checkpoint: str = field(default="llama3.2:3b")
    port: Optional[int] = field(default=11434)
    options: Optional[Dict[str, Any]] = field(default=None)

    @options.validator
    def _validate_options(self, _, value):
        if value is None:
            return

        allowed_keys = {
            "num_keep": int,
            "seed": int,
            "num_predict": int,
            "top_k": int,
            "top_p": float,
            "min_p": float,
            "typical_p": float,
            "repeat_last_n": int,
            "temperature": float,
            "repeat_penalty": float,
            "presence_penalty": float,
            "frequency_penalty": float,
            "penalize_newline": bool,
            "stop": List,
            "numa": bool,
            "num_ctx": int,
            "num_batch": int,
            "num_gpu": int,
            "main_gpu": int,
            "use_mmap": bool,
            "num_thread": int,
            "think": bool,
        }

        for key, val in value.items():
            if key not in allowed_keys:
                raise ValueError(f"Invalid key in options: {key}")
            expected_type = allowed_keys[key]
            if key == "stop":
                if not isinstance(val, list) or not all(
                    isinstance(item, str) for item in val
                ):
                    raise TypeError(f"Value for key '{key}' must be a list of strings")
            elif not isinstance(val, expected_type):
                raise TypeError(
                    f"Value for key '{key}' must be of type {expected_type.__name__}"
                )

    def _get_init_params(self) -> Dict:
        """Get init params for model initialization."""
        return {
            "checkpoint": self.checkpoint,
            "options": self.options,
        }


@define(kw_only=True)
class TransformersLLM(LLM):
    """An LLM model that needs to be initialized with any LLM checkpoint available on HuggingFace transformers. This model can be used with a roboml client.

    :param name: An arbitrary name given to the model.
    :type name: str
    :param checkpoint: The name of the pre-trained model's checkpoint. Default is "microsoft/Phi-3-mini-4k-instruct". For available checkpoints consult [HuggingFace LLM Models](https://huggingface.co/models?other=LLM)
    :type checkpoint: str
    :param quantization: The quantization scheme used by the model. Can be one of "4bit", "8bit" or None (default is "4bit").
    :type quantization: str or None
    :param init_timeout: The timeout in seconds for the initialization process. Defaults to None.
    :type init_timeout: int, optional

    Example usage:
    ```python
    llm = TransformersLLM(name='llm', checkpoint="meta-llama/Meta-Llama-3.1-8B-Instruct")
    ```
    """

    checkpoint: str = field(default="microsoft/Phi-3-mini-4k-instruct")


@define(kw_only=True)
class TransformersMLLM(TransformersLLM):
    """An MLLM model that needs to be initialized with any MLLM checkpoint available on HuggingFace transformers. This model can be used with a roboml client.

    :param name: An arbitrary name given to the model.
    :type name: str
    :param checkpoint: The name of the pre-trained model's checkpoint. Default is "HuggingFaceM4/idefics2-8b". For available checkpoints consult [HuggingFace Image-Text to Text Models](https://huggingface.co/models?pipeline_tag=image-text-to-text)
    :type checkpoint: str
    :param quantization: The quantization scheme used by the model. Can be one of "4bit", "8bit" or None (default is "4bit").
    :type quantization: str or None
    :param init_timeout: The timeout in seconds for the initialization process. Defaults to None.
    :type init_timeout: int, optional

    Example usage:
    ```python
    mllm = TransformersMLLM(name='mllm', checkpoint="gemma2:latest")
    ```
    """

    checkpoint: str = field(default="HuggingFaceM4/idefics2-8b")


@define(kw_only=True)
class RoboBrain2(Model):
    """[RoboBrain 2.0 by BAAI](https://github.com/FlagOpen/RoboBrain2.0) supports interactive reasoning with long-horizon planning and closed-loop feedback, spatial perception for precise point and bbox prediction from complex instructions and temporal perception for future trajectory estimation.
        @article{RoboBrain2.0TechnicalReport,
        title={RoboBrain 2.0 Technical Report},
        author={BAAI RoboBrain Team},
        journal={arXiv preprint arXiv:2507.02029},
        year={2025}
    }
    :param name: An arbitrary name given to the model.
    :type name: str
    :param checkpoint: The name of the pre-trained model's checkpoint. Default is "BAAI/RoboBrain2.0-7B". For available checkpoints consult [RoboBrain2 Model Collection](https://huggingface.co/collections/BAAI/robobrain20-6841eeb1df55c207a4ea0036) on HuggingFace.
    :type checkpoint: str
    :param init_timeout: The timeout in seconds for the initialization process. Defaults to None.
    :type init_timeout: int, optional

    Example usage:
    ```python
    robobrain = RoboBrain2(name='robobrain', checkpoint="BAAI/RoboBrain2.0-32B")
    ```
    """

    checkpoint: str = field(default="BAAI/RoboBrain2.0-7B")

    def _get_init_params(self) -> Dict:
        """Get init params for model initialization."""
        return {
            "checkpoint": self.checkpoint,
        }


@define(kw_only=True)
class Whisper(Model):
    """Whisper is an automatic speech recognition (ASR) system by OpenAI trained on 680,000 hours of multilingual and multitask supervised data collected from the web. [Details](https://openai.com/index/whisper/)

    :param name: An arbitrary name given to the model.
    :type name: str
    :param checkpoint: Size of the model to use (tiny, tiny.en, base, base.en, small, small.en, distil-small.en, medium, medium.en, distil-medium.en, large-v1, large-v2, large-v3, large, distil-large-v2, distil-large-v3, large-v3-turbo, or turbo). For more information check [here](https://github.com/SYSTRAN/faster-whisper/blob/d3bfd0a305eb9d97c08047c82149c1998cc90fcb/faster_whisper/transcribe.py#L606)
    :type checkpoint: str
    :param compute_type: The compute type used by the model. Can be one of "int8", "fp16", "fp32", None (default is "int8").
    :type compute_type: str or None
    :param init_timeout: The timeout in seconds for the initialization process. Defaults to None.
    :type init_timeout: int, optional

    Example usage:
    ```python
    whisper = Whisper(name='s2t', checkpoint="small") # Initialize with a different checkpoint
    ```
    """

    checkpoint: str = field(default="small.en")
    compute_type: Optional[str] = field(
        default="int8",
        validator=base_validators.in_(["int8", "float16", "float32", None]),
    )

    def _get_init_params(self) -> Dict:
        """Get init params for model initialization."""
        return {"checkpoint": self.checkpoint, "compute_type": self.compute_type}


@define(kw_only=True)
class SpeechT5(Model):
    """A model for text-to-speech synthesis developed by Microsoft. [Details](https://github.com/microsoft/SpeechT5)

    :param name: An arbitrary name given to the model.
    :type name: str
    :param checkpoint: The name of the pre-trained model's checkpoint. Default is "microsoft/speecht5_tts".
    :type checkpoint: str
    :param voice: The voice to use for synthesis. Can be one of "awb", "bdl", "clb", "jmk", "ksp", "rms", or "slt". Default is "clb".
    :param init_timeout: The timeout in seconds for the initialization process. Defaults to None.
    :type init_timeout: int, optional

    Example usage:
    ```python
    speecht5 = SpeechT5(name='t2s1', voice="bdl")  # Initialize with a different voice
    ```
    """

    checkpoint: str = field(default="microsoft/speecht5_tts")
    voice: str = field(
        default="clb",
        validator=base_validators.in_([
            "awb",
            "bdl",
            "clb",
            "jmk",
            "ksp",
            "rms",
            "slt",
        ]),
    )

    def _get_init_params(self) -> Dict:
        """Get init params for model initialization."""
        return {"checkpoint": self.checkpoint, "voice": self.voice}


@define(kw_only=True)
class Bark(Model):
    """A model for text-to-speech synthesis developed by SunoAI. [Details](https://github.com/suno-ai/bark)

    :param name: An arbitrary name given to the model.
    :type name: str
    :param checkpoint: The name of the pre-trained model's checkpoint. [Bark checkpoints on HuggingFace](https://huggingface.co/collections/suno/bark-6502bdd89a612aa33a111bae). Default is "suno/bark-small".
    :type checkpoint: str
    :param attn_implementation: The attention implementation to use for the model. Default is "flash_attention_2".
    :param voice: The voice to use for synthesis. More choices are available [here](https://suno-ai.notion.site/8b8e8749ed514b0cbf3f699013548683?v=bc67cff786b04b50b3ceb756fd05f68c). Default is "v2/en_speaker_6".
    :param init_timeout: The timeout in seconds for the initialization process. Defaults to None.
    :type init_timeout: int, optional

    Example usage:
    ```python
    bark = Bark(name='t2s2', voice="v2/en_speaker_1")  # Initialize with a different voice
    ```
    """

    checkpoint: str = field(default="suno/bark-small")
    voice: str = field(default="v2/en_speaker_6")

    def _get_init_params(self) -> Dict:
        """Get init params for model initialization."""
        return {
            "checkpoint": self.checkpoint,
            "voice": self.voice,
        }


@define(kw_only=True)
class MeloTTS(Model):
    """A model for text-to-speech synthesis developed by MyShell AI using the MeloTTS engine.

    :param name: An arbitrary name given to the model.
    :type name: str
    :param language: The language for speech synthesis. Supported values: ["EN", "ES", "FR", "ZH", "JP", "KR"]. Default is "EN".
    :type language: str
    :param speaker_id: The speaker ID for the chosen language. Default is "EN-US". For details check [here](https://github.com/myshell-ai/MeloTTS/blob/main/docs/install.md#python-api)
    :type speaker_id: str
    :param init_timeout: The timeout in seconds for the initialization process. Defaults to None.
    :type init_timeout: int, optional

    Example usage:
    ```python
    melotts = MeloTTS(name='melo1', language='JP', speaker_id='JP-1')
    ```
    """

    language: str = field(default="EN")
    speaker_id: str = field(default="EN-US")

    def _get_init_params(self) -> Dict:
        """Get init params for model initialization."""
        return {
            "language": self.language,
            "speaker_id": self.speaker_id,
        }


@define(kw_only=True)
class VisionModel(Model):
    """Object Detection Model with Optional Tracking.

    This vision model provides a flexible framework for object detection and tracking using the [mmdet framework](https://github.com/open-mmlab/mmdetection). It can be used as a standalone detector or as a tracker to follow detected objects over time. It can be initizaled with any checkpoint available in the mmdet framework.

    :param name: An arbitrary name given to the model.
    :type name: str
    :param checkpoint: The name of the pre-trained model's checkpoint. [All available checkpoints in the mmdet framework](https://github.com/open-mmlab/mmdetection?tab=readme-ov-file#overview-of-benchmark-and-model-zoo). Default is "dino-4scale_r50_8xb2-12e_coco".
    :type checkpoint: str
    :param cache_dir: The directory where downloaded models are cached. Default is 'mmdet'.
    :type cache_dir: str
    :param setup_trackers: Whether to set up trackers using norfair or not. Default is False.
    :type setup_trackers: bool
    :param tracking_distance_function: The function used to calculate the distance between detected objects. This can be any distance metric string available in [scipy.spatial.distance.cdist](https://docs.scipy.org/doc/scipy/reference/generated/scipy.spatial.distance.cdist.html) Default is "euclidean".
    :type tracking_distance_function: str
    :param tracking_distance_threshold: The threshold for determining whether two object in consecutive frames are considered close enough to be considered the same object. Default is 30, with a minimum value of 1.
    :type tracking_distance_threshold: int
    :param deploy_tensorrt: Deploy the vision model using NVIDIA TensorRT. To utilize this feature with roboml, checkout the instructions [here](https://github.com/automatika-robotics/roboml). Default is False.
    :type deploy_tensorrt: bool
    :param _num_trackers: The number of trackers to use. This number depends on the number of inputs image streams being given to the component. It is set automatically if **setup_trackers** is True.
    :type _num_trackers: int
    :param init_timeout: The timeout in seconds for the initialization process. Defaults to None.
    :type init_timeout: int, optional

    Example usage:
    ```python
    model = DetectionModel(name='detection1', setup_trackers=True, num_trackers=1, tracking_distance_threshold=20)  # Initialize the model for tracking one object
    ```
    """

    checkpoint: str = field(default="dino-4scale_r50_8xb2-12e_coco")
    cache_dir: str = field(default="mmdet")
    setup_trackers: bool = field(default=False)
    tracking_distance_function: str = field(default="euclidean")
    tracking_distance_threshold: int = field(
        default=30, validator=base_validators.gt(0)
    )
    deploy_tensorrt: bool = field(default=False)
    _num_trackers: int = field(default=1, validator=base_validators.gt(0))

    def _get_init_params(self) -> Dict:
        """Get init params for model initialization."""
        return {
            "checkpoint": self.checkpoint,
            "cache_dir": self.cache_dir,
            "setup_trackers": self.setup_trackers,
            "num_trackers": self._num_trackers,
            "tracking_distance_function": self.tracking_distance_function,
            "tracking_distance_threshold": self.tracking_distance_threshold,
            "deploy_tensorrt": self.deploy_tensorrt,
        }


@define(kw_only=True)
class LeRobotPolicy(Model):
    """LeRobot Policy Model for Vision-Language-Action Robotics.

    This model provides a high-level interface for loading and running **LeRobot** policies— vision-language-action (VLA) models trained for robotic manipulation tasks.

    It supports automatic extraction of feature and action specifications directly from dataset metadata, as well as flexible configuration of policy behavior.

    The policy can be instantiated from any compatible **LeRobot** checkpoint hosted on
    HuggingFace, making it easy to load pretrained models such as `smolvla_base` or others from LeRobot. Upon initialization, the wrapper downloads and parses the dataset
    metadata file to derive the feature schema and action space that the policy expects.

    :param checkpoint:
        The name or HuggingFace repository ID of the pretrained LeRobot checkpoint to load.
        Default: `"lerobot/smolvla_base"`.
    :type checkpoint: str
    :param policy_type:
    The type of LeRobot policy to load.
    Supported values are:
    - `"diffusion"` — Diffusion-based action generation policy
    - `"act"` — Action Chunk Transformer policy
    - `"smolvla"` — General VLA from HuggingFace
    - `"pi0"` — General VLA from Physical Intelligence
    - `"pi05"` — General VLA from Physical Intelligence
    This field determines how the checkpoint is interpreted and which policy architecture is instantiated.
    Default: `"smolvla"`.
    :param dataset_info_file:
        URL or local path to the dataset metadata file (`info.json`).
        This file defines the input features and action structure that the policy must follow. Empty by default. If not provided, an attempt will be made by the VLA component to auto-generate it from the component config.
    :type dataset_info_file: str, optional
    :param policy_type:
    The device on which the server should initialize the policy.
    Supported values are:
    - `"cuda"` — NVIDIA GPU available on server
    - `"cpu"` — GPU not available on server
    Default: `"cuda"`.
    :param actions_per_chunk:
        The number of predicted actions produced per inference chunk. This is only applicable for certain policy types that implement Real Time Chunking (RTC) such as Pi0, and SmolVLA
        Default: `50`.
    :type actions_per_chunk: int
    :param init_timeout:
        Optional timeout (in seconds) for initialization.
        Default: `None`.
    :type init_timeout: int, optional

    ### Example
    ```python
    # Load a standard LeRobot policy
    policy = LeRobotPolicy(
        checkpoint="lerobot/smolvla_base",
        policy_type="smolvla"
    )

    # Load a policy with a different dataset specification
    policy = LeRobotPolicy(
        checkpoint="my_own/smolvla_finetuned",
        policy_type="smolvla"
        dataset_info_file="path/to/my_dataset/info.json",
        actions_per_chunk=25,
    )
    """

    checkpoint: str = field(default="lerobot/smolvla_base")
    policy_type: Literal["smolvla", "diffusion", "act", "pi0", "pi05"] = field(
        default="smolvla"
    )
    actions_per_chunk: int = field(default=50)
    policy_device: Literal["cpu", "cuda"] = field(default="cuda")
    dataset_info_file: Optional[str] = field(default=None)
    _features: Dict = field(default={})  # Created in the component if missing
    _actions: Optional[Dict] = field(default=None)
    _image_keys: Optional[List] = field(default=None)
    _joint_keys: Optional[List] = field(default=None)

    def __attrs_post_init__(self):
        if self.dataset_info_file:
            lerobot_features = build_lerobot_features_from_dataset_info(
                self.dataset_info_file
            )
            self._features = lerobot_features["features"]
            self._actions = lerobot_features["actions"]
            self._image_keys = lerobot_features["image_keys"]
            self._joint_keys = self._features["observation.state"]["names"]

    def _get_init_params(self):
        return {
            "checkpoint": self.checkpoint,
            "policy_type": self.policy_type,
            "features": self._features,
            "actions_per_chunk": self.actions_per_chunk,
            "device": self.policy_device,
        }
