from typing import Any, Union, Optional, List, Dict
import time
import queue
import threading
import os
import numpy as np
import cv2

from ..clients.model_base import ModelClient
from ..config import VisionConfig
from ..ros import (
    DetectionsMultiSource,
    Detections,
    Trackings,
    FixedInput,
    Image,
    RGBD,
    Topic,
    TrackingsMultiSource,
    ROSImage,
    ROSCompressedImage,
    component_action,
)
from ..utils import (
    validate_func_args,
    load_model,
    draw_points_2d,
    draw_detection_bounding_boxes,
)
from .model_component import ModelComponent
from .component_base import ComponentRunType


class Vision(ModelComponent):
    """
    This component performs object detection and tracking on input images and outputs a list of detected objects, along with their bounding boxes and confidence scores.

    :param inputs: The input topics for the object detection.
        This should be a list of Topic objects or FixedInput objects, limited to Image (or RGBD) type.
    :type inputs: list[Union[Topic, FixedInput]]
    :param outputs: The output topics for the object detection.
        This should be a list of Topic objects, Detection and Tracking types are handled automatically.
    :type outputs: list[Topic]
    :param model_client: Optional model client for the vision component to access remote vision models. If not provided, enable_local_classifier should be set to True in VisionConfig
        This should be an instance of ModelClient. Defaults to None.
    :type model_client: Optional[ModelClient]
    :param config: The configuration for the vision component.
        This should be an instance of VisionConfig. If not provided, defaults to VisionConfig().
    :type config: VisionConfig
    :param trigger: The trigger value or topic for the vision component.
        This can be a single Topic object, a list of Topic objects, or a float value for timed components.
    :type trigger: Union[Topic, list[Topic], float]
    :param component_name: The name of the vision component.
        This should be a string and defaults to "vision_component".
    :type component_name: str

    Example usage:
    ```python
    image_topic = Topic(name="image", msg_type="Image")
    detections_topic = Topic(name="detections", msg_type="Detections")
    config = VisionConfig()
    model_client = ModelClient(model=DetectionModel(name='yolov5'))
    vision_component = Vision(
        inputs=[image_topic],
        outputs=[detections_topic],
        model_client=model_client
        config=config,
        component_name = "vision_component"
    )
    ```
    """

    @validate_func_args
    def __init__(
        self,
        *,
        inputs: List[Union[Topic, FixedInput]],
        outputs: List[Topic],
        model_client: Optional[ModelClient] = None,
        config: Optional[VisionConfig] = None,
        trigger: Union[Topic, List[Topic], float] = 1.0,
        component_name: str,
        **kwargs,
    ):
        self.config: VisionConfig = config or VisionConfig()
        self.allowed_inputs = {"Required": [[Image, RGBD]]}
        self.handled_outputs = [
            Detections,
            Trackings,
            DetectionsMultiSource,
            TrackingsMultiSource,
        ]

        self._images: List[Union[np.ndarray, ROSImage, ROSCompressedImage]] = []

        super().__init__(
            inputs,
            outputs,
            model_client,
            self.config,
            trigger,
            component_name,
            **kwargs,
        )

        if model_client:
            # check for correct model and setup number of trackers to be initialized if any
            if model_client.model_type != "VisionModel":
                raise TypeError(
                    "A vision component can only be started with a Vision Model"
                )
            if (
                hasattr(model_client, "_model")
                and self.model_client._model.setup_trackers  # type: ignore
            ):
                model_client._model._num_trackers = len(inputs)
        else:
            if not self.config.enable_local_classifier:
                raise TypeError(
                    "Vision component either requires a model client or enable_local_classifier needs to be set True in the VisionConfig."
                )

    def custom_on_configure(self):
        # deploy local model if enabled
        if not self.model_client and self.config.enable_local_classifier:
            self._deploy_local_model()

        # configure parent component
        super().custom_on_configure()

        # create visualization thread if enabled
        if self.config.enable_visualization:
            self.queue = queue.Queue()
            self.stop_event = threading.Event()
            self.visualization_thread = threading.Thread(target=self._visualize)
            self.visualization_thread.start()

    def _deploy_local_model(self):
        """Deploy local vision model on demand."""
        if self.local_model is not None:
            return  # already deployed
        from ..utils.local_vision import LocalVisionModel, _MS_COCO_LABELS

        if not self.config.dataset_labels:
            self.get_logger().warning(
                "No dataset labels provided for the local model, using default MS_COCO labels"
            )
            self.config.dataset_labels = _MS_COCO_LABELS

        # Auto-enable config flag
        self.config.enable_local_classifier = True

        self.local_model = LocalVisionModel(
            model_path=load_model(
                "local_classifier", self.config.local_classifier_model_path
            ),
            ncpu=self.config.ncpu_local_classifier,
            device=self.config.device_local_classifier,
            input_height=self.config.input_height,
            input_width=self.config.input_width,
            dataset_labels=self.config.dataset_labels,
        )

    def custom_on_deactivate(self):
        # if visualization is enabled, shutdown the thread
        if self.config.enable_visualization:
            if self.visualization_thread:
                self.stop_event.set()
                self.visualization_thread.join()
        # deactivate component
        super().custom_on_deactivate()

    @component_action
    def take_picture(
        self,
        topic_name: str,
        save_path: str = "~/emos/pictures",
        timeout: float = 0.5,
    ) -> bool:
        """
        Take a picture from a specific input topic and save it to the specified location.

        This method acts as an Action to capture a specific frame from a specific camera/topic.
        It prioritizes triggers over standard inputs if a name conflict exists (though unique names are expected).

        :param topic_name: The name of the topic to capture the image from.
                           Must be one of the component's registered input topics.
        :type topic_name: str
        :param save_path: The directory path where images will be saved.
                          Defaults to "~/emos/pictures".
        :type save_path: str
        :param timeout: Timeout if an image is not available on the topic.
                          Defaults to 0.5 seconds.
        :type timeout: float
        :return: True if successful, False otherwise.
        :rtype: bool
        :raises ValueError: If the provided topic_name is not found in inputs.
        """
        try:
            # Preflight check for timed components
            if (
                self.run_type == ComponentRunType.TIMED
                and (loop_time := 1 / self.config.loop_rate) > timeout
            ):
                self.get_logger().warning(
                    f"Warning: take_picture timeout ({timeout}s) is strictly shorter than the component's trigger period ({loop_time}s) for this timed component. "
                    f"The action is highly likely to timeout before the image callback executes. Consider running the component faster or increasing the timeout for this action."
                )
            # Expand user path
            save_path = os.path.expanduser(save_path)
            os.makedirs(save_path, exist_ok=True)

            # Identify callback type
            trig_dict = getattr(self, "trig_callbacks", {})
            target_callback = trig_dict.get(topic_name) or self.callbacks.get(
                topic_name
            )
            if not target_callback:
                self.get_logger().error(
                    f"Topic '{topic_name}' is not one of the component inputs. You can only take pictures on topics that are provided as inputs to this component."
                )
                return False

            # if target is a trigger, issue a warning
            is_trigger = topic_name in trig_dict
            if is_trigger:
                self.get_logger().warning(
                    f"Capturing image from trigger '{topic_name}'. Inference paused momentarily."
                )
            # save callback state
            original_callback = target_callback._extra_callback
            original_get_processed = target_callback._get_processed

            # Define a single frame interceptor function
            frames = []

            # extra callback for capturing image
            def single_frame_interceptor(msg, topic, output=None):
                if output is not None and not frames:
                    frames.append(output.copy())

            # Swap extracallback, wait and restore
            try:
                target_callback.on_callback_execute(
                    single_frame_interceptor, get_processed=True
                )

                start_time = time.time()
                while (time.time() - start_time) < timeout and not frames:
                    time.sleep(0.01)  # Check frequently

            finally:
                # Always restore the original callback state
                if original_callback:
                    target_callback.on_callback_execute(
                        original_callback, get_processed=original_get_processed
                    )
                else:
                    target_callback._extra_callback = None

            if not frames:
                self.get_logger().warning(
                    f"Timeout: No image received on '{topic_name}'."
                )
                return False

            # Save Image
            timestamp = int(time.time() * 1000)
            filename = f"capture_{topic_name}_{timestamp}.jpg"
            full_path = os.path.join(save_path, filename)

            # Ensure BGR for OpenCV saving
            save_img = cv2.cvtColor(frames[0], cv2.COLOR_RGB2BGR)
            cv2.imwrite(full_path, save_img)
            self.get_logger().info(f"Saved picture to {full_path}")

            return True

        except Exception as e:
            self.get_logger().error(f"Failed to take picture: {e}")
            return False

    @component_action
    def record_video(
        self,
        topic_name: str,
        duration: float = 5.0,
        save_path: str = "~/emos/videos",
        fps: int = 30,
    ) -> bool:
        """
        Record a video from a specific input topic for a set duration.

        This action spawns a background thread to capture frames and save them to a video file.
        It does not block the main execution loop.

        :param topic_name: The name of the topic to record from.
        :type topic_name: str
        :param duration: The duration of the recording in seconds. Defaults to 5.0.
        :type duration: float
        :param save_path: The directory path where the video will be saved.
                          Defaults to "~/emos/videos".
        :type save_path: str
        :param fps: The frames per second for the recording. Defaults to 20.
        :type fps: int
        :return: True if the recording thread started successfully, False otherwise.
        :rtype: bool
        :raises ValueError: If the topic_name is not registered.
        """
        try:
            # Preflight checks for timed components
            if self.run_type == ComponentRunType.TIMED:
                if self.config.loop_rate < fps:
                    self.get_logger().warning(
                        f"Warning: Requested {fps} FPS, but the component's trigger period is {1 / self.config.loop_rate}s "
                        f"(~{self.config.loop_rate:.2f} FPS max). The recorded video will heavily repeat frames or play too fast. Consider running the component faster or reduce the fps"
                    )

                if duration < 1 / self.config.loop_rate:
                    self.get_logger().warning(
                        f"Warning: Recording duration ({duration}s) is shorter than the component's loop period "
                        f"({1 / self.config.loop_rate}s). You are likely to capture 0 frames. Consider running the component faster or increase duration."
                    )
            # Expand user path
            save_path = os.path.expanduser(save_path)
            os.makedirs(save_path, exist_ok=True)

            trig_dict = getattr(self, "trig_callbacks", {})
            target_callback = trig_dict.get(topic_name) or self.callbacks.get(
                topic_name
            )
            # Identify callback type
            if not target_callback:
                self.get_logger().error(
                    f"Topic '{topic_name}' is not one of the component inputs. You can only record videos on topics that are provided as inputs to this component."
                )
                return False

            # if target is a trigger, issue a warning
            is_trigger = topic_name in trig_dict
            if is_trigger:
                self.get_logger().warning(
                    f"Recording video on trigger topic '{topic_name}'. "
                    f"Detection or tracking will be PAUSED for {duration} seconds!"
                )

            # Spawn the background thread
            recording_thread = threading.Thread(
                target=self._record_video_thread,
                kwargs={
                    "target_callback": target_callback,
                    "topic_name": topic_name,
                    "duration": duration,
                    "save_path": save_path,
                    "fps": fps,
                    "is_trigger": is_trigger,
                },
                daemon=True,
            )
            recording_thread.start()
            self.get_logger().info(
                f"Started recording video on topic '{topic_name}' for {duration} seconds."
            )

            return True

        except Exception as e:
            self.get_logger().error(f"Failed to start recording: {e}")
            return False

    def _record_video_thread(
        self,
        target_callback,
        topic_name: str,
        duration: float,
        save_path: str,
        fps: int,
        is_trigger: bool,
    ):
        """
        Internal worker thread to buffer frames and write video to disk.
        """
        frames = []

        # Save current callback state
        original_callback = target_callback._extra_callback
        original_get_processed = target_callback._get_processed

        # extra callback for capturing images
        def frame_interceptor(msg, topic, output=None):
            if output is not None:
                frames.append(output.copy())

        try:
            target_callback.on_callback_execute(frame_interceptor, get_processed=True)
            time.sleep(duration)
        finally:
            # Safely restore execution step or original state
            if original_callback:
                target_callback.on_callback_execute(
                    original_callback, get_processed=original_get_processed
                )
                if is_trigger:
                    self.get_logger().info(
                        f"Video recording finished. Vision inference RESUMED on '{topic_name}'."
                    )
            else:
                target_callback._extra_callback = None

        if not frames:
            self.get_logger().warning(
                f"No frames captured for video on topic '{topic_name}'."
            )
            return

        # Encode video
        timestamp = int(time.time() * 1000)
        filename = f"recording_{topic_name}_{timestamp}.mp4"
        full_path = os.path.join(save_path, filename)

        height, width, _ = frames[0].shape
        fourcc = cv2.VideoWriter_fourcc(*"mp4v")
        out = cv2.VideoWriter(full_path, fourcc, fps, (width, height))

        # Subsample frames to match requested FPS
        actual_frames = len(frames)
        target_frames = int(duration * fps)

        if actual_frames > 0:
            step = max(1, actual_frames / target_frames)
            for i in range(target_frames):
                idx = int(i * step)
                if idx < actual_frames:
                    bgr_frame = cv2.cvtColor(frames[idx], cv2.COLOR_RGB2BGR)
                    out.write(bgr_frame)

        out.release()
        self.get_logger().info(f"Video saved successfully: {full_path}")

    def _visualize(self):
        """CV2 based visualization of inference results"""
        cv2.namedWindow(self.node_name)

        while not self.stop_event.is_set():
            try:
                # Add timeout to periodically check for stop event
                data = self.queue.get(timeout=1)
            except queue.Empty:
                self.get_logger().warning(
                    "Visualization queue is empty, waiting for new data..."
                )
                continue

            # Only handle the first image and its output
            image = cv2.cvtColor(
                data["images"][0], cv2.COLOR_RGB2BGR
            )  # as cv2 expects a BGR

            bounding_boxes = data["output"][0].get("bboxes", [])
            labels = data["output"][0].get("labels", [])
            tracked_objects = data["output"][0].get("tracked_points", [])

            image = draw_detection_bounding_boxes(
                image, bounding_boxes, labels, handle_bbox2d_msg=False
            )

            for point_list in tracked_objects:
                # Each point_list is a list of points on one tracked object
                image = draw_points_2d(image, point_list)

            cv2.imshow(self.node_name, image)

            if cv2.waitKey(1) & 0xFF == ord("q"):
                self.get_logger().warning("User pressed 'q', stopping visualization.")
                break

        cv2.destroyAllWindows()

    def _create_input(self, *_, **kwargs) -> Optional[Dict[str, Any]]:
        """Create inference input for ObjectDetection models
        :param args:
        :param kwargs:
        :rtype: dict[str, Any]
        """
        self._images = []
        # set one image topic as query for event based trigger
        if trigger := kwargs.get("topic"):
            if msg := self.trig_callbacks[trigger.name].msg:
                self._images.append(msg)
            images = [self.trig_callbacks[trigger.name].get_output(clear_last=True)]
        else:
            images = []

            for i in self.callbacks.values():
                msg = i.msg
                if (item := i.get_output(clear_last=True)) is not None:
                    images.append(item)
                    if msg:
                        self._images.append(msg)

        if not images:
            return None

        return {"images": images, **self.inference_params}

    def _execution_step(self, *args, **kwargs):
        """_execution_step.

        :param args:
        :param kwargs:
        """

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
        result = self._call_inference(inference_input, unpack=True)
        if not result:
            return

        # result acquired, publish inference result
        self._publish(
            result,
            images=self._images,
            time_stamp=self.get_ros_time(),
        )
        if self.config.enable_visualization:
            result["images"] = inference_input["images"]
            self.queue.put_nowait(result)

    def _warmup(self):
        """Warm up and stat check"""
        import time
        from pathlib import Path

        if (
            hasattr(self, "trig_callbacks")
            and (image := list(self.trig_callbacks.values())[0].get_output())
            is not None
        ):
            self.get_logger().warning("Got image input from trigger topic")
        else:
            self.get_logger().warning(
                "Did not get image input from trigger topic. Camera device might not be working and topic is not being published to, using a test image."
            )
            image = cv2.imread(
                str(Path(__file__).parents[1] / Path("resources/test.jpeg"))
            )

        inference_input = {"images": [image], **self.inference_params}

        # Run inference once to warm up and once to measure time
        if self.model_client:
            self.model_client.inference(inference_input)
        elif self.local_model:
            self.local_model(inference_input)

        start_time = time.time()
        if self.model_client:
            result = self.model_client.inference(inference_input)
            elapsed_time = time.time() - start_time
            self.get_logger().warning(f"Model Output: {result}")
            self.get_logger().warning(
                f"Approximate Inference time: {elapsed_time} seconds"
            )
            self.get_logger().warning(
                f"Max throughput: {1 / elapsed_time} frames per second"
            )
        elif self.local_model:
            result = self.local_model(inference_input)
            elapsed_time = time.time() - start_time
            self.get_logger().warning(f"Model Output: {result}")
            self.get_logger().warning(
                f"Approximate Inference time: {elapsed_time} seconds"
            )
            self.get_logger().warning(
                f"Max throughput: {1 / elapsed_time} frames per second"
            )
        else:
            result = "Component was run without a client. Did not execute warmup"
