from agents.components import LLM
from agents.models import OllamaModel, TransformersLLM
from agents.clients import OllamaClient, RoboMLHTTPClient
from agents.config import LLMConfig
from agents.ros import Launcher, Topic, Action

# Define the Models and Clients
# Primary: A powerful model hosted remotely (e.g., via RoboML)
# NOTE: This is illustrative for the sake of executing on the local machine
# For a more realistic scenario, replace this with a GenericHTTPClient and use a
# paid cloud model subscription
primary_model = TransformersLLM(
    name="qwen_heavy", checkpoint="Qwen/Qwen2.5-1.5B-Instruct"
)
primary_client = RoboMLHTTPClient(model=primary_model)

# Backup: A smaller model running locally (e.g., via Ollama)
backup_model = OllamaModel(name="llama_local", checkpoint="llama3.2:3b")
backup_client = OllamaClient(model=backup_model)

# Define Topics
user_query = Topic(name="user_query", msg_type="String")
llm_response = Topic(name="llm_response", msg_type="String")

# Configure the LLM Component
# We initialize the component with the primary client.
llm_component = LLM(
    inputs=[user_query],
    outputs=[llm_response],
    model_client=primary_client,
    trigger=user_query,
    component_name="brain",
    config=LLMConfig(stream=True),
)

# Register the Backup Client
# We store the backup client in the component's internal registry.
# 'local_backup_client' is the key we will use to refer to this client later.
llm_component.additional_model_clients = {"local_backup_client": backup_client}

# Define the Fallback Action
# This action calls the component's internal method `change_model_client`.
# We pass the key ('local_backup_client') defined in previous step
switch_to_backup = Action(
    method=llm_component.change_model_client, args=("local_backup_client",)
)

# Bind Failures to the Action
# If the component fails (e.g. if the initial model setup fails) or the algorithm
# crashes (if inference fails due to disconnection or server error), it will
# attempt to switch clients.
llm_component.on_component_fail(action=switch_to_backup, max_retries=3)
llm_component.on_algorithm_fail(action=switch_to_backup, max_retries=3)

# Launch
launcher = Launcher()
launcher.add_pkg(
    components=[llm_component],
    multiprocessing=True,
    package_name="automatika_embodied_agents",
)
launcher.bringup()
