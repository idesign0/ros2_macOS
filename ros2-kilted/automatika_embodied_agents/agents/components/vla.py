from typing import Optional, List, Dict, Callable, Literal
import inspect
from functools import partial, wraps
import queue
import threading
import numpy as np
from rclpy.logging import get_logger

try:
    from pynput import keyboard
except ImportError:
    keyboard = None

from ..config import VLAConfig
import time
from ..ros import (
    Event,
    Action,
    RGBD,
    Image,
    Topic,
    JointTrajectory,
    JointJog,
    JointState,
    ComponentRunType,
    MutuallyExclusiveCallbackGroup,
    VisionLanguageAction,
    run_external_processor,
)
from ..utils import validate_func_args, find_missing_values
from ..utils.actions import (
    JointsData,
    parse_urdf_joints,
    check_joint_limits,
    cap_actions_with_limits,
    create_observation_spec,
    validate_mapping_completeness,
)
from ..clients.lerobot import LeRobotClient
from .model_component import ModelComponent


class VLA(ModelComponent):
    """Vision-Language-Agent (VLA) Component."""

    @validate_func_args
    def __init__(
        self,
        *,
        inputs: List[Topic],
        outputs: List[Topic],
        model_client: LeRobotClient,
        config: VLAConfig,
        component_name: str,
        **kwargs,
    ):
        self.config: VLAConfig = config
        self.allowed_inputs = {
            "Required": [JointState, [Image, RGBD]],
        }
        self.handled_outputs = [JointState, JointTrajectory, JointJog]

        self.model_client = model_client

        # Verify config and model definition
        # Dataset definition of actions
        self._dataset_action_dtype = None
        self._dataset_sorted_joint_names = None
        # Joint Limits
        self.robot_joints_limits = None
        # Make verifications on init, skip after serialization
        if hasattr(self.model_client, "_model"):
            self._verify_config(component_name)

        # queue aggregation function
        self._aggregator_function: Callable = lambda _, y: y

        # Set the component to run as an action server and set the main action type
        self.run_type = ComponentRunType.ACTION_SERVER

        # TODO: Trigger will be passed by serialized component in the kwargs
        # We will remove it here. This can be improved by passing kwargs differently
        if "trigger" in list(kwargs.keys()):
            kwargs.pop("trigger")

        super().__init__(
            inputs=inputs,
            outputs=outputs,
            model_client=model_client,
            trigger=None,
            config=self.config,
            component_name=component_name,
            main_action_type=VisionLanguageAction,
            **kwargs,
        )

    def custom_on_activate(self):
        """Custom activation"""

        if not isinstance(self.model_client, LeRobotClient):
            raise TypeError(
                "Currently VLA component only takes in LeRobotClient. Please use LeRobot Policy Server to serve your VLA."
            )

        if self.config.warmup:
            # TODO: warmup with lerobot client
            self.get_logger().warning(
                "Warmup cannot not be called with LeRobot client."
            )
            self.config.warmup = False

        # Activate component and initialize client
        super().custom_on_activate()

        # Queue for receiving actions
        self._actions_received = queue.Queue()
        self._action_queue_lock = threading.Lock()

        # track last executed action timestep
        self._last_executed_timestep_lock = threading.Lock()
        self._last_executed_timestep = -1

        # track task status
        self._task_completed = False

        # Action timers
        self.__action_sending_timer = None
        self.__action_receiving_timer = None

        # Look for state topic
        self._state_topic = None
        for key, callback in self.callbacks.items():
            if callback.input_topic.msg_type == JointState:
                self._state_topic = key
                break
        if not self._state_topic:
            raise RuntimeError(
                "Could not find a topic of type JointState. VLA component needs at least one topic of type JointState as input."
            )

        # Assign external aggregator function in case its provided
        if agg_fun := self._external_processors.get("aggregator_function", None):
            # Get the first element of the tuple and the only function in that list
            self._aggregator_function = partial(
                run_external_processor,
                logger_name=self.node_name,
                topic_name="aggregator_function",
                processor=agg_fun[0][0],
            )

    def custom_on_deactivate(self):
        """Custom deactivation"""

        # Mark any pendings actions as finished
        while not self._actions_received.empty():
            try:
                self._actions_received.get_nowait()
                self._actions_received.task_done()
            except queue.Empty:
                break

        # Deactivate component
        super().custom_on_deactivate()

    def set_termination_trigger(
        self,
        mode: Literal["timesteps", "keyboard", "event"] = "timesteps",
        max_timesteps: int = 100,
        stop_key: str = "q",
        stop_event: Optional[Event] = None,
    ):
        """
        Set the condition used to determine when an action is done.

        :param mode: One of 'timesteps', 'keyboard', 'event'.
        :param max_timesteps: The number of timesteps after which to stop (used if mode='timesteps' or 'event').
        :param stop_key: The key to press to stop the action (used if mode='keyboard').
        """
        valid_modes = ["timesteps", "keyboard", "event"]
        if mode not in valid_modes:
            raise ValueError(f"Termination mode must be one of {valid_modes}")

        self.config._termination_mode = mode
        get_logger(self.node_name).info(
            f"Action termination configured for mode: {mode}"
        )

        if mode == "timesteps":
            self.config._termination_timesteps = max_timesteps
            get_logger(self.node_name).info(
                f"Action will terminate after {max_timesteps} timesteps."
            )

        elif mode == "keyboard":
            if keyboard is None:
                raise RuntimeError(
                    "pynput is required for keyboard based termination. Its either not installed or has thrown an error because you might be on an ssh session. Install the package with `pip install pynput` and test it with python3 -c 'import pynput'."
                )

            self.config._termination_key = stop_key

        elif mode == "event":
            if stop_event is None:
                raise ValueError(
                    "A stop_event must be provided when setting the termination mode to `event`"
                )
            get_logger(self.node_name).info(
                f"Action will terminate on {stop_event.name} event or after {max_timesteps}"
            )
            self.config._termination_timesteps = max_timesteps
            self._add_event_action_pair(stop_event, Action(self.signal_done))

    def signal_done(self):
        """Signals that the action is complete.
        Can be used as an action for signaled events"""
        self._task_completed = True
        self.get_logger().info("Action completion signaled")

    def _on_key_press(self, key):
        """Callback for keyboard listener."""
        try:
            # Check for char keys
            if hasattr(key, "char") and key.char == self.config._termination_key:
                self.signal_done()
            # Check for special keys (e.g. Esc) if configured as such
            elif hasattr(key, "name") and key.name == self.config._termination_key:
                self.signal_done()
        except AttributeError:
            pass

    def _verify_config(self, component_name: str):
        """Run checks on provided config and model definition"""
        logger = get_logger(component_name)

        # Initialize dataset features if missing
        # TODO: Make prefix and image shape config params
        if not self.model_client._model._features:
            self.model_client._model._features = create_observation_spec(
                self.config.joint_names_map,
                self.config.camera_inputs_map,
                prefix="observation",
                image_shape=(480, 640, 3),
            )
            logger.warning(
                "You have not provided a dataset file for the model. Feature specification are required for initializing the policy on LeRobot Policy Server. We are going to auto-generate a feature spec from `joint_names_map` and `camera_inputs_map` that you provided. Please make sure their keys correspond to the names of features and actions used when training the model. Policy init might fail."
            )
            return

        # Verify Joint Keys
        dataset_joint_keys = self.model_client._model._joint_keys
        validate_mapping_completeness(
            target_keys=dataset_joint_keys,
            mapped_keys=self.config.joint_names_map.keys(),
            logger=logger,
            missing_data_msg="Dataset metadata for 'joint names' is missing. Skipping validation of 'joint_names_map'.",
            error_msg="Your 'joint_names_map' in VLAConfig does not map all the dataset joint names to the robot joint names correctly. The following joint names from the dataset info are unmapped: {missing}",
        )

        # Process Action Keys
        dataset_action_keys = self.model_client._model._actions
        if dataset_action_keys:
            dataset_joint_names = dataset_action_keys.get("names", None)
            if dataset_joint_names:
                self._dataset_sorted_joint_names = [
                    self.config.joint_names_map[j] for j in dataset_joint_names
                ]
            self._dataset_action_dtype = dataset_action_keys.get("dtype", None)
        else:
            logger.warning(
                "Dataset metadata for 'actions' is missing. Actions being sent out to the robot will be sorted the same as the order of joint_names_map keys."
            )

        # Setup and Verify Joint Limits
        self.robot_joints_limits = self.__resolve_joint_limits(logger)
        if self.robot_joints_limits:
            ok, errors = check_joint_limits(
                self.robot_joints_limits, requirements=[self.config.state_input_type]
            )
            if not ok:
                error_str = "\n".join(errors)
                logger.warning(
                    f"The following limits were not provided for joints. Consider adding them in config:\n{error_str}"
                )

        # TODO:: Handle partially available image keys with error logging
        # Remove LeRobot specific prefix in case it has been added by the user
        self.config.camera_inputs_map = {
            k.removeprefix("observation.images."): v
            for k, v in self.config.camera_inputs_map.items()
        }

        # Verify Image Keys
        dataset_image_keys = self.model_client._model._image_keys
        validate_mapping_completeness(
            target_keys=dataset_image_keys,
            mapped_keys=self.config.camera_inputs_map.keys(),
            logger=logger,
            missing_data_msg="Dataset metadata for 'images' is missing. Skipping validation of 'camera_inputs_map'.",
            error_msg="Your 'camera_inputs_map' in VLAConfig does not map all the dataset camera names to the robot camera topics correctly. The following camera names from the dataset info are unmapped: {missing}",
        )

    def __resolve_joint_limits(self, logger) -> Optional[Dict]:
        """Determines the source of joint limits (URDF or Config) and returns them."""
        if self.config.robot_urdf_file:
            limits = parse_urdf_joints(self.config.robot_urdf_file)

            # Validate URDF joints against config map
            missing_in_urdf = find_missing_values(
                limits.keys(), list(self.config.joint_names_map.values())
            )
            if missing_in_urdf:
                logger.warning(
                    f"Your 'joint_names_map' includes robot joint names not found in the URDF. "
                    f"Missing: {missing_in_urdf}. Available in URDF: {list(limits.keys())}"
                )
            return limits

        # Fallback to config limits
        if not self.config.joint_limits:
            logger.warning(
                "No URDF file provided. Consider adding 'joint_limits' to config to ensure safe movement execution."
            )
            return None

        return self.config.joint_limits

    def _receive_actions_from_client(self):
        """Timer callback for continuously receiving actions from client"""
        latest_actions = self.model_client.receive_actions()
        if latest_actions:
            self._update_actions_queue(latest_actions)

    def _update_actions_queue(
        self,
        new_actions: List,
    ):
        """Update actions queue with new result. Similar to LeRobot async_client implementation"""

        with self._actions_received.mutex:
            # Get internal deque
            internal_deque = self._actions_received.queue

            # Mapping: timestep -> TimedAction object
            action_map = {a.timestep: a for a in internal_deque}

            # Process new actions
            queue_modified = False

            for new_act in new_actions:
                ts = new_act.timestep

                # Get last executed action
                with self._last_executed_timestep_lock:
                    _latest_action = self._last_executed_timestep

                # Skip: actions that have already passed
                if ts <= _latest_action:
                    continue

                queue_modified = True

                # Aggregate: If timestep exists, merge arrays
                if ts in action_map:
                    existing_act = action_map[ts]

                    # Perform the aggregation on the array
                    merged_array = self._aggregator_function(
                        existing_act.action, new_act.action
                    )

                    # Update the existing action object
                    existing_act.action = merged_array
                    existing_act.timestamp = new_act.timestamp

                # Insert: If timestep is new, add to map
                else:
                    action_map[ts] = new_act

            # Rebuild the Queue only if data changed
            if queue_modified:
                # Clear the internal deque
                internal_deque.clear()

                # Sort by timestep to ensure FIFO execution order
                sorted_timesteps = sorted(action_map.keys())

                # Bulk extend the deque
                internal_deque.extend([action_map[k] for k in sorted_timesteps])

                # Manually notify any consumers
                self._actions_received.not_empty.notify()

    def _create_input(self, task: str) -> Optional[Dict]:
        """Prepare observations from current inputs

        :param task: Task string
        :return: Inference input if inference input is available, None otherwise
        """
        if not self._state_topic:
            return
        joint_state: JointsData = self.callbacks[self._state_topic].get_output()

        # map robot state to dataset keys
        mapped_state = joint_state.get_mapped_state(
            self.config.state_input_type, self.config.joint_names_map
        )

        # Return if no mapped state found
        if not mapped_state:
            self.get_logger().warning(
                f"Did not receive all joint states of type: {self.config.state_input_type}, not sending input for inference"
            )
            return

        # Get images
        images = {}
        for key, value in self.config.camera_inputs_map.items():
            img_out = self.callbacks[value["name"]].get_output(clear_last=True)
            if img_out is None:
                self.get_logger().warning(
                    f"Did not receive an image for topic: {value['name']}, not sending input for inference"
                )
                return
            images[key] = img_out

        # Combine state, images and task
        inference_input = {
            "timestamp": time.time(),
            **mapped_state,
            **images,
            "task": task,
        }
        return inference_input

    def _send_action_commands(self):
        """Send action commands"""
        # Pop an action from queue action, return if queue is empty
        with self._action_queue_lock:
            if not self._actions_received.empty():
                # Return the immediate next action
                action_to_pub = self._actions_received.get()
                # TODO: Remove torch dependency here once server can send numpy arrays
                action_to_pub_data = action_to_pub.action.numpy()
            else:
                return

        # Create publishing action
        action_data = JointsData(
            joints_names=self._dataset_sorted_joint_names
            or list(self.config.joint_names_map.values()),
        )

        # Cap actions within limits
        safe_action = (
            cap_actions_with_limits(
                action_data.joints_names,
                action_to_pub_data,
                self.robot_joints_limits,
                self.config.action_output_type,
                self.node_name,
            )
            if self.robot_joints_limits
            else action_to_pub_data
        )

        # TODO: Add smoothing for bigger deltas between new action and currect state

        # Set appropriate values based on output type
        setattr(
            action_data,
            self.config.action_output_type,
            safe_action.astype(np.float64),
        )

        # Publish action
        self._publish(result={"output": action_data})

        # Update the last executed timestep
        with self._last_executed_timestep_lock:
            self._last_executed_timestep = action_to_pub.timestep

    def _action_cleanup(self):
        """Destroy action timers and other listeners"""

        if self.__action_sending_timer is not None:
            self.destroy_timer(self.__action_sending_timer)
            self.__action_sending_timer = None
        if self.__action_receiving_timer is not None:
            self.destroy_timer(self.__action_receiving_timer)
            self.__action_receiving_timer = None

        # Cleanup keyboard listener
        if (
            self.config._termination_mode == "keyboard"
            and self._keyboard_listener is not None
        ):
            self._keyboard_listener.stop()
            self._keyboard_listener = None

        # Cleanup action queue
        with self._action_queue_lock:
            self._actions_received.queue.clear()

        with self._last_executed_timestep_lock:
            self._last_executed_timestep = -1

        # reset task status
        self._task_completed = False

    def set_aggregation_function(
        self, agg_fn: Callable[[np.ndarray, np.ndarray], np.ndarray]
    ):
        """
        Set the aggregation function to be used for aggregating generated actions from the robot policy model

        :param agg_fn: A callable that takes two numpy arrays as input and returns a single numpy array.
        :type agg_fn: Callable[[np.ndarray, np.ndarray], np.ndarray]

        :raises TypeError: If `agg_fn` is not a callable or does not match the expected signature.
        """
        if not callable(agg_fn):
            raise TypeError(
                "Aggregation function has to be a callable that takes as input two numpy arrays and returns a single numpy array"
            )

        # Check if the function has exactly two arguments
        sig = inspect.signature(agg_fn)
        params = list(sig.parameters.values())

        if len(params) != 2 or any(
            param.annotation not in (np.ndarray, inspect.Parameter.empty)
            for param in params
        ):
            raise TypeError(
                "Aggregation function must have exactly two parameters, both expected to be numpy arrays."
            )

        # Closure for using external functions
        def agg_closure(func: Callable[[np.ndarray, np.ndarray], np.ndarray]):
            """Wrapper for aggregator function"""

            @wraps(func)
            def _wrapper(*, x: np.ndarray, y: np.ndarray):
                """_wrapper"""
                out = func(x, y)
                # Check if we have action dtype
                # type check agg fn output, if incorrect, raise an error
                if type(out) is not np.ndarray:
                    raise TypeError(
                        "Only numpy arrays are acceptable as outputs of aggregator functions."
                    )
                elif (
                    self._dataset_action_dtype
                    and out.dtype != self._dataset_action_dtype
                ):
                    raise TypeError(
                        f"Only numpy arrays of dtype {self._dataset_action_dtype} are acceptable as outputs of aggregator functions."
                    )

            _wrapper.__name__ = func.__name__
            return _wrapper

        self._external_processors["aggregator_function"] = (
            [agg_closure(agg_fn)],
            "aggregator_function",
        )

    def main_action_callback(self, goal_handle: VisionLanguageAction.Goal):
        """
        Callback for the VLA main action server

        :param goal_handle: Incoming action goal
        :type goal_handle: VisionLanguageAction.Goal

        :return: Action result
        :rtype: VisionLanguageAction.Result
        """
        # Clear the action queue
        self._actions_received.queue.clear()

        # Get request
        task: str = goal_handle.request.task

        # Setup feedback of the action
        task_feedback_msg = VisionLanguageAction.Feedback()

        # Setup response of the action
        task_result = VisionLanguageAction.Result()
        task_result.success = False

        # Add keyboard listener if required by termination config
        if self.config._termination_mode == "keyboard":
            self._keyboard_listener = keyboard.Listener(on_press=self._on_key_press)
            self._keyboard_listener.start()
            self.get_logger().info(
                f"Listening for stop key: '{self.config._termination_key}'"
            )

        # Create a timer to send actions to the robot at a fixed rate
        self.__action_sending_timer = self.create_timer(
            timer_period_sec=1 / self.config.action_sending_rate,
            callback=self._send_action_commands,
        )

        # Create a tight timer for receiving actions from lerobot client
        self.__action_receiving_timer = self.create_timer(
            timer_period_sec=0.001,
            callback=self._receive_actions_from_client,
            callback_group=MutuallyExclusiveCallbackGroup(),
        )
        self.get_logger().debug("Started timer for receiving actions")

        # Wait for all inputs to be available
        _timeout = 0.0
        while (
            not self.got_all_inputs()
            and _timeout < self.config.input_timeout
            and not goal_handle.is_cancel_requested
        ):
            self.get_logger().warning(
                f"Inputs topics {self.get_missing_inputs()} are not available, waiting to start executing actions...",
                once=True,
            )
            _timeout += 1 / self.config.loop_rate
            time.sleep(1 / self.config.loop_rate)

        if _timeout >= self.config.input_timeout:
            self.get_logger().error(
                "Inputs were not received within the specified timeout period, aborting action."
            )
            self._action_cleanup()
            with self._main_goal_lock:
                goal_handle.abort()
                return task_result

        try:
            while not self._action_done():
                start_time = time.perf_counter()
                # Check if goal is canceled
                if not goal_handle.is_active or goal_handle.is_cancel_requested:
                    self._action_cleanup()
                    self.get_logger().info("Goal Canceled")
                    return task_result

                # Get new observations from inputs
                model_observations = self._create_input(task)

                # TODO: Add condition for sending based on queue consumption
                if model_observations:
                    # Get last executed timestep
                    with self._last_executed_timestep_lock:
                        last_timestep = self._last_executed_timestep
                    # Add last executed action timestep
                    model_observations["timestep"] = last_timestep
                    # send input for inference
                    self.model_client.inference(model_observations)
                else:
                    self.get_logger().warning(
                        "Could not prepare inference input, skipping this step..."
                    )
                    continue

                # Compute errors and publish feedback
                task_feedback_msg.timestep = last_timestep
                task_feedback_msg.completed = self._task_completed
                goal_handle.publish_feedback(task_feedback_msg)
                self.get_logger().debug(f"Action Feedback: {task_feedback_msg}")

                # Add sleep time to dynamically adjust loop_rate of action server
                # NOTE: using Python time directly, as ros rate sleep (from self.create_rate) was not functioning as expected
                time.sleep(
                    max(
                        0,
                        (1 / self.config.loop_rate)
                        - (time.perf_counter() - start_time),
                    )
                )

        except Exception as e:
            self.get_logger().error(f"Action execution error - {e}")
            with self._main_goal_lock:
                self._action_cleanup()
                goal_handle.abort()
                return task_result

        if self._task_completed:
            # Task was successful
            task_result.success = True

            # Action cleanup
            with self._main_goal_lock:
                self._action_cleanup()
                goal_handle.succeed()

        return task_result

    def _action_done(self) -> bool:
        """Check if action is done based on configuration"""

        # External Event or Keyboard signal (sets _task_completed to True)
        if self._task_completed:
            return True

        with self._last_executed_timestep_lock:
            current_ts = self._last_executed_timestep

        # Check if we exceeded the max steps
        if current_ts >= self.config._termination_timesteps:
            self.get_logger().info(
                f"Reached max timesteps ({self.config._termination_timesteps})."
            )
            # Timestep limit logic
            if self.config._termination_mode in ["timesteps"]:
                self.get_logger().info("Action done because we reached max timesteps.")
                self._task_completed = True
                return True
            else:
                self.get_logger().error(
                    "Action did not complete but we exceeded maximum timesteps."
                )
                self._task_completed = False
                return True

        return False

    def _warmup(self):
        """Warm up and stat check"""
        # TODO: implement warmup
        pass
