from agents.components import Vision, VLM
from agents.config import VisionConfig
from agents.clients import OllamaClient
from agents.models import OllamaModel
from agents.ros import Launcher, Topic, FixedInput, Event

# Define Topics
camera_image = Topic(name="/image_raw", msg_type="Image")
detections = Topic(name="/detections", msg_type="Detections")  # Output of Vision
description_output = Topic(name="/description", msg_type="String")  # Output of VLM

# Setup the Vision Component (The Trigger)
# We use a lower threshold to ensure we catch people easily and we use a small local model
vision_config = VisionConfig(threshold=0.6, enable_local_classifier=True)

vision_detector = Vision(
    inputs=[camera_image],
    outputs=[detections],
    trigger=camera_image,  # Runs on every frame
    config=vision_config,
    component_name="eye_detector",
)

# Define the Event
# This event listens to the 'detections' topic.
# It triggers ONLY if the "labels" list inside the message contains "person"
# after not containing a person (within a 5 second interval).
event_person_detected = Event(
    detections.msg.labels.contains_any(["person"]),
    on_change=True,  # Trigger only when a change has occurred to stop repeat triggering
    keep_event_delay=5,  # A delay in seconds
)

# Setup the VLM Component (The Responder)
# This component does NOT run continuously. It waits for the event.

# Setup a model client for the component
qwen_vl = OllamaModel(name="qwen_vl", checkpoint="qwen2.5vl:7b")
ollama_client = OllamaClient(model=qwen_vl)


# We define a fixed prompt that is injected whenever the component runs.
fixed_prompt = FixedInput(
    name="prompt",
    msg_type="String",
    fixed="A person has been detected. Describe their appearance briefly.",
)

visual_describer = VLM(
    inputs=[fixed_prompt, camera_image],  # Takes the fixed prompt + current image
    outputs=[description_output],
    model_client=ollama_client,
    trigger=event_person_detected,  # CRITICAL: Only runs when the event fires
    component_name="visual_describer",
)

# Launch
launcher = Launcher()
launcher.add_pkg(
    components=[vision_detector, visual_describer],
    multiprocessing=True,
    package_name="automatika_embodied_agents",
)
launcher.bringup()
