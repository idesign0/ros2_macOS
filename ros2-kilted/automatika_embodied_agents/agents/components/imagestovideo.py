import math
from typing import Optional, Union, List

import cv2
import numpy as np

from ..config import VideoMessageMakerConfig
from ..ros import Image, Topic, Video, ROSImage, ROSCompressedImage
from ..utils import validate_func_args
from .component_base import Component


class VideoMessageMaker(Component):
    """
    This component generates ROS video messages from input image messages. A video message is a collection of image messages that have a perceivable motion.
    I.e. the primary task of this component is to make intentionality decisions about what sequence of consecutive images should be treated as one coherent temporal sequence.
    The motion estimation method used for selecting images for a video can be configured in component config.

    :param inputs: The input topics for the object detection.
        This should be a list of Topic objects or FixedInput objects, limited to Image type.
    :type inputs: list[Topic]
    :param outputs: The output topics for the object detection.
        This should be a list of Topic objects, Video type.
    :type outputs: list[Topic]
    :param config: The configuration for the video message generation.
        This should be an instance of VideoMessageMakerConfig.
    :type config: VideoMessageMakerConfig
    :param trigger: The trigger value or topic for the object detection.
        This can be a single Topic object or a list of Topic objects.
    :type trigger: Union[Topic, list[Topic]]
    :param component_name: The name of the video message generation component.
        This should be a string.
    :type component_name: str

    Example usage:
    ```python
    image_topic = Topic(name="image", msg_type="Image")
    video_topic = Topic(name="video", msg_type="Video")
    config = VideoMessageMakerConfig()
    video_message_maker = VideoMessageMaker(
        inputs=[image_topic],
        outputs=[video_topic],
        config=config,
        component_name="video_message_maker",
    )
    ```
    """

    @validate_func_args
    def __init__(
        self,
        *,
        inputs: List[Topic],
        outputs: List[Topic],
        config: Optional[VideoMessageMakerConfig] = None,
        trigger: Union[Topic, List[Topic]],
        component_name: str,
        **kwargs,
    ):
        if isinstance(trigger, float):
            raise TypeError(
                "VideoMessageMaker component needs to be given a valid trigger topic. It cannot be started as a timed component."
            )

        self.config: VideoMessageMakerConfig = config or VideoMessageMakerConfig()
        self.allowed_inputs = {"Required": [Image]}
        self.allowed_outputs = {"Required": [Video]}

        super().__init__(
            inputs,
            outputs,
            self.config,
            trigger,
            component_name,
            **kwargs,
        )

        self._frames: Union[List[ROSImage], List[ROSCompressedImage]] = []
        self._last_frame: Optional[np.ndarray] = None
        self._capture: bool = False

    def _motion_estimation(self, current_frame: np.ndarray) -> bool:
        """Motion estimation methods between two frames.
        :param current_frame:
        :type current_frame: np.ndarray
        :rtype: bool
        """
        # get gray scale image
        gray = cv2.cvtColor(current_frame, cv2.COLOR_RGB2GRAY)
        if self.config.motion_estimation_func == "frame_difference":
            return self._frame_difference(gray, self.config.threshold)
        elif self.config.motion_estimation_func == "optical_flow":
            return self._optical_flow(
                gray, self.config.threshold, **self.config.flow_kwargs
            )
        else:
            return True

    def _frame_difference(self, img: np.ndarray, threshold: float) -> bool:
        """Calculates difference between two frames and returns true
        if difference is greater than defined threshold.
        :param img:
        :type img: np.ndarray
        :param threshold:
        :type threshold: int
        :rtype: bool
        """
        # calculate frame difference
        diff = cv2.subtract(img, self._last_frame)
        # apply blur to improve thresholding
        diff = cv2.medianBlur(diff, 3)
        # apply adaptive thresholding
        mask = cv2.adaptiveThreshold(
            diff, 1, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY_INV, 11, 2
        )
        return True if mask.sum() > (threshold * math.prod(img.shape) / 100) else False

    def _optical_flow(self, img: np.ndarray, threshold: float, **kwargs) -> bool:
        """Calculates optical flow between two frames and returns true
        if flow is greater than defined threshold.
        :param img:
        :type img: np.ndarray
        :param threshold:
        :type threshold: int
        :rtype: bool
        """
        # calculate optical flow
        flow = cv2.calcOpticalFlowFarneback(self._last_frame, img, None, **kwargs)
        mask = np.uint8(flow > 1) / 10
        return True if mask.sum() > (threshold * math.prod(img.shape) / 100) else False

    def _execution_step(self, *_, **kwargs) -> None:
        """Collects incoming image messages until a criteria is met
        When met, publishes image messages as video
        :param args:
        :param kwargs:
        """
        msg = kwargs.get("msg")
        topic = kwargs.get("topic")
        if msg and topic:
            output = self.trig_callbacks[topic.name].get_output()
            if self._last_frame is not None:
                # calculate motion estimation for start and stop
                self._capture = (
                    True
                    if self._motion_estimation(output)
                    and len(self._frames) < self.config.max_video_frames
                    else False
                )
                if self._capture:
                    self._frames.append(msg)
            self._last_frame = cv2.cvtColor(output, cv2.COLOR_RGB2GRAY)

        # publish if video capture finished
        if (
            self.publishers_dict
            and (not self._capture)
            and len(self._frames) >= self.config.min_video_frames
        ):
            self.get_logger().debug(f"Sending out video of {len(self._frames)} frames")
            for publisher in self.publishers_dict.values():
                publisher.publish(output=self._frames)
            self._frames = []
