from typing import Optional, List, Union, Dict, Callable, cast
import json
from pathlib import Path
from enum import Enum

from ..clients.db_base import DBClient
from ..clients.model_base import ModelClient
from ..config import SemanticRouterConfig, LLMConfig
from ..ros import String, Topic, Route, get_logger, Event, Action
from ..utils import validate_func_args
from .llm import LLM


class RouterMode(Enum):
    """
    Enum representing the operational modes of the SemanticRouter.
    Modes:
    - LLM: Agentic mode using LLM for intent analysis and routing
    - VECTOR: Vector mode using embeddings and vector database for routing
    """

    LLM = "LLM"
    VECTOR = "VECTOR"


class SemanticRouter(LLM):
    """
    A unified component that routes semantic information from input topics to output topics.

    This component can operate in two modes:
    1. **Vector Mode (Standard):** Uses a vector database to route inputs based on embedding similarity to route samples.
    2. **LLM Mode (Agentic):** Uses an LLM to intelligently analyze intent and route inputs via function calling.

    The mode is determined automatically based on the client provided (`db_client` vs `model_client`).

    :param inputs:
        A list of input text topics that this component will subscribe to.
    :type inputs: list[Topic]
    :param routes:
        A list of pre-defined routes that publish incoming input to the routed output topics.
    :type routes: list[Route]
    :param default_route:
        An optional route that specifies the default behavior when no specific route matches.
        In Vector Mode, this is used based on distance threshold.
        In LLM Mode, this is used if the model fails to select a route.
    :type default_route: Optional[Route]
    :param config:
        The configuration object. accepts `SemanticRouterConfig` (for vector mode parameters)
        or `LLMConfig` (if specific LLM settings are needed). Defaults to SemanticRouterConfig.
    :type config: Union[SemanticRouterConfig, LLMConfig]
    :param db_client:
        (Vector Mode) A database client used to store and retrieve routing information.
    :type db_client: Optional[DBClient]
    :param model_client:
        (LLM Mode) A model client used for intelligent intent analysis and tool calling.
    :type model_client: Optional[ModelClient]
    :param component_name:
        The name of this Semantic Router component (default: "router_component").
    :type component_name: str
    :param kwargs:
        Additional keyword arguments.

    Example usage (Vector Mode):
    ```python
    # ... define topics and routes ...
    config = SemanticRouterConfig(router_name="my_vector_router")
    db_client = HTTPDBClient(db=ChromaDB(host='localhost', port=8080))

    router = SemanticRouter(
        inputs=[input_text],
        routes=[route1, route2],
        db_client=db_client,
        config=config,
        component_name="router"
    )
    ```

    Example usage (LLM Mode):
    ```python
    # ... define topics and routes ...
    model_client = OllamaClient(model_name="llama3", checkpoint=llama3.1:latest,
                                init_params={"temperature": 0.0})

    router = SemanticRouter(
        inputs=[input_text],
        routes=[route1, route2],
        model_client=model_client,
        component_name="smart_router"
    )
    ```
    """

    @validate_func_args
    def __init__(
        self,
        *,
        inputs: List[Topic],
        routes: List[Route],
        config: Optional[Union[SemanticRouterConfig, LLMConfig]] = None,
        db_client: Optional[DBClient] = None,
        model_client: Optional[ModelClient] = None,
        default_route: Optional[Route] = None,
        component_name: str,
        **kwargs,
    ):
        self.allowed_inputs = {"Required": [String]}
        self.allowed_outputs = {"Required": [String]}

        # Prepare output topics
        self.routes_dict: Dict[str, Route] = {}
        self._action_routes: Dict[Event, Union[Action, List[Action]]] = {}
        topic_routes: List[Topic] = self._config_routes(inputs, routes, component_name)

        # validate outputs, inputs will be validated in parent
        self._validate_topics(topic_routes, self.allowed_outputs, "Outputs")

        # Determine operation mode
        if model_client:
            if not model_client.supports_tool_calls:
                raise TypeError(
                    f"The provided model client ({model_client.__class__.__name__}) does not support tool calling, "
                    "which is required for Agentic Routing."
                )
            if db_client:
                get_logger(component_name).warning(
                    "You have provided a model client with an LLM model to the SemanticRouter, the db client will be ignored and the LLM would be used for routing decisions."
                )

            self.routing_mode = RouterMode.LLM
            # If config is missing or is the wrong type, create a strict LLMConfig
            if isinstance(config, LLMConfig):
                # enforce certain options for routing for routing logic
                component_config = config
                component_config.stream = False
                component_config.enable_rag = False
                component_config.chat_history = False
            else:
                component_config = LLMConfig(
                    stream=False, enable_rag=False, chat_history=False
                )
        elif db_client:
            self.routing_mode = RouterMode.VECTOR
            component_config: Union[SemanticRouterConfig, LLMConfig] = (
                config
                if isinstance(config, SemanticRouterConfig)
                else SemanticRouterConfig(router_name="default_router")
            )
        elif isinstance(config, LLMConfig) and config.enable_local_model:
            self.routing_mode = RouterMode.LLM
            component_config = config
            component_config.stream = False
            component_config.enable_rag = False
            component_config.chat_history = False
        else:
            raise ValueError(
                "A semantic router must be initiated with a DB Client (vector mode), a model client with an LLM model or an LLMConfig with enable_local_model=True (agentic mode)."
            )

        # Create the parent, db client would be created in the parent
        super().__init__(
            inputs=inputs,
            outputs=topic_routes,
            config=component_config
            if isinstance(component_config, LLMConfig)
            else None,
            model_client=model_client if self.routing_mode is RouterMode.LLM else None,
            db_client=db_client if self.routing_mode is RouterMode.VECTOR else None,
            trigger=inputs,
            component_name=component_name,
            **kwargs,
        )

        # After super init we add any event/action pairs to component
        for e, a in self._action_routes.items():
            self._add_event_action_pair(e, a)

        # We keep an internal config as self.config is being used in LLM
        self._internal_config = (
            self.config if isinstance(component_config, LLMConfig) else component_config
        )

        # Setup default route
        if default_route:
            self._internal_config._default_route = self._get_route_name(
                default_route.routes_to
            )

            # Check if the calculated name exists in our processed routes
            if self._internal_config._default_route not in self.routes_dict:
                raise ValueError(
                    f"Default route '{self._internal_config._default_route}' must be one of the specified routes."
                )

        self.default_route = self._internal_config._default_route

        # Initialize internal state
        self._current_payload: Optional[str] = None
        self._route_funcs: Dict[str, Callable] = {}

    def _get_route_name(
        self, entity: Union[Topic, Action, List[Topic], List[Action]]
    ) -> str:
        """Helper to generate a unique string name for a route destination."""
        if isinstance(entity, list):
            # Sort to ensure consistent naming regardless of list order
            # Recursive call is safe because we don't support mixed lists or nested lists
            names = sorted([self._get_route_name(e) for e in entity])
            return "_and_".join(names)
        elif isinstance(entity, Topic):
            return entity.name
        elif isinstance(entity, Action):
            return entity.action_name
        raise ValueError(f"Unsupported route entity type: {type(entity)}")

    def _config_routes(
        self, inputs: List[Topic], routes: List[Route], component_name: str
    ) -> List[Topic]:
        """Configure routes"""
        route_topics = []

        for route in routes:
            route_name = self._get_route_name(route.routes_to)

            # Check for duplicate names
            if route_name in self.routes_dict:
                raise ValueError(
                    f"Duplicate route detected: '{route_name}'. Routes must have unique destination names."
                )

            # Determine the type of the route target (Topic/List[Topic] OR Action/List[Action])
            targets = (
                route.routes_to
                if isinstance(route.routes_to, list)
                else [route.routes_to]
            )

            # Identify if this is a Topic route or an Action route based on the first element
            is_action_route = isinstance(targets[0], Action)

            # Sanity check for mixed types, should not be needed with attrs type check
            if any(isinstance(t, Action) != is_action_route for t in targets):
                raise ValueError(f"Route '{route_name}' cannot mix Topics and Actions.")

            if is_action_route:
                # Create event topic for action route and add to list
                # augment action name to make it clear it's an internal topic
                event_topic = Topic(
                    name=f"internal_router_event/{component_name}/{route_name}",
                    msg_type="String",
                )
                # Add new event topic to list which will be sent to create publishers
                route_topics.append(event_topic)

                # Create a new route based on the new topic
                new_route = Route(routes_to=event_topic, samples=route.samples)
                self.routes_dict[event_topic.name] = (
                    new_route  # use the event topic name
                )

                # Create an OnAny event
                event = Event(event_topic)

                # Cast to actions to remove any type confusion
                action_targets = cast(List[Action], targets)

                # Replace any query topic inputs with event topic inputs for ALL actions
                # This is necessary to reduce the delay for event triggering
                for action in action_targets:
                    for input in inputs:
                        action.replace_input_topic(input, event_topic)

                # Keep event action/list(action) pair to be added to component later
                # Cast route.routes_to because the dict expects Action | List[Action]
                self._action_routes[event] = cast(
                    Union[Action, List[Action]], route.routes_to
                )
            else:
                for topic in targets:
                    # Add topic(s) to list which will be sent to create publishers
                    route_topics.append(topic)

                self.routes_dict[route_name] = route

        return route_topics

    def custom_on_configure(self):
        self.get_logger().debug(f"Current Status: {self.health_status.value}")

        # initialize routes
        if self.routing_mode is RouterMode.LLM:
            self.get_logger().info("SemanticRouter starting in LLM (Agentic) Mode.")
            self._setup_llm_routes(self.routes_dict)
        else:
            self.get_logger().info(
                "SemanticRouter starting in VECTOR (Embedding) Mode."
            )
            self._initialize_vector_routes()

        # NOTE: It is important to call super config AFTER setting routes as tools
        # in case of agentic routing so that system prompt is set correctly
        super().custom_on_configure()

    def custom_on_deactivate(self):
        """Deactivate component."""
        super().custom_on_deactivate()

    # =========================================================================
    # LOCKED METHODS (Public API Restrictions)
    # =========================================================================

    def set_component_prompt(self, template: Union[str, Path]) -> None:
        """
        LOCKED: The SemanticRouter does not use component prompts.
        """
        raise NotImplementedError(
            "SemanticRouter does not support custom component prompts. "
            "Routing is determined strictly by the input topic content and Route definitions."
        )

    def set_topic_prompt(self, input_topic: Topic, template: Union[str, Path]) -> None:
        """
        LOCKED: Input topics are routed as-is.
        """
        raise NotImplementedError(
            "SemanticRouter does not support topic-level prompts. "
            "Raw input is required for accurate semantic comparison."
        )

    def register_tool(
        self, tool, tool_description, send_tool_response_to_model=False
    ) -> None:
        """
        LOCKED: Tools are automatically generated from 'Route' objects.
        """
        raise NotImplementedError(
            "You cannot manually register tools with the SemanticRouter. "
            "Please provide 'Route' objects in the constructor, which are automatically converted to tools."
        )

    def add_documents(self, ids, metadatas, documents) -> None:
        """
        LOCKED: Document storage is managed via Route samples.
        """
        raise NotImplementedError(
            "Do not add documents manually. The router manages its own vector store "
            "based on the 'samples' provided in your Route objects."
        )

    def _publish_to_route(self, route_name: str, payload: Optional[str]):
        """Helper to publish payload to all topics associated with a route name."""
        route = self.routes_dict.get(route_name)
        if not route:
            self.get_logger().error(
                f"Attempted to publish to unknown route: {route_name}"
            )
            return

        # route.routes_to here is guaranteed to be Topic or List[Topic]
        # because _config_routes converts Actions to Event Topics.
        targets = (
            route.routes_to if isinstance(route.routes_to, list) else [route.routes_to]
        )

        for target in targets:
            # We strictly expect Topics here (either external or internal event topics)
            self.publishers_dict[target.name].publish(payload)

    def _initialize_vector_routes(self):
        """(VECTOR MODE) Create routes by saving route samples in the database."""
        self.get_logger().info("Initializing all routes in VECTOR MODE")
        for idx, (name, route) in enumerate(self.routes_dict.items()):
            route_to_add = {
                "collection_name": self._internal_config.router_name,  # type: ignore
                "distance_func": self._internal_config.distance_func,
                "documents": route.samples,
                "metadatas": [{"route_name": name} for _ in range(len(route.samples))],
                "ids": [f"{name}.{i}" for i in range(len(route.samples))],
            }
            # reset collection on the addition of first route if it exists
            if idx == 0:
                route_to_add["reset_collection"] = True

            self.db_client.add(route_to_add)

    def _setup_llm_routes(self, routes: Dict[str, Route]):
        """(LLM MODE) Configure LLM as the router."""
        self.get_logger().info("Initializing all routes in LLM (Agentic) MODE")
        # If a system prompt has been set by the user, keep it
        if self._internal_config._system_prompt:  # type: ignore
            return

        # Define a strict system prompt
        system_prompt = (
            "You are a strict semantic routing agent. You will receive input text. "
            "Analyze the intent and call the specific tool that matches that intent. "
            "Do not respond with text. Only call functions."
        )

        # Set it in the config
        self._internal_config._system_prompt = system_prompt

        for name, route in routes.items():
            self._register_route_tool(name, route)

    def _register_route_tool(self, route_name: str, route: Route):
        """Creates a tool and registers it via the parent class."""

        # Tool: Publish the buffered payload
        def route_action():
            if self._current_payload:
                self._publish_to_route(route_name, self._current_payload)

        # Tool Description: Use samples from the Route object
        samples_str = ", ".join(f"'{s}'" for s in route.samples)
        description = {
            "type": "function",
            "function": {
                "name": f"route_to_{route_name}",
                "description": f"Route message to '{route_name}'. Use this for intents like: {samples_str}",
                "parameters": {"type": "object", "properties": {}, "required": []},
            },
        }

        # Register tool
        self._route_funcs[description["function"]["name"]] = route_action
        self._internal_config._tool_descriptions.append(description)  # type: ignore

    def _vector_mode_execution_step(self):
        """Vector mode execution"""
        self.get_logger().debug("Executing VECTOR mode routing step")
        # get route
        db_input = {
            "collection_name": self._internal_config.router_name,  # type: ignore
            "query": self._current_payload,
            "n_results": 1,
        }
        result = self.db_client.query(db_input) if self.db_client else None

        # TODO: Add treatment of multiple results by using an averaging function
        if result:
            distance = result["output"]["distances"][0][0]

            # if default route is specified and distance is less than min
            # threshold, redirect to default route
            route_name = (
                self.default_route
                if self.default_route
                and distance > self._internal_config.maximum_distance  # type: ignore
                else result["output"]["metadatas"][0][0]["route_name"]
            )

            self.get_logger().debug(f"Routing payload to: {route_name}")

            # Publish to route
            self._publish_to_route(route_name, self._current_payload)

        else:
            self.health_status.set_fail_algorithm()

    def _llm_mode_execution_step(self, **kwargs):
        """LLM Mode Execution"""
        self.get_logger().debug("Executing LLM mode routing step")

        # Create inference input
        inference_input = self._create_input(**kwargs)

        if not inference_input:
            self.get_logger().warning("Input creation failed, skipping inference")
            return

        # Call inference
        result = self._call_inference(inference_input)

        # Response must return tool calls
        routed = False
        tool_calls = result.get("tool_calls", []) if result else []

        if tool_calls:
            for tool in tool_calls:
                fn_name = tool["function"]["name"]
                # Retrieve routing function
                if func := self._route_funcs.get(fn_name):
                    self.get_logger().debug(f"Routing payload to: {fn_name}")
                    func()
                    routed = True

        # Fallback to default route
        if not routed:
            self.get_logger().info("LLM did not trigger any route.")
            if self.default_route:
                self.get_logger().info(f"Using default route: {self.default_route}")
                self._publish_to_route(self.default_route, self._current_payload)
            else:
                self.health_status.set_fail_algorithm()

    def _execution_step(self, **kwargs):
        """Execution step for Semantic Router component.
        :param kwargs:
        """
        trigger = kwargs.get("topic")
        if not trigger:
            return

        self.get_logger().debug(f"Received trigger on {trigger.name}")
        self._current_payload = self.trig_callbacks[trigger.name].get_output()

        if not self._current_payload:
            return

        self.get_logger().debug(f"Routing payload: {self._current_payload}")

        # VECTOR MODE Processing
        if self.routing_mode is RouterMode.VECTOR:
            self._vector_mode_execution_step()

        # LLM MODE
        elif self.routing_mode is RouterMode.LLM:
            self._llm_mode_execution_step(**kwargs)

    def _update_cmd_args_list(self):
        """
        Update launch command arguments
        """
        self.config = self._internal_config  # type: ignore
        super()._update_cmd_args_list()

        self.launch_cmd_args = [
            "--routes",
            self._get_routes_json(),
        ]

    def _get_routes_json(self) -> Union[str, bytes, bytearray]:
        """
        Serialize component routes to json.

        If a route destination is a Topic, it is serialized normally.

        :return: Serialized inputs
        :rtype:  str | bytes | bytearray
        """
        if not hasattr(self, "routes_dict"):
            return "[]"
        return json.dumps([route.to_json() for route in self.routes_dict.values()])
