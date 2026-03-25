import numpy as np
from agents.components import LLM
from agents.models import OllamaModel
from agents.vectordbs import ChromaDB
from agents.config import LLMConfig
from agents.clients import ChromaClient, OllamaClient
from agents.ros import Launcher, Topic

# Start a Llama3.2 based llm component using ollama client
llama = OllamaModel(name="llama", checkpoint="llama3.2:3b")
llama_client = OllamaClient(llama)

# Initialize a vector DB that will store our routes
chroma = ChromaDB()
chroma_client = ChromaClient(db=chroma)

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
    template="""What are the position coordinates in the given metadata?"""
)


# pre-process the output before publishing to a topic of msg_type PoseStamped
def get_coordinates(position: list[float]) -> np.ndarray:
    """Get position coordinates"""
    return np.array(position, dtype=float)


function_description = {
    "type": "function",
    "function": {
        "name": "get_coordinates",
        "description": "Get position coordinates",
        "parameters": {
            "type": "object",
            "properties": {
                "position": {
                    "type": "list[float]",
                    "description": "The position coordinates in x, y and z",
                }
            },
        },
        "required": ["position"],
    },
}

# add the pre-processing function to the goal_point output topic
goto.register_tool(
    tool=get_coordinates,
    tool_description=function_description,
    send_tool_response_to_model=False,
)

# Launch the component
launcher = Launcher()
launcher.add_pkg(components=[goto])
launcher.bringup()
