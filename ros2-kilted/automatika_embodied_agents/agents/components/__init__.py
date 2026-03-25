"""
A Component is the main execution unit in _EmbodiedAgents_ and in essence each component is synctactic sugar over a ROS2 Lifecycle Node. EmbodiedAgents provides the following components. These components can be arbitrarily combined to form an embodied agent graph.

```{list-table}
:widths: 20 80
:header-rows: 1
* - Component Name
  - Description

* - **[LLM](agents.components.llm.md)**
  - Uses large language models (e.g., LLaMA) to process text input. Can be used for reasoning, tool calling, instruction following, or dialogue. It can also utilize vector DBs for storing and retreiving contextual information.

* - **[VLM](agents.components.mllm.md)**
  - Leverages multimodal LLMs (e.g., Llava) for understanding and processing both text and image data. Inherits all functionalities of the LLM component.

* - **[VLA](agents.components.vla.md)**
  - Provides an interface to utilize Vision Language Action (VLA) models for manipulation and control tasks. It can use VLA Policies (such as SmolVLA, Pi0 etc.) served with HuggingFace LeRobot Async Policy Server and publish them to common topic formats in MoveIt Servo and ROS2 Control.

* - **[SpeechToText](agents.components.speechtotext.md)**
  - Converts spoken audio into text using speech-to-text models (e.g., Whisper). Suitable for voice command recognition. It also implements small on-board models for Voice Activity Detection (VAD) and Wakeword recognition, using audio capture devices onboard the robot.

* - **[TextToSpeech](agents.components.texttospeech.md)**
  - Synthesizes audio from text using TTS models (e.g., SpeechT5, Bark). Output audio can be played using the robot's speakers or published to a topic. Implements `say(text)` and `stop_playback` functions to play/stop audio based on events from other components or the environment.

* - **[MapEncoding](agents.components.map_encoding.md)**
  - Provides a spatio-temporal working memory by converting semantic outputs (e.g., from MLLMs or Vision) into a structured map representation. Uses robot localization data and output topics from other components to store information in a vector DB.

* - **[SemanticRouter](agents.components.semantic_router.md)**
  - Routes information between topics based on semantic content and predefined routing rules. Uses a vector DB for semantic matching or an LLM for decision-making. This allows for creating complex graphs of components where a single input source can trigger different information processing pathways.

* - **[Vision](agents.components.vision.md)**
  - An essential component in all vision powered robots. Performs object detection and tracking on incoming images. Outputs object classes, bounding boxes, and confidence scores. It implements a low-latency small on-board classification model as well.

* - **[VideoMessageMaker](agents.components.imagestovideo.md)**
  - This component generates ROS video messages from input image messages. A video message is a collection of image messages that have a perceivable motion. I.e. the primary task of this component is to make intentionality decisions about what sequence of consecutive images should be treated as one coherent temporal sequence. The chunking method used for selecting images for a video can be configured in component config. It can be useful in sending videos to ML models that take image sequences.
```
"""

from .component_base import Component
from .imagestovideo import VideoMessageMaker
from .llm import LLM
from .map_encoding import MapEncoding
from .mllm import MLLM, VLM
from .model_component import ModelComponent
from .semantic_router import SemanticRouter
from .speechtotext import SpeechToText
from .texttospeech import TextToSpeech
from .vision import Vision
from .vla import VLA

__all__ = [
    "Component",
    "ModelComponent",
    "MapEncoding",
    "MLLM",
    "VLM",
    "LLM",
    "VLA",
    "SpeechToText",
    "TextToSpeech",
    "Vision",
    "VideoMessageMaker",
    "SemanticRouter",
]
