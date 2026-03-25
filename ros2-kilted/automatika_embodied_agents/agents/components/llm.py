import json
import re
from pathlib import Path
from typing import Any, Optional, Union, Callable, List, Dict, MutableMapping
import msgpack
import msgpack_numpy as m_pack

from ..callbacks import TextCallback
from ..clients.db_base import DBClient
from ..clients.model_base import ModelClient
from ..clients import OllamaClient, GenericHTTPClient
from ..config import LLMConfig
from ..ros import (
    FixedInput,
    Event,
    String,
    Topic,
    DetectionsMultiSource,
    Detections,
    StreamingString,
)
from ..utils import get_prompt_template, validate_func_args
from .model_component import ModelComponent
from .component_base import ComponentRunType

# patch msgpack for numpy arrays
m_pack.patch()


class LLM(ModelComponent):
    """
    This component utilizes large language models (e.g LLama) that can be used to process text data.

    :param inputs: The input topics or fixed inputs for the LLM component.
        This should be a list of Topic objects or FixedInput instances.
    :type inputs: list[Topic | FixedInput]
    :param outputs: The output topics for the LLM component.
        This should be a list of Topic objects. String type is handled automatically.
    :type outputs: list[Topic]
    :param model_client: The model client for the LLM component.
        This should be an instance of ModelClient. Optional if ``enable_local_model`` is set to True in the config.
    :type model_client: Optional[ModelClient]
    :param config: The configuration for the LLM component.
        This should be an instance of LLMConfig. If not provided, defaults to LLMConfig().
    :type config: LLMConfig
    :param db_client: An optional database client for the LLM component.
        If provided, this should be an instance of DBClient. Otherwise, it defaults to None.
    :type db_client: Optional[DBClient]
    :param trigger: The trigger value or topic for the LLM component.
        This can be a single Topic object, a list of Topic objects, or a float value for a timed component. Defaults to 1.
    :type trigger: Union[Topic, list[Topic], float]
    :param component_name: The name of the LLM component. This should be a string.
    :type component_name: str
    :param kwargs: Additional keyword arguments for the LLM.

    Example usage:
    ```python
    text0 = Topic(name="text0", msg_type="String")
    text1 = Topic(name="text1", msg_type="String")
    config = LLMConfig()
    model = OllamaModel(name='llama', checkpoint='llama3.2:3b')
    model_client = ModelClient(model=model)
    llm_component = LLM(inputs=[text0],
                        outputs=[text1],
                        model_client=model_client,
                        config=config,
                        component_name='llama_component')
    ```

    Example usage with local model:
    ```python
    text0 = Topic(name="text0", msg_type="String")
    text1 = Topic(name="text1", msg_type="String")
    config = LLMConfig(enable_local_model=True)
    llm_component = LLM(inputs=[text0],
                        outputs=[text1],
                        config=config,
                        component_name='local_llm')
    ```
    """

    @validate_func_args
    def __init__(
        self,
        *,
        inputs: List[Union[Topic, FixedInput]],
        outputs: List[Topic],
        model_client: Optional[ModelClient] = None,
        config: Optional[LLMConfig] = None,
        db_client: Optional[DBClient] = None,
        trigger: Union[Topic, List[Topic], float, Event] = 1.0,
        component_name: str,
        **kwargs,
    ):
        self.config: LLMConfig = config or LLMConfig()
        # set allowed inputs/outputs when parenting multimodal LLMs
        self.allowed_inputs = (
            kwargs["allowed_inputs"]
            if kwargs.get("allowed_inputs")
            else {
                "Required": [String],
                "Optional": [DetectionsMultiSource, Detections],
            }
        )
        self.handled_outputs = [String, StreamingString]

        if type(self) is LLM and not model_client:
            if not self.config.enable_local_model:
                raise RuntimeError(
                    "LLM component requires a model_client or enable_local_model=True in LLMConfig."
                )

        self.model_client = model_client

        self.db_client = db_client if db_client else None

        self.component_prompt = (
            get_prompt_template(self.config._component_prompt)
            if self.config._component_prompt
            else None
        )

        # Initialize a messages buffer
        self.messages: List[Dict] = (
            [{"role": "system", "content": self.config._system_prompt}]
            if self.config._system_prompt
            else []
        )

        super().__init__(
            inputs,
            outputs,
            model_client,
            self.config,
            trigger,
            component_name,
            **kwargs,
        )

    def custom_on_configure(self):

        # deploy local LLM if enabled (only for LLM, not subclasses like MLLM)
        if (
            not self.model_client
            and self.config.enable_local_model
            and type(self) is LLM
        ):
            self._deploy_local_model()

        # configure the rest
        super().custom_on_configure()

        # add system prompt if set after init
        self.messages: List[Dict] = (
            [{"role": "system", "content": self.config._system_prompt}]
            if self.config._system_prompt
            else []
        )
        # add component prompt if set after init
        self.component_prompt = (
            get_prompt_template(self.config._component_prompt)
            if self.config._component_prompt
            else None
        )
        # add topic templates
        if self.config._topic_prompts:
            for topic_name, template in self.config._topic_prompts.items():
                callback = self.callbacks[topic_name]
                if isinstance(callback, TextCallback):
                    callback._template = get_prompt_template(template)

        # initialize db client
        if self.db_client:
            self.db_client.check_connection()
            self.db_client.initialize()

        # initialize response buffers used for streaming
        if self.config.stream:
            # initialize think block state for streaming
            self._in_think_block: bool = False
            # initialize result buffers
            self.result_partial: List = []
            self.result_complete: List = []
            # issue a warning in case StreamingText type is not used in output
            streaming_string_topics = any(
                topic.msg_type is StreamingString for topic in self.out_topics
            )
            if not streaming_string_topics:
                self.get_logger().warning("""Consider using `StreamingString` msg_type in output topic(s) when streaming text responses from LLMs/MLLMs. Example:
    output_topic = Topic(name='<topic_name>', msg_type='StreamingString')""")

    def custom_on_deactivate(self):
        # deactivate db client
        if self.db_client:
            self.db_client.check_connection()
            self.db_client.deinitialize()

        # deactivate the rest
        super().custom_on_deactivate()

    @validate_func_args
    def add_documents(
        self, ids: List[str], metadatas: List[Dict], documents: List[str]
    ) -> None:
        """Add documents to vector DB for Retrieval Augmented Generation (RAG).

        ```{important}
        Documents can be provided after parsing them using a document parser. Checkout various document parsers, available in packages like [langchain_community](https://github.com/langchain-ai/langchain/tree/master/libs/community/langchain_community/document_loaders/parsers)
        ```

        :param ids: List of unique string ids for each document
        :type ids: list[str]
        :param metadatas: List of metadata dicts for each document
        :type metadatas: list[dict]
        :param documents: List of documents which are to be store in the vector DB
        :type documents: list[str]
        :rtype: None
        """

        if not self.db_client:
            raise AttributeError(
                "db_client needs to be set in component for add_documents to work"
            )

        db_input = {
            "collection_name": self.config.collection_name,
            "distance_func": self.config.distance_func,
            "ids": ids,
            "documents": documents,
            "metadatas": metadatas,
        }
        self.db_client.add(db_input)

    def _strip_think_tokens(self, text: str) -> str:
        """Strip <think>...</think> blocks from model output."""
        if self.config.strip_think_tokens:
            return re.sub(r"<think>.*?</think>", "", text, flags=re.DOTALL).strip()
        return text

    def _deploy_local_model(self):
        """Deploy local LLM model on demand."""
        if self.local_model is not None:
            return  # already deployed
        from ..utils.local_llm import LocalLLM

        self.local_model = LocalLLM(
            model_path=self.config.local_model_path,
            device=self.config.device_local_model,
            ncpu=self.config.ncpu_local_model,
        )

    def _handle_rag_query(self, query: str) -> Optional[str]:
        """Internal handler for retrieving documents for RAG.
        :param query:
        :type query: str
        :rtype: str | None
        """
        if not self.db_client:
            raise AttributeError(
                "db_client needs to be set in component for RAG to work"
            )
        # get documents
        db_input = {
            "collection_name": self.config.collection_name,
            "query": query,
            "n_results": self.config.n_results,
        }
        result = self.db_client.query(db_input)
        if result:
            # add metadata as string if asked
            rag_docs = (
                "\n".join(
                    f"{str(meta)}, {doc}"
                    for meta, doc in zip(
                        result["output"]["metadatas"],
                        result["output"]["documents"],
                        strict=True,
                    )
                )
                if self.config.add_metadata
                else "\n".join(doc for doc in result["output"]["documents"])
            )
            return rag_docs

    def _handle_chat_history(self, message: Dict) -> None:
        """Internal handler for chat history"""
        if not self.config.chat_history:
            # keep system prompt if set else empty the list
            self.messages = self.messages[:1] if self.config._system_prompt else []
        else:
            # if the size of history exceeds specified size than take out first
            # two messages (keeping system prompt if it exists)
            messages_length = (
                len(self.messages)
                if not self.config._system_prompt
                else len(self.messages) - 1
            )
            if messages_length / 2 > self.config.history_size + 1:
                self.messages = (
                    self.messages[2:]
                    if not self.config._system_prompt
                    else self.messages[:1] + self.messages[3:]
                )

        self.messages.append(message)

    def _handle_tool_calls(self, result: MutableMapping) -> Optional[MutableMapping]:
        """Internal handler for tool calling"""
        if not result.get("tool_calls"):
            self.get_logger().warning(
                "Tools have been provided but no tool calls found in model response."
            )
            return result

        response_flags = []

        # make tool calls
        for tool in result["tool_calls"]:
            function_to_call = self._external_processors[tool["function"]["name"]][0][0]

            try:
                # HACK: Read function argument as serialized datatypes
                # if they are returned as string
                arg_json = {
                    key: json.loads(str(arg))
                    for key, arg in tool["function"]["arguments"].items()
                }
                if isinstance(function_to_call, Callable):
                    function_response = function_to_call(**arg_json)
                else:
                    payload = msgpack.packb(arg_json)
                    if payload:
                        function_to_call.sendall(payload)
                    else:
                        raise Exception(
                            f"Could not serialize the following function arguments for tool calling: {arg_json}"
                        )

                    result_b = function_to_call.recv(1024)
                    function_response = msgpack.unpackb(result_b)
            except Exception as e:
                self.get_logger().error(f"Exception in tool calling. {e}")
                return result

            # make last function call output the publishable output
            result["output"] = function_response

            # Add function response to the messages
            self.messages.append({"role": "tool", "content": function_response})

            # check for response flags
            response_flags.append(
                self.config._tool_response_flags[tool["function"]["name"]]
            )

        # make call to model again if any tool requires response to be sent back
        if any(response_flags):
            self.get_logger().debug(f"Input from component: {self.messages}")

            input = {
                "query": self.messages,
                **self.config._get_inference_params(),
            }
            if self.model_client:
                return self.model_client.inference(input)
            elif hasattr(self, "local_model"):
                return self.local_model(input)

        else:
            # return result with its output set to last function response
            return result

    def _extract_query_and_context(self, kwargs, context: dict) -> Optional[str]:
        """Extract the query from the trigger topic and initialize context."""
        if trigger := kwargs.get("topic"):
            query = self.trig_callbacks[trigger.name].get_output()
            context[trigger.name] = query
            return query
        return None

    def _should_reset_chat(self, query: Optional[str]) -> bool:
        """Check if the chat should be reset based on the query."""
        return bool(
            self.config.chat_history
            and query
            and query.strip().lower() == self.config.history_reset_phrase
        )

    def _create_input(self, *_, **kwargs) -> Optional[Dict[str, Any]]:
        """Create inference input for LLM models
        :param args:
        :param kwargs:
        :rtype: dict[str, Any]
        """
        # context dict to gather all String inputs for use in system prompt
        context = {}

        # set llm query as trigger
        query = self._extract_query_and_context(kwargs, context)
        if self._should_reset_chat(query):
            self.messages = []
            return None

        # aggregate all inputs that are available
        for i in self.callbacks.values():
            if (item := i.get_output()) is None:
                continue
            msg_type = i.input_topic.msg_type
            # set trigger equal to a topic with type String if trigger not found
            if msg_type == String:
                if not query:
                    query = item
                context[i.input_topic.name] = item
            elif msg_type in [DetectionsMultiSource, Detections]:
                context[i.input_topic.name] = item

        if query is None:
            return None

        # get RAG results if enabled in config and if docs retrieved
        rag_result = self._handle_rag_query(query) if self.config.enable_rag else None

        # set system prompt template
        query = (
            self.component_prompt.render(context) if self.component_prompt else query
        )

        # attach rag results to templated query if available
        query = f"{rag_result}\n{query}" if rag_result else query

        message = {"role": "user", "content": query}
        self._handle_chat_history(message)

        self.get_logger().debug(f"Input from component: {self.messages}")

        input = {
            "query": self.messages,
            **self.inference_params,
        }

        # Add any tools, if registered
        if self.config._tool_descriptions:
            input["tools"] = self.config._tool_descriptions

        return input

    def __process_stream_token(self, token: str):
        """
        Processes a single token from a stream based on the break_character config.
        """
        if self.config.strip_think_tokens:
            if "<think>" in token:
                self._in_think_block = True
                return
            if "</think>" in token:
                self._in_think_block = False
                return
            if self._in_think_block:
                return
        if self.config.break_character:
            self.result_partial.append(token)
            if self.config.break_character in token:
                self.result_complete += self.result_partial
                self._publish(
                    {"output": "".join(self.result_partial)}, stream=True, done=False
                )
                self.result_partial = []
        else:
            self.result_complete.append(token)
            # Publish tokens as they arrive
            # If the token is empty, indicate that the stream is finished
            self._publish(
                {"output": token}, stream=True, done=(False if token else True)
            )
            self.result_partial = []

    def __finalize_stream(self):
        """
        Finalizes the stream by publishing any remaining partial results and
        appending the complete message to the message history.
        """
        self._in_think_block = False
        # Send remaining result after break character or termination if any
        if self.config.break_character:
            if self.result_partial:
                self.result_complete += self.result_partial
                self._publish({"output": "".join(self.result_partial)}, stream=True)
                self.result_partial = []
            else:
                # Publish an empty msg to mark the end of stream
                # (useful only with StreamingText)
                self._publish({"output": ""}, stream=True)

        self.messages.append({
            "role": "assistant",
            "content": "".join(self.result_complete),
        })

    def _handle_websocket_streaming(self) -> Optional[List]:
        """Handle streaming output from a websocket client"""
        try:
            token = self.resp_queue.get(block=True)
            if token and token != self.config.response_terminator:
                self.__process_stream_token(token)
            elif token:  # Token is the response_terminator, finalize stream
                self.__finalize_stream()
        except Exception as e:
            self.get_logger().error(str(e))
            # raise a fallback trigger via health status
            self.health_status.set_fail_algorithm()

    def __handle_streaming_generator(self, result: MutableMapping) -> Optional[List]:
        """Handle streaming output"""
        try:
            for token in result["output"]:
                # Handle ollama client result format
                if isinstance(self.model_client, OllamaClient):
                    token = token["message"]["content"]
                # Handle OpenAI style API result format
                elif isinstance(self.model_client, GenericHTTPClient):
                    token = token["choices"][0]["delta"]["content"]
                self.__process_stream_token(token)

            # finalize stream after the generator is exhausted
            self.__finalize_stream()
        except Exception as e:
            self.get_logger().error(str(e))
            # raise a fallback trigger via health status
            self.health_status.set_fail_algorithm()

    def _execution_step(self, *args, **kwargs):
        """_execution_step.

        :param args:
        :param kwargs:
        """

        if self.run_type is ComponentRunType.EVENT and (trigger := kwargs.get("topic")):
            if trigger:
                self.get_logger().debug(f"Received trigger on topic {trigger.name}")
            else:
                self.get_logger().debug("Got triggered by an event")

        else:
            time_stamp = self.get_ros_time().sec
            self.get_logger().debug(f"Sending at {time_stamp}")

        # create inference input
        inference_input = self._create_input(*args, **kwargs)
        # call model inference
        if not inference_input:
            self.get_logger().warning("Input not received, not calling model inference")
            return

        # conduct inference
        result = self._call_inference(inference_input)

        if result:
            if self.config.stream:
                self.__handle_streaming_generator(result)
                return

            result["output"] = self._strip_think_tokens(result["output"])
            self.messages.append({"role": "assistant", "content": result["output"]})

            # handle tool calls
            if self.config._tool_descriptions:
                result = self._handle_tool_calls(result)

            # raise a fallback trigger via health status
            if not result:
                self.health_status.set_fail_component()
                return

            # publish inference result
            self._publish(result)

    @validate_func_args
    def set_topic_prompt(self, input_topic: Topic, template: Union[str, Path]) -> None:
        """Set prompt template on any input topic of type string.

        :param input_topic: Name of the input topic on which the prompt template is to be applied
        :type input_topic: Topic
        :param template: Template in the form of a valid jinja2 string or a path to a file containing the jinja2 string.
        :type template: Union[str, Path]
        :rtype: None

        Example usage:
        ```python
        llm_component = LLM(inputs=[text0],
                            outputs=[text1],
                            model_client=model_client,
                            config=config,
                            component_name='llama_component')
        llm_component.set_topic_prompt(text0, template="Please answer the following: {{ text0 }}")
        ```
        """
        if callback := self.callbacks.get(input_topic.name):
            if not callback:
                raise TypeError("Specified input topic does not exist")
            if not isinstance(callback, TextCallback):
                raise TypeError(
                    f"Prompt can only be set for a topic of type String, {callback.input_topic.name} is of type {callback.input_topic.msg_type}"
                )
            self.config._topic_prompts[input_topic.name] = template

    @validate_func_args
    def set_component_prompt(self, template: Union[str, Path]) -> None:
        """Set component level prompt template which can use multiple input topics.

        :param template: Template in the form of a valid jinja2 string or a path to a file containing the jinja2 string.
        :type template: Union[str, Path]
        :rtype: None

        Example usage:
        ```python
        llm_component = LLM(inputs=[text0],
                            outputs=[text1],
                            model_client=model_client,
                            config=config,
                            component_name='llama_component')
        llm_component.set_component_prompt(template="You can see the following items: {{ detections }}. Please answer the following: {{ text0 }}")
        ```
        """
        self.config._component_prompt = template

    @validate_func_args
    def set_system_prompt(self, prompt: str) -> None:
        """Set system prompt for the model, which defines the models 'personality'.

        :param prompt: string or a path to a file containing the string.
        :type template: Union[str, Path]
        :rtype: None

        Example usage:
        ```python
        llm_component = LLM(inputs=[text0],
                            outputs=[text1],
                            model_client=model_client,
                            config=config,
                            component_name='llama_component')
        llm_component.set_system_prompt(prompt="You are an amazing and funny robot. You answer all questions with short and concise answers.")
        ```
        """
        self.config._system_prompt = prompt

    @validate_func_args
    def register_tool(
        self,
        tool: Callable,
        tool_description: Dict,
        send_tool_response_to_model: bool = False,
    ) -> None:
        """Register a tool with the component which can be called by the model. If the send_tool_response_to_model flag is set to True than the output of the tool is sent back to the model and final output of the model is sent to component publishers (i.e. the model "uses" the tool to give a more accurate response.). If the flag is set to False than the output of the tool is sent to publishers of the component.

        :param tool: An arbitrary function that needs to be called. The model response will describe a call to this function.
        :type tool: Callable
        :param tool_description: A dictionary describing the function. This dictionary needs to be made in the format shown [here](https://ollama.com/blog/tool-support). Also see usage example.
        :type tool_description: dict
        :param send_tool_response_to_model: Whether the model should be called with the tool response. If set to false the tool response will be sent to component publishers. If set to true, the response will be sent back to the model and the final response from the model will be sent to the publishers. Default is False.
        :param send_tool_response_to_model: bool
        :rtype: None

        Example usage:
        ```python
        def my_arbitrary_function(first_param: str, second_param: int) -> str:
            return f"{first_param}, {second_param}"

        my_func_description = {
                  'type': 'function',
                  'function': {
                    'name': 'my_arbitrary_function',
                    'description': 'Description of my arbitrary function',
                    'parameters': {
                      'type': 'object',
                      'properties': {
                        'first_param': {
                          'type': 'string',
                          'description': 'Description of the first param',
                        },
                        'second_param': {
                          'type': 'int',
                          'description': 'Description of the second param',
                        },
                      },
                      'required': ['first_param', 'second_param'],
                    },
                  },
                }

        my_component.register_tool(tool=my_arbitrary_function, tool_description=my_func_description, send_tool_response_to_model=False)
        ```
        """
        if self.model_client and not self.model_client.supports_tool_calls:
            raise TypeError(
                f"The provided model client ({self.model_client.__class__.__name__}) does not support tool calling."
            )
        if self.config.stream:
            raise TypeError(
                "Tools cannot be registered for a component with streaming output. Please set stream option to False."
            )
        self._external_processors[tool_description["function"]["name"]] = (
            [tool],
            "tool",
        )
        self.config._tool_descriptions.append(tool_description)
        self.config._tool_response_flags[tool_description["function"]["name"]] = (
            send_tool_response_to_model
        )

    def _update_cmd_args_list(self):
        """
        Update launch command arguments
        """
        super()._update_cmd_args_list()

        self.launch_cmd_args = [
            "--db_client",
            self._get_db_client_json(),
        ]

    def _get_db_client_json(self) -> Union[str, bytes, bytearray]:
        """
        Serialize component routes to json

        :return: Serialized inputs
        :rtype:  str | bytes | bytearray
        """
        if not self.db_client:
            return ""
        return json.dumps(self.db_client.serialize())

    def _warmup(self):
        """Warm up and stat check"""
        import time

        message = {"role": "user", "content": "Hello robot."}
        inference_input = {"query": [message], **self.inference_params}

        # Run inference once to warm up and once to measure time
        if self.model_client:
            self.model_client.inference(inference_input)
        elif hasattr(self, "local_model"):
            self.local_model(inference_input)

        inference_input = {"query": [message], **self.config._get_inference_params()}
        start_time = time.time()
        if self.model_client:
            result = self.model_client.inference(inference_input)
        elif hasattr(self, "local_model"):
            result = self.local_model(inference_input)
        else:
            result = None
        elapsed_time = time.time() - start_time

        if result:
            self.get_logger().warning(f"Model Output: {result['output']}")
            self.get_logger().warning(
                f"Approximate Inference time: {elapsed_time} seconds"
            )
        else:
            self.get_logger().error("Model inference failed during warmup.")
