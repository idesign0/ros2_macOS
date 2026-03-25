from typing import List, Dict, Literal, Optional, Iterable, Tuple
from attr import define, field
import numpy as np

from rclpy.logging import get_logger

from .utils import _read_spec_file, find_missing_values


def _size_validator(instance, attribute, value):
    """Size validator for positions, velocities, accelerations or efforts"""
    if value.size > 0 and value.size != len(instance.joints_names):
        raise ValueError(
            f"Length of {attribute} must be the same as length of joint_names: {len(value)} not equal to {len(instance.joints_names)}"
        )


@define(kw_only=True)
class JointsData:
    """
    JointsData structure representing the state, command, or trajectory for a set of robot joints.

    It encapsulates kinematic and dynamic information such as positions, velocities,
    accelerations, and efforts, along with timing constraints.

    :param joints_names: A list of unique identifiers/names for the joints.
    :type joints_names: List[str]
    :param positions: An array of joint positions (e.g., angles in radians for revolute joints
        or meters for prismatic joints). Defaults to an empty array.
    :type positions: np.ndarray
    :param velocities: An array of joint velocities. Defaults to an empty array.
    :type velocities: np.ndarray
    :param accelerations: An array of joint accelerations. Defaults to an empty array.
    :type accelerations: np.ndarray
    :param efforts: An array of joint efforts (e.g., torque or force). Defaults to an empty array.
    :type efforts: np.ndarray
    :param duration: The time duration associated with this joint state (e.g., time to reach
        this state). Defaults to 0.0.
    :type duration: float
    :param delay: A time delay to wait before applying or processing this joint data.
        Defaults to 0.0.
    :type delay: float
    """
    joints_names: List[str] = field()
    positions: np.ndarray = field(
        default=np.array([], dtype=np.float64), validator=_size_validator
    )
    velocities: np.ndarray = field(
        default=np.array([], dtype=np.float64), validator=_size_validator
    )
    accelerations: np.ndarray = field(
        default=np.array([], dtype=np.float64), validator=_size_validator
    )
    efforts: np.ndarray = field(
        default=np.array([], dtype=np.float64), validator=_size_validator
    )
    duration: float = field(default=0.0)
    delay: float = field(default=0.0)

    def get_mapped_state(
        self,
        state_type: Literal["positions", "velocities", "accelerations", "efforts"],
        joint_names_map: Dict[str, str],
    ) -> Optional[Dict[str, np.float32]]:
        """Return the state mapped according to the dataset keys provided in the config"""
        # Get particular state values of the type required
        state_values = getattr(self, state_type, None)

        if state_values is None or state_values.size == 0:
            return None

        # Build a name -> index lookup table
        name_to_index = {name: i for i, name in enumerate(self.joints_names)}

        mapped = {}

        for target_name, source_name in joint_names_map.items():
            idx = name_to_index.get(source_name)
            if idx is None:
                return None
            mapped[target_name] = state_values[idx]

        return mapped


def create_observation_spec(
    joints_map, camera_map, prefix="observation", image_shape=(480, 640, 3)
):
    """Create a specification dictionary for observation data structure.

    This function generates a structured specification for observation data
    that includes both state information from joints and visual information
    from cameras. The specification defines data types, shapes, and metadata
    for each observation component.

    joints_map : dict
        Mapping of joint names to their respective configurations. The keys
        are used to define the state observation dimensions.
    camera_map : dict
        Mapping of camera names to their respective configurations. Each
        camera will contribute a visual observation entry to the specification.
    prefix : str, optional
        Prefix to use for observation keys, defaults to "observation"
    image_shape : tuple of int, optional
        Shape of camera images as (height, width, channels), defaults to (480, 640, 3)

    dict
        Observation specification dictionary with the following structure:
        - ``{prefix}.state``: State observation containing joint positions
        - ``{prefix}.images.{cam_name}``: Visual observation for each camera
        Each entry contains:
        - ``dtype``: Data type ("float32" for state, "video" for images)
        - ``shape``: Shape of the data
        - ``type``: Observation type ("STATE" or "VISUAL")
        - ``names``: List of dimension names
    """

    # Validate image_shape
    if not isinstance(image_shape, (tuple, list)) or len(image_shape) != 3:
        raise ValueError(
            f"image_shape must be a tuple of 3 values (height, width, channels). Got: {image_shape}"
        )

    spec = {}

    # Build the State entry
    # use the keys from joint_names_map as the 'names' list
    joint_names = list(joints_map.keys())
    state_key = f"{prefix}.state"

    spec[state_key] = {
        "dtype": "float32",
        "shape": (len(joint_names),),
        "type": "STATE",
        "names": joint_names,
    }

    # Build the Camera entries
    # iterate over the keys in camera_inputs_map to create entries
    for cam_name in camera_map.keys():
        cam_key = f"{prefix}.images.{cam_name}"
        spec[cam_key] = {
            "dtype": "video",
            "shape": tuple(image_shape),
            "type": "VISUAL",
            "names": ["height", "width", "channels"],
        }

    return spec


def validate_mapping_completeness(
    target_keys: Optional[List],
    mapped_keys,
    logger,
    missing_data_msg: str,
    error_msg: str,
):
    """
    Generic helper to check if specific keys exist in a map.
    Warns if data is missing, raises ValueError if mapping is incomplete.
    Called from inside the component
    """
    if not target_keys:
        logger.warning(missing_data_msg)
        return

    missing_keys = find_missing_values(mapped_keys, target_keys)
    if missing_keys:
        raise ValueError(error_msg.format(missing=missing_keys))


# Map requirement categories -> URDF limit keys
req_map = {
    "positions": ["lower", "upper"],
    "efforts": ["effort"],
    "velocities": ["velocity"],
    "accelerations": ["velocity"],
}


def check_joint_limits(
    joints_limits: Dict[str, Optional[Dict[str, float]]], requirements: Iterable[str]
) -> Tuple:
    """
    Validate that required joint limits are provided for all joints.

    :param joints_limits: Output of parse_urdf_joints().
    :type joints_limits: Dict[str, Optional[Dict[str, float]]]

    :param requirements: Iterable of requirement categories:
                         - "positions"  -> requires "lower" and "upper"
                         - "efforts"    -> requires "effort"
                         - "velocities" or "accelerations" -> requires "velocity"
    :type requirements: Iterable[str]

    :return: A tuple containing a boolean indicating if all requirements are satisfied,
             and a list of human-readable error messages.
    :rtype: Tuple[bool, List[str]]
    """

    # Build final required keys
    required_keys = []
    for req in requirements:
        required_keys.extend(req_map.get(req, []))

    errors = []

    for joint, limits in joints_limits.items():
        if limits is None:
            # No limits available at all
            missing = required_keys
            if missing:
                errors.append(
                    f"Joint '{joint}' has no <limit> tag but requires: {missing}"
                )
            continue

        # Check required keys for this joint
        for key in required_keys:
            if limits.get(key) is None:
                errors.append(f"Joint '{joint}' missing required limit '{key}'.")

    return (len(errors) == 0, errors)


def cap_actions_with_limits(
    joint_names: List[str],
    target_actions: np.ndarray,
    limits_dict: Optional[Dict[str, Optional[Dict[str, float]]]],
    action_type: Literal["positions", "velocities", "efforts", "accelerations"],
    logger_name: str,
) -> np.ndarray:
    """
    Cap the target actions for a set of joints based on their specified limits.

    :param joint_names: A list of names corresponding to the joints.
    :type joint_names: List[str]
    :param target_actions:
        The target actions (positions, velocities, efforts, or accelerations) for each joint.
    :type target_actions: np.ndarray(dtype=np.float32)
    :param limits_dict:
        A dictionary containing the limit specifications for each joint. Each key is a joint name,
        and the value is another dictionary with keys 'lower' and/or 'upper' (for positions)
        or 'velocity', 'effort', etc. (for other action types).
    :type limits_dict: Dict[str, Optional[Dict[str, float]]]
    :param action_type: The type of actions being capped.
    :type action_type: Literal["positions", "velocities", "efforts", "accelerations"]
    :param logger_name:
        The name of the component to use for logging warnings.
    :type logger_name: str

    :return:
        An np.array of capped actions for each joint, ensuring they do not exceed their specified limits.
    :rtype: np.ndarray(dtype=np.float32)

    Notes
    -----
    - For positions, both 'lower' and 'upper' limits must be provided in `limits_dict`.
    - For velocities, efforts, and accelerations, the corresponding limit key (e.g., 'velocity') must be provided.
    """

    result = np.copy(target_actions)
    required_limits = req_map[action_type]

    # TODO: Implement correct limit checking for acceleration based on velocities and timestep
    # Special case: accelerations -> skip limit checking entirely
    if action_type == "accelerations":
        get_logger(logger_name).warning(
            "Acceleration action is not currently limit-checked "
        )
        return target_actions

    for idx, (jname, target) in enumerate(
        zip(joint_names, target_actions, strict=True)
    ):
        # If limit missing in dict
        joint_limits = limits_dict.get(jname)
        if joint_limits is None:
            get_logger(logger_name).warning(
                f"Joint '{jname}' has no limit assigned — action left uncapped."
            )
            result[idx] = target
            continue

        # Check required limits exist
        missing_any = False
        for limit_key in required_limits:
            if joint_limits.get(limit_key) is None:
                get_logger(logger_name).warning(
                    f"Joint '{jname}' missing required limit '{limit_key}' — action left uncapped."
                )
                missing_any = True
        if missing_any:
            result[idx] = target
            continue

        # Safety capping logic
        capped = target
        if action_type == "positions":
            lower = joint_limits["lower"]
            upper = joint_limits["upper"]

            if capped < lower:
                get_logger(logger_name).warning(
                    f"Position for joint '{jname}' capped: {capped} -> {lower}"
                )
                capped = lower
            elif capped > upper:
                get_logger(logger_name).warning(
                    f"Position for joint '{jname}' capped: {capped} -> {upper}"
                )
                capped = upper

        else:
            # effort / velocity / acceleration -> absolute magnitude clamp
            limit_key = required_limits[0]
            max_mag = joint_limits[limit_key]

            if abs(capped) > max_mag:
                new_val = max_mag if capped > 0 else -max_mag
                get_logger(logger_name).warning(
                    f"{action_type.capitalize()} for joint '{jname}' capped: "
                    f"{capped} -> {new_val}"
                )
                capped = new_val

        result[idx] = capped

    return result


def parse_urdf_joints(path_or_url: str) -> Dict:
    """
    Parse a URDF file and extract joint limits.

    :param path_or_url: The file path or URL of the URDF file to parse.
    :type path_or_url: str
    :return: A dictionary where the keys are joint names and the values are dictionaries containing the joint limits (lower, upper, effort, velocity).
             If no limits are found for a joint, the value will be `None`.
    :rtype: dict
    """
    root = _read_spec_file(path_or_url, spec_type="xml")

    joints_limits = {}

    for joint in root.findall("joint"):
        name = joint.get("name")

        limit_tag = joint.find("limit")
        if limit_tag is None:
            joints_limits[name] = None
            continue

        # Extract attributes if present
        limits = {}
        for attr in ["lower", "upper", "effort", "velocity"]:
            value = limit_tag.get(attr)
            limits[attr] = float(value) if value is not None else None

        joints_limits[name] = limits

    return joints_limits
