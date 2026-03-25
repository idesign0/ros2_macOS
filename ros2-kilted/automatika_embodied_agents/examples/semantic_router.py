from typing import Optional
import json
import numpy as np
from agents.components import LLM, SemanticRouter
from agents.models import OllamaModel
from agents.vectordbs import ChromaDB
from agents.config import LLMConfig, SemanticRouterConfig
from agents.clients import ChromaClient, OllamaClient
from agents.ros import Launcher, Topic, Route

# Start a Llama3.2 based llm component using ollama client
llama = OllamaModel(name="llama", checkpoint="llama3.2:3b")
llama_client = OllamaClient(llama)

# Initialize a vector DB that will store our routes
chroma = ChromaDB()
chroma_client = ChromaClient(db=chroma)


# Make a generic LLM component using the Llama3_2 model
llm_in = Topic(name="text_in_llm", msg_type="String")
llm_out = Topic(name="text_out_llm", msg_type="String")

llm = LLM(
    inputs=[llm_in],
    outputs=[llm_out],
    model_client=llama_client,
    trigger=llm_in,
    component_name="generic_llm",
)


# Define LLM input and output topics including goal_point topic of type PoseStamped
goto_in = Topic(name="goto_in", msg_type="String")
goal_point = Topic(name="goal_point", msg_type="PoseStamped")

config = LLMConfig(
    enable_rag=True,
    collection_name="map",
    distance_func="l2",
    n_results=1,
    add_metadata=True,
)

# initialize the component
goto = LLM(
    inputs=[goto_in],
    outputs=[goal_point],
    model_client=llama_client,
    db_client=chroma_client,  # check the previous example where we setup this database client
    trigger=goto_in,
    config=config,
    component_name="go_to_x",
)

# set a component prompt
goto.set_component_prompt(
    template="""From the given metadata, extract coordinates and provide
    the coordinates in the following json format:\n {"position": coordinates}"""
)


# pre-process the output before publishing to a topic of msg_type PoseStamped
def llm_answer_to_goal_point(output: str) -> Optional[np.ndarray]:
    # extract the json part of the output string (including brackets)
    # one can use sophisticated regex parsing here but we'll keep it simple
    json_string = output[output.find("{") : output.find("}") + 1]

    # load the string as a json and extract position coordinates
    # if there is an error, return None, i.e. no output would be published to goal_point
    try:
        json_dict = json.loads(json_string)
        return np.array(json_dict["position"])
    except Exception:
        return


# add the pre-processing function to the goal_point output topic
goto.add_publisher_preprocessor(goal_point, llm_answer_to_goal_point)

# Create the input topic for the router
query_topic = Topic(name="question", msg_type="String")

# Define a route to a topic that processes go-to-x commands
goto_route = Route(
    routes_to=goto_in,
    samples=[
        "Go to the door",
        "Go to the kitchen",
        "Get me a glass",
        "Fetch a ball",
        "Go to hallway",
    ],
)

# Define a route to a topic that is input to an LLM component
llm_route = Route(
    routes_to=llm_in,
    samples=[
        "What is the capital of France?",
        "Is there life on Mars?",
        "How many tablespoons in a cup?",
        "How are you today?",
        "Whats up?",
    ],
)

# --- MODE 1: VECTOR ROUTING (Active) ---
router_config = SemanticRouterConfig(router_name="go-to-router", distance_func="l2")

router = SemanticRouter(
    inputs=[query_topic],
    routes=[llm_route, goto_route],
    default_route=llm_route,  # If none of the routes fall within a distance threshold
    config=router_config,
    db_client=chroma_client,  # Vector mode requires db_client
    component_name="router",
)

# --- MODE 2: LLM ROUTING (Commented Out) ---
# To use LLM routing (Agentic), comment out the block above and uncomment this:
#
# router = SemanticRouter(
#     inputs=[query_topic],
#     routes=[llm_route, goto_route],
#     default_route=llm_route,
#     model_client=llama_client, # LLM mode requires model_client
#     component_name="router",
# )

# Launch the components
launcher = Launcher()
launcher.add_pkg(components=[llm, goto, router])
launcher.bringup()
