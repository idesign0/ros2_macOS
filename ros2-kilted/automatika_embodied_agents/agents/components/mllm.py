from typing import Any, Union, Optional, List, Dict, Literal, MutableMapping

import numpy as np
from ..clients.db_base import DBClient
from ..clients.model_base import ModelClient
from ..config import MLLMConfig
from ..ros import (
    FixedInput,
    Event,
    Image,
    String,
    StreamingString,
    Topic,
    DetectionsMultiSource,
    Detections,
    RGBD,
    PointsOfInterest,
    ComponentRunType,
    ROSImage,
    ROSCompressedImage,
)
from ..utils import validate_func_args
from .llm import LLM


class MLLM(LLM):
    """
    This component utilizes multi-modal large language models (e.g. Llava) that can be used to process text and image data.

    :param inputs: The input topics or fixed inputs for the MLLM component.
        This should be a list of Topic objects or FixedInput instances, limited to String and Image types.
    :type inputs: list[Topic | FixedInput]
    :param outputs: The output topics for the MLLM component.
        This should be a list of Topic objects. String, Detections2D and PointsOfInterest2D types is handled automatically.
    :type outputs: list[Topic]
    :param model_client: The model client for the MLLM component.
        This should be an instance of ModelClient. Optional if ``enable_local_model`` is set to True in the config.
    :type model_client: Optional[ModelClient]
    :param config: Optional configuration for the MLLM component.
        This should be an instance of MLLMConfig. If not provided, defaults to MLLMConfig().
    :type config: MLLMConfig
    :param trigger: The trigger value or topic for the MLLM component.
        This can be a single Topic object, a list of Topic objects, or a float value for a timed component. Defaults to 1.
    :type trigger: Union[Topic, list[Topic], float]
    :param component_name: The name of the MLLM component.
        This should be a string and defaults to "mllm_component".
    :type component_name: str

    Example usage:
    ```python
    text0 = Topic(name="text0", msg_type="String")
    image0 = Topic(name="image0", msg_type="Image")
    text0 = Topic(name="text1", msg_type="String")
    config = MLLMConfig()
    model = TransformersMLLM(name='idefics')
    model_client = ModelClient(model=model)
    mllm_component = MLLM(inputs=[text0, image0],
                          outputs=[text1],
                          model_client=model_client,
                          config=config,
                          component_name='mllm_component')
    ```

    Example usage with local model:
    ```python
    text0 = Topic(name="text0", msg_type="String")
    image0 = Topic(name="image0", msg_type="Image")
    text1 = Topic(name="text1", msg_type="String")
    config = MLLMConfig(enable_local_model=True)
    mllm_component = MLLM(inputs=[text0, image0],
                          outputs=[text1],
                          config=config,
                          component_name='local_vlm')
    ```
    """

    @validate_func_args
    def __init__(
        self,
        *,
        inputs: List[Union[Topic, FixedInput]],
        outputs: List[Topic],
        model_client: Optional[ModelClient] = None,
        config: Optional[MLLMConfig] = None,
        db_client: Optional[DBClient] = None,
        trigger: Union[Topic, List[Topic], float, Event] = 1.0,
        component_name: str,
        **kwargs,
    ):
        self.allowed_inputs = {
            "Required": [String, [Image, RGBD]],
            "Optional": [DetectionsMultiSource, Detections],
        }

        config = config or MLLMConfig()

        if not model_client and not config.enable_local_model:
            raise RuntimeError(
                "MLLM/VLM component requires a model_client or enable_local_model=True in MLLMConfig."
            )

        super().__init__(
            inputs=inputs,
            outputs=outputs,
            model_client=model_client,
            config=config,
            db_client=db_client,
            trigger=trigger,
            component_name=component_name,
            allowed_inputs=self.allowed_inputs,
            **kwargs,
        )

        self.handled_outputs = [
            String,
            StreamingString,
            Detections,
            DetectionsMultiSource,
            PointsOfInterest,
        ]
        self._images: List[Union[np.ndarray, ROSImage, ROSCompressedImage]] = []

    def custom_on_configure(self):
        # deploy local VLM if enabled
        if not self.model_client and self.config.enable_local_model:
            self._deploy_local_model()

        # configure the rest
        super().custom_on_configure()

        # Setup task
        self._task = self.config.task
        if self._task:
            # Initialize the topic type lists
            self._string_publishers: List = []
            self._poi_publishers: List = []
            self._detections_publishers: List = []

            # Loop through the list of topics and categorize them
            for topic in self.out_topics:
                if topic.msg_type in [String, StreamingString]:
                    self._string_publishers.append(topic.name)
                elif topic.msg_type is PointsOfInterest:
                    self._poi_publishers.append(topic.name)
                elif topic.msg_type in [Detections, DetectionsMultiSource]:
                    self._detections_publishers.append(topic.name)
                else:
                    pass

    def _deploy_local_model(self):
        """Deploy local VLM model on demand."""
        if self.local_model is not None:
            return  # already deployed
        from ..utils.local_vlm import LocalVLM

        self.local_model = LocalVLM(
            model_path=self.config.local_model_path,
            device=self.config.device_local_model,
            ncpu=self.config.ncpu_local_model,
        )

    def _create_input(self, *_, **kwargs) -> Optional[Dict[str, Any]]:
        """Create inference input for MLLM models
        :param args:
        :param kwargs:
        :rtype: dict[str, Any]
        """
        self._images = []  # image msgs for publishing
        images = []  # image msg outputs as np arrays

        # context dict to gather all String inputs for use in system prompt
        context = {}
        # set mllm query as trigger
        query = self._extract_query_and_context(kwargs, context)
        if self._should_reset_chat(query):
            self.messages = []
            return None

        # aggregate all inputs that are available
        for i in self.callbacks.values():
            msg = i.msg
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
            # get images from image topics
            if issubclass(msg_type, (Image, RGBD)):
                images.append(item)
                if msg:
                    self._images.append(msg)  # Collect all images for publishing

        if not query or not images:
            return None

        # get RAG results if enabled in config and if docs retrieved
        rag_result = self._handle_rag_query(query) if self.config.enable_rag else None

        # set system prompt template
        query = (
            self.component_prompt.render(context) if self.component_prompt else query
        )

        # get RAG results if enabled in config and if docs retreived
        query = f"{rag_result}\n{query}" if rag_result else query

        message = {"role": "user", "content": query}
        self._handle_chat_history(message)

        self.get_logger().debug(f"Input from component: {self.messages}")

        input = {
            "query": self.messages,
            "images": images,
            **self.inference_params,
        }

        # Add any tools, if registered
        if self.config._tool_descriptions:
            input["tools"] = self.config._tool_descriptions

        return input

    @validate_func_args
    def set_task(
        self,
        task: Literal["general", "pointing", "affordance", "trajectory", "grounding"],
    ) -> None:
        """Set a task for the MLLM component. This is useful when using a multimodal LLM model that has been trained on specific tasks. This method can be invoked as an action, in response to an event, to change the task at runtime.
            For an example checkout [RoboBrain2.0](https://github.com/FlagOpen/RoboBrain2.0), available on [RoboML](https://github.com/automatika-robotics/roboml).

        :param task: A task that is one of the following "general", "pointing", "affordance", "trajectory", "grounding".
        :type text: Literal
        """
        if task not in ["general", "pointing", "affordance", "trajectory", "grounding"]:
            raise ValueError(
                'Task value should be one of the following "general", "pointing", "affordance", "trajectory", "grounding"'
            )
        self._task = task
        self.config.task = task
        self.config.stream = False
        self.inference_params = self.config.get_inference_params()

    def _publish_task_specific_outputs(self, result: MutableMapping) -> None:
        """Publish outputs based on task type"""
        if self._task == "general":
            result["output"] = self._strip_think_tokens(result["output"])
            self.messages.append({"role": "assistant", "content": result["output"]})
            for pub_name in self._string_publishers:
                self.publishers_dict[pub_name].publish(
                    **result, time_stamp=self.get_ros_time()
                )
        elif self._task == "pointing":
            for pub_name in self._poi_publishers:
                self.publishers_dict[pub_name].publish(
                    **result,
                    img=self._images[0],  # POI msg takes only one image
                    time_stamp=self.get_ros_time(),
                )
        elif self._task == "grounding":
            result["output"] = [
                {"bboxes": result["output"], "labels": [], "scores": []}
            ]
            for pub_name in self._detections_publishers:
                self.publishers_dict[pub_name].publish(
                    **result, images=self._images, time_stamp=self.get_ros_time()
                )
        elif self._task == "affordance":
            result["output"] = [
                {"bboxes": result["output"], "labels": [], "scores": []}
            ]
            for pub_name in self._detections_publishers:
                self.publishers_dict[pub_name].publish(
                    **result, images=self._images, time_stamp=self.get_ros_time()
                )
        elif self._task == "trajectory":
            for pub_name in self._poi_publishers:
                self.publishers_dict[pub_name].publish(
                    **result,
                    img=self._images[0],  # POI msg takes only one image
                    time_stamp=self.get_ros_time(),
                )

    def _execution_step(self, *args, **kwargs):
        """_execution_step.

        :param args:
        :param kwargs:
        """
        if not self._task:
            super()._execution_step(*args, **kwargs)
            return

        # If a task has been specified then handle it here
        if self.run_type is ComponentRunType.EVENT and (trigger := kwargs.get("topic")):
            if not trigger:
                return
            self.get_logger().debug(f"Received trigger on topic {trigger.name}")
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
        unpack = True if self._task != "general" else False
        result = self._call_inference(inference_input, unpack=unpack)

        # Publish results to output topics in accordance with the tasks
        if result:
            self._publish_task_specific_outputs(result)
            if result.get("thinking"):
                self.get_logger().info(f"<think>{result['thinking']}</think>")

    def _warmup(self):
        """Warm up and stat check"""
        import time
        from pathlib import Path
        import cv2

        image = cv2.imread(str(Path(__file__).parents[1] / Path("resources/test.jpeg")))

        message = {"role": "user", "content": "What do you see?"}
        inference_input = {
            "query": [message],
            "images": [image],
            **self.inference_params,
        }

        # Run inference once to warm up and once to measure time
        if self.model_client:
            self.model_client.inference(inference_input)
        elif hasattr(self, "local_model"):
            self.local_model(inference_input)

        inference_input = {
            "query": [message],
            "images": [image],
            **self.config._get_inference_params(),
        }
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


# Alias
VLM = MLLM
