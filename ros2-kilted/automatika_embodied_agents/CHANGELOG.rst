^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package automatika_embodied_agents
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.6.0 (2026-03-21)
------------------
* (docs) Adds advanced guides
* (docs) Adds developer docs
* (refactor) Adds sherpa-onnx for stt and tts local models
* (refactor) Adds ggml based local llm and vlm models with llama cpp
* (feature) Adds config option to strip think tokens
* (fix) Fixes execution provider setup when executor is set to be cpu
* (refactor) Standardizes local model handling in vision model
* (docs) Updates fallback example and adds new local model fallback recipe
* (feature) Adds fallback_to_local as a model component action for switching to local model on the fly
* (chore) Adds markers for running tests that require model serving
* (chore) Adds comprehensive test coverage for components
* (fix) Fixes bug in model components
* (fix) Fixes init of local LLM in semantic router for multiprocessing setup
* (feature) Adds local models for STT and TTS
* (feature) Enables local llm in semantic router component
* (feature) Adds moondream as local vlm for VLM component
* (feature) Adds local llm model for LLM component
* (fix) Adds extra check in rgbd callback
* (refactor) Adds warning for timed components when calling take_picture or record_video
* (docs) Updates vlm based planning recipe
* Contributors: ahr, mkabtoul

0.5.1 (2026-02-16)
------------------
* (feature) Adds a record_video action to the Vision component
* Takes specific input topic name and duration to record video
* (feature) Adds a take_picture action to the vision component
* (feature) Adds arbitrary action execution to router
* (feature) Adds handling of topic and action lists in semantic router
* (refactor) Makes action methods in Vision component use extra_callback to avoid starvation
* (refactor) Increases default max new tokens param for llm/vlm configs
* (refactor) Makes changes for new api of events/actions in sugar
* (fix) Fixes semantic router handling of action routes based on new api
* (fix) Fixes warmup for model components
* (chore) Adds think option for ollama models
* (chore) Adds proper docstrings to exposed methods and classes
* (docs) Updates model definitions in examples
* (docs) Adds new prompt for llms.txt and new tutorial intros
* (docs) Fix incorrect import in docs
* (docs) Updates installation instructions
* Contributors: ahr, mkabtoul

0.5.0 (2026-01-19)
------------------
* (docs) Adds supported types table and component runtype in basics
* (docs) Adds VLA tutorial to docs and refactors them
* (docs) Adds tutorials and examples for using events/fallbacks
* (refactor) Enables health broadcast by default
* (feautre) Adds feature for passing additional model clients to components
  Component can switch model clients based on events
* (feature) Adds external events as component triggers for certain components that can be run in a timed way
* (feature) Adds stt and tts endpoints and generic model wrappers for the generic http client (openai compatible)
* (feature) Enables support for tool calls in generic http client
* (feature) Adds llms.txt in docs
* (docs) Adds explanation of both routing modes to semantic router docs
* (feature) Adds LLM based agentic routing to semantic router
* (feature) Adds timeout for event based termination if the event doesnt happen within max_timesteps
* (feature) Create observation spec form mapping dicts if dataset info.json is not provided
* (feature) Adds publisher for joint state msg
* (feature) Adds parameter to regulate observation sending rate
* (fix) Adds various fixes for running in multiprocessing (vla)
* (feature) Adds dynamically adjusting loop rate to action server (vla)
* (feature) Adds termination logic to the VLA action server
* (feature) Adds capping of actions based on joint limits before publishing
* (feature) Adds action sending in VLA for multiple datatypes
* (feature) Adds a setter for passing an external aggregation function
* (feature) Adds mapping between LeRobot dataset keys and robot urdf and topics
  - Adds warning and errors for mapped keys, topics
  - Adds creation of input for VLA
  - Adds action receiving and aggregation
* (feature) Adds utility for reading joint limits from URDF files/urls
  - Adds mapping utility function in internal joint state class
* (fix) Adds destruction of stream timer if its created for async websocket clients
* (docs) Adds documentaion for lerobot client
* (feature) Adds an async grpc client for lerobot
* (docs) Removes onnxruntime as a mandatory dependancy, its only required for local models
* (feature) Adds LeRobotPolicy wrapper in models
  Adds utility for extracting features and actions from info.json
* (feature) Implements replacing input topics to handle trigger callbacks
* (feature) Enables None triggers for Server components
* (fix) Fixes unnecessary logging causing error before component activation
* (fix) Fixes deserialization of map layers and routes in multiprocess run
* Contributors: ahr, mkabtoul

0.4.3 (2025-11-07)
------------------
* (docs) Adds instructions for using the dynamic web UI
* (chore) Removes tiny web client
* (fix) Removes StreamingString as input option for LLM/MLLM
* (feature) Adds logging output from StreamingString in the UI
* (chore) Adds warning for using templates with StreamingString
* (chore) Adds alias VLM for MLLM component
* (fix) Adds detections as handled output in mllm component
* (feature) Adds callback for points of interest msg
* (feature) Adds ui callback for rgbd images
* (docs) Updates docs for planning model example
* (fix) Fixes publishing images as part of detection msgs
* (refactor) Updates callbacks for video and rgbd type messages
* (feature) Adds handling of additional types from other sugar derived packages in agent's components
* (feature) Adds UI element definitions for custom types
* (feature) Adds ui callbacks for Detections and DetectionsMultiSource
* (feature) Adds utility for drawing images with bounding boxes
* (fix) Fixes passing topic from sugar derived packages to agents components
* (feature) Adds Detection2D as allowed input in map encoding component
* (feature) Adds callback for Detections2D and their use in llm/mllm components
  - Streamlines names of detection msgs
  - Streamlines names of tracking msgs
* (fix) Gets raw msg data in execution step to avoid calling get_output twice
* (chore) Adds websockets as an explicit dependency
* Contributors: ahr, mkabtoul

0.4.2 (2025-09-03)
------------------
* (feature) Adds udp streaming to IP:PORT as an option to TextToStream component when play_on_device is enabled
* (docs) Updates docs to use new web based client
* (feature) Adds processing of audio messages in web client
* (chore) Removes chainlit based client
* (feature) Adds a custom webclient to replace chainlit
* (feature) Adds persistent ros node in web client for async stream handling
* (feature) Adds warning when not using streaming string msg_type with streaming enabled in components
* (feature) Adds streaming string msg for managing streams in external clients
* (docs) Adds recipe for vision guided point navigation
* (fix) Fixes empty image input for Detection2D msg publication
* (fix) Fixes websocket receiving in text to speech
* (fix) Fixes keyword argument in detection and tracking publishing
* (feature) Adds publishing a singular detection or tracking message from the vision component
* Contributors: ahr, mkabtoul

0.4.1 (2025-07-10)
------------------
* (docs) Updates docs for using planning based MLLMs
* (feature) Adds options to get RGBD array from rgbd message callback
* (refactor) Breaks complex functions and fixes warmup result logging
* (feature) Adds support for planning mllm models, starting with robobrain2.0
* (docs) Adds streaming to conversational agent example
* Contributors: ahr, mkabtoul

0.4.0 (2025-06-18)
------------------
* (docs) Adds international readme files
* (feature) Adds better connection error messages in clients, adds installation instructions
* (chore) Adds debian packaging workflow
* (docs) Updates installation instructions
* (chore) Updates package names .. ROS Agents -> EmbodiedAgents
* (feature) Adds a GenericHTTPClient for using llm and mllm models served on any OpenAI compatible API
* (feature) Adds ollama specific inference options to OllamaModel and client
* (feature) Adds MeloTTS model to model definitions
* (feature) Adds say text method to text to speech for invoking with events
* (feature) Adds streaming playback for streaming input in speeech to text component
* (fix) Fixes clearing old output in the vision component when getting subscription data in a timed manner
* (feature) Adds tensorrt as an onnx provider option for local models
* (refactor) Removes sounddevice as a dependancy for text to speech component
* (feature) Adds local classification model for Vision component
  Default model: DEIM: DETR with Improved Matching for Fast Convergence by Huang et al.
* (feature) Adds warnings if device for local models is set to GPU and runtime is not available
* (feature) Adds hypothesis buffer for publishing confirmed transcripts when using streaming
* (feature) Adds asynchronous receiving for streaming websockets client in speech to text component
* (refactor) Adds getting inference params just once during node configuration
* (fix) Fixes handling of model init params and sending np arrays during inference
* (feature) Adds asynchronous publishing of response in LLM component when streaming with websocket client
* (feature) Adds local embeddings option using sentence-transformers to ChromaDB client
* (feature) Adds ChromaDB http client with ollama embeddigs
* (feature) Adds streaming with websocket client in llm component
* (fix) Fixes error message for required topics when they can be either/or
* (feature) Adds support for RGBD messages (in realsense style)
* (feature) Adds async websocket client for roboml
* (refactor) Marks child threads as daemons for smoother termination
* (feature) Adds break_character to llm component config to handle breaking streaming output into chunks for publishing
* (feature) Adds streaming to roboml http client for text data
* (feature) Adds streaming output handling to ollama client
* (refactor) Adds set_system_prompt to components and removes it from model config
  The same model can be called with various system prompts by different components
* (fix) Fixes typing bugs for for python 3.8 compatibility
* Contributors: ahr, aleph-ra, mkabtoul

0.3.3 (2025-01-28)
------------------
* (fix) Removes python dependencies from package manifest until package names merged in rosdistro
* Contributors: ahr

0.3.2 (2025-01-28)
------------------
* (docs) Updates docs for conversational agent and SpeechToTextConfig
* (feature) Adds vad, audio feautres and wakeword classification classes based local onnx models
* (feature) Adds utility function for downloading models and status classes for speech processing
* (feature) Adds configuration for wakeword detections in speechtotext component
* (fix) Fixes error in ollama client where tool calls are received without output content
* (fix) Adds a fix to map encoding where it can start with a single detections layer
* (refactor) Makes component name non-optional in components to avoid name conflicts
* (fix) Fixes error for long prompts when checking if prompt is a filename
* (refactor) Removes pytorch as a dependency and runs VAD model with onnxruntime
* (refactor) Makes warmup a property of model components that defaults to false
* (feature) Adds utility method to download onnx model files
* (refactor) Replaces info with debug to reduce logging spam
* (fix) Fixes getting logging severity level for jazzy onwards
* (fix) Adds minor improvements to branching for llm and mllm components
* (chore) Cleansup dependencies for packaging
* (chore) Adds dependency for sugar and removes unnecessary python dependencies from packaging
* (fix) Corrects import of Topic class
* (docs) Removes redefinition of Topic and corrects links to ROS Sugar
* (fix) Changes topic in base component to be directly inherited from ROS Sugar for consistency accross packages
* (feature) Adds warmup functions to all model based components
* (refactor) Removes pillow as a dependancy
* (refactor) Removes overrrides from components and adds custom meathods instead
* (feature) Adds warmup to vision component for displaying stats on init
* (fix) Adds fix for correct colors in cv2 visualization
* (fix) Adds node name as window name for visualization in vision component
* (feature) Adds cv2 based visualization option to vision component
* (refactor) Reduces branching in execution step for components
* (chore) Combines agents and agents_interfaces to one package
* (chore) Changes deb package name
* (fix) Fixes raising error in model initialization for roboml clients
* (refactor) Adds passing additional agent types to ros sugar
* (fix) Fixes error messages when wrong component inputs/outputs are passed
* (feature) Adds support for CompressedImage msg type in components
* (feature) Adds option to deploy vision models using tensorrt
  Works with roboml
* (fix) Fixes check on sufficient topics in component validation
* (fix) Fixes a bug in topic validation
* (fix) Fixes validation of topics in components
* (refactor) Changes handling of image messages for publication
  - Adds support for CompressedImage messages
  - Gathers image messages directly in vision component instead of getting them back from clients
* (feature) Adds frame_id to trackings publisher and updates msg and callback
* (feature) Adds boxes to vision tracking message
* Contributors: ahr, mkabtoul

0.3.1 (2024-10-29)
------------------
* (chore) bump version 0.3.0 -> 0.3.1
* (feature) Adds support for using tool calling in LLM components in multiprocess execution
* Contributors: ahr

0.3.0 (2024-10-28)
------------------
* (chore) bump version 0.2.0 -> 0.3.0
* (chore) Adds bumpver config
* Merge pull request `#14 <https://github.com/automatika-robotics/ros-agents/issues/14>`_ from automatika-robotics/feature/external_processors
  Adds support for running components as separate processes
* (docs) Updates docs based on ROS Sugar version update
* (fix) Fixes bug in registering triggers with components
* (refactor) Simplifies by adding direct serialization of clients and triggers
* (refactor) Removes gratuitous logging from utils
* (fix) Minor bug fixes for components to run in multiprocessing
  - Fixes trigger assignment for components
  - Handles private attributes of attrs classes
  - Fixes component and config init in common executable
* (fix) Fixes serializing log level in clients
* (fix) Fixes minor bugs in utils, components, configs and models
* (feature) Adds support for running components in multiple processes
  - Adds common executable to the package for ROS Sugar launcher
  - Refactors components to be serializable
  - Adds serialization to clients
  - Minor type hint changes for compatibility with older versions of ROS
* (fix) Adds the correct check for external processors given new ros-sugar implementation
* Contributors: ahr

0.2.0 (2024-09-28)
------------------
* (chore) Bump up the version
* Merge pull request `#13 <https://github.com/automatika-robotics/ros-agents/issues/13>`_ from automatika-robotics/feature/better_clients
  Adds enhanced functionality in clients specifically for LLM and MLLM components
* (feature) Adds tool calling for LLM component using the OllamaClient
* (fix) Fixes rag results in templated inputs to LLMs which do not contain input
* (refactor) Makes named models subclasses of TransformersLLM and TransformersMLLM for easier handling in roboml client
* (fix) Fixes key error in ollama client response retreival
* (fix) Adds flag for chat history for chat history reset and fixes logging
* (feature) Adds TransformersLLM and TransformersMLLM models for roboml clients
* (fix) Removes history reset phrase from model definitions and add system prompt for LLMs and derivates
* (refactor) Changes model component to have execution step as an abstract method implemented by child components
* (fix) Changes ollama client inference call to use chat endpoint
* (feature) Adds chat history management to llm and mllm components
* (docs) Clarifies handling of RAG results for llm component
* (fix) Fixes bug in rag result handling for llm component
* (fix) Removes default init_timeout from models
* (refactor) Moves roboml resp client dependancies inside the client initialization
* (fix) Explicity exposes QoSConfig in ros module
* (refactor) Replaces map_meta_data parameter with map_topic for MapEncoding component
* (refactor) Removes direct dependancy on pypdf
* (fix) Changes map meta data topic to type OccupancyGrid
* (feature) Adds audio options to chainlit client
* (fix) Removes unused imports
* (fix) Fixes the initialization of map encoding and semantic router components
* (refactor) Fixes imports and refactors code according to latest version of ROS sugar
* (fix) Fixes passing the config in components to parent base component
* (fix) Fixes ROS sugar import for BaseTopic
* (refactor) Removes auto_ros as a dependency
* (feature) Adds init_on_activation flag to all implemented clientsc
* (feature) Seperates abstract methods from callable methods in db client base
* (feature) Seperates callable methods, from abstract methods in client base class
* Contributors: ahr

0.1.1 (2024-09-05)
------------------
* (feature) Adds component action for adding points to map collection (`#12 <https://github.com/automatika-robotics/ros-agents/issues/12>`_)
  * Makes version compliant with ROS convention
* (chore) Adds license declaration in setup.py
* Bumps version number and adds license information
* Initial release 0.1.1a
* Contributors: ahr, mkabtoul
