from agents.components import MLLM, SpeechToText, TextToSpeech
from agents.config import SpeechToTextConfig, TextToSpeechConfig, MLLMConfig
from agents.clients import OllamaClient, RoboMLWSClient
from agents.models import Whisper, SpeechT5, OllamaModel
from agents.ros import Topic, Launcher

audio_in = Topic(name="audio0", msg_type="Audio")
text_query = Topic(name="text0", msg_type="String")

whisper = Whisper(name="whisper")  # Custom model init params can be provided here
roboml_whisper = RoboMLWSClient(whisper)

s2t_config = SpeechToTextConfig(
    enable_vad=True,  # option to listen for speech through the microphone
    # enable_wakeword=True,  # option to invoke the component with a wakeword like 'hey jarvis'
)
speech_to_text = SpeechToText(
    inputs=[audio_in],
    outputs=[text_query],
    model_client=roboml_whisper,
    trigger=audio_in,
    config=s2t_config,
    component_name="speech_to_text",
)

image0 = Topic(name="image_raw", msg_type="Image")
text_answer = Topic(name="text1", msg_type="String")

qwen_vl = OllamaModel(name="qwen_vl", checkpoint="qwen2.5vl:latest")
qwen_client = OllamaClient(qwen_vl)
mllm_config = MLLMConfig(
    stream=True
)  # Other inference specific paramters can be provided here

mllm = MLLM(
    inputs=[text_query, image0],
    outputs=[text_answer],
    model_client=qwen_client,
    trigger=text_query,
    config=mllm_config,
    component_name="vqa",
)

# config for asynchronously playing audio on device
t2s_config = TextToSpeechConfig(
    play_on_device=True, stream=True
)  # Set play_on_device to false if using the web UI

# Uncomment the following line for receiving output on the web UI
# audio_out = Topic(name="audio_out", msg_type="Audio")

speecht5 = SpeechT5(name="speecht5")
roboml_speecht5 = RoboMLWSClient(speecht5)
text_to_speech = TextToSpeech(
    inputs=[text_answer],
    outputs=[],  # use outputs=[audio_out] for web UI
    trigger=text_answer,
    model_client=roboml_speecht5,
    config=t2s_config,
    component_name="text_to_speech",
)

launcher = Launcher()
launcher.enable_ui(inputs=[audio_in, text_query], outputs=[image0])  # specify topics
launcher.add_pkg(components=[speech_to_text, mllm, text_to_speech])
launcher.bringup()
