from agents.components import Vision, MLLM
from agents.models import VisionModel, TransformersMLLM
from agents.clients import RoboMLRESPClient, RoboMLHTTPClient
from agents.ros import Topic, Launcher
from agents.config import VisionConfig

image0 = Topic(name="image_raw", msg_type="Image")
detections_topic = Topic(name="detections", msg_type="Detections")

object_detection = VisionModel(
    name="object_detection", checkpoint="dino-4scale_r50_8xb2-12e_coco"
)
roboml_detection = RoboMLRESPClient(object_detection)

detection_config = VisionConfig(threshold=0.5)
vision = Vision(
    inputs=[image0],
    outputs=[detections_topic],
    trigger=image0,
    config=detection_config,
    model_client=roboml_detection,
    component_name="detection_component",
)

text_query = Topic(name="text0", msg_type="String")
text_answer = Topic(name="text1", msg_type="String")

idefics = TransformersMLLM(name="idefics_model", checkpoint="HuggingFaceM4/idefics2-8b")
idefics_client = RoboMLHTTPClient(idefics)

mllm = MLLM(
    inputs=[text_query, image0, detections_topic],
    outputs=[text_answer],
    model_client=idefics_client,
    trigger=text_query,
    component_name="mllm_component",
)

mllm.set_component_prompt(
    template="""Imagine you are a robot.
    This image has following items: {{ detections }}.
    Answer the following about this image: {{ text0 }}"""
)
launcher = Launcher()
launcher.add_pkg(components=[vision, mllm])
launcher.bringup()
