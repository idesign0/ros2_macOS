from agents.components import LLM
from agents.models import TransformersLLM
from agents.clients import RoboMLHTTPClient
from agents.ros import Launcher, Topic, Action

# Primary: A powerful model hosted remotely (e.g., via RoboML)
# NOTE: This is illustrative for the sake of executing on the local machine
# For a more realistic scenario, replace this with a GenericHTTPClient and use a
# paid cloud model subscription
primary_model = TransformersLLM(
    name="qwen_heavy", checkpoint="Qwen/Qwen2.5-1.5B-Instruct"
)
primary_client = RoboMLHTTPClient(model=primary_model)

# Define Topics
user_query = Topic(name="user_query", msg_type="String")
llm_response = Topic(name="llm_response", msg_type="String")

# Configure the LLM Component with the primary client.
# No need to set enable_local_model in config
# fallback_to_local() handles it automatically
llm_component = LLM(
    inputs=[user_query],
    outputs=[llm_response],
    model_client=primary_client,
    trigger=user_query,
    component_name="brain",
)

# One-liner fallback: switch to built-in local model on failure.
# No additional model clients, no Ollama server — just a single Action.
switch_to_local = Action(method=llm_component.fallback_to_local)
llm_component.on_component_fail(action=switch_to_local, max_retries=3)
llm_component.on_algorithm_fail(action=switch_to_local, max_retries=3)

# Launch
launcher = Launcher()
launcher.add_pkg(
    components=[llm_component],
    multiprocessing=True,
    package_name="automatika_embodied_agents",
)
launcher.bringup()
