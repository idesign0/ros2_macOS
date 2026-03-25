from typing import List, Tuple, Dict, Callable, Union, Any
import re
import sys
import base64
import numpy as np
import cv2
from socket import socket

from rclpy.logging import get_logger
import std_msgs.msg as std_msg
from nav_msgs.msg import Odometry

import msgpack
import msgpack_numpy as m_pack

# patch msgpack for numpy arrays
m_pack.patch()


def convert_img_to_jpeg_str(img, node_name: str = "util") -> str:
    # Encode image as JPEG
    img = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)  # as cv2 expects a BGR

    encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 90]
    result, buffer = cv2.imencode(".jpg", img, encode_param)
    if not result:
        get_logger(node_name).error("Failed to encode image to JPEG format.")
        raise Exception("Failed to encode image to JPEG format.")
    else:
        # Convert to base64
        return base64.b64encode(buffer).decode("utf-8")


def process_encoding(encoding: str) -> Tuple[np.dtype, int]:
    """
    Returns dtype and number of channels from encoding
    """
    encoding = encoding.lower()

    # Define mapping from encoding to (dtype, channels)
    encoding_map = {
        # RGB/BGR family
        "rgb8": (np.uint8, 3),
        "rgba8": (np.uint8, 4),
        "rgb16": (np.uint16, 3),
        "rgba16": (np.uint16, 4),
        "bgr8": (np.uint8, 3),
        "bgra8": (np.uint8, 4),
        "bgr16": (np.uint16, 3),
        "bgra16": (np.uint16, 4),
        # Mono
        "mono8": (np.uint8, 1),
        "mono16": (np.uint16, 1),
        # Bayer – typically raw single-channel
        "bayer_rggb8": (np.uint8, 1),
        "bayer_bggr8": (np.uint8, 1),
        "bayer_gbrg8": (np.uint8, 1),
        "bayer_grbg8": (np.uint8, 1),
        "bayer_rggb16": (np.uint16, 1),
        "bayer_bggr16": (np.uint16, 1),
        "bayer_gbrg16": (np.uint16, 1),
        "bayer_grbg16": (np.uint16, 1),
        # CvMat types
        "8uc1": (np.uint8, 1),
        "8uc2": (np.uint8, 2),
        "8uc3": (np.uint8, 3),
        "8uc4": (np.uint8, 4),
        "8sc1": (np.int8, 1),
        "8sc2": (np.int8, 2),
        "8sc3": (np.int8, 3),
        "8sc4": (np.int8, 4),
        "16uc1": (np.uint16, 1),
        "16uc2": (np.uint16, 2),
        "16uc3": (np.uint16, 3),
        "16uc4": (np.uint16, 4),
        "16sc1": (np.int16, 1),
        "16sc2": (np.int16, 2),
        "16sc3": (np.int16, 3),
        "16sc4": (np.int16, 4),
        "32sc1": (np.int32, 1),
        "32sc2": (np.int32, 2),
        "32sc3": (np.int32, 3),
        "32sc4": (np.int32, 4),
        "32fc1": (np.float32, 1),
        "32fc2": (np.float32, 2),
        "32fc3": (np.float32, 3),
        "32fc4": (np.float32, 4),
        "64fc1": (np.float64, 1),
        "64fc2": (np.float64, 2),
        "64fc3": (np.float64, 3),
        "64fc4": (np.float64, 4),
        "yuv422": (np.uint8, 2),
    }

    if encoding not in encoding_map:
        if "yuv422" in encoding:
            return encoding_map["yuv422"]
        raise ValueError(f"Unsupported encoding: {encoding}")

    return encoding_map[encoding]


def image_pre_processing(img, dtype, num_channels) -> np.ndarray:
    """
    Pre-processes ROS sensor_msgs/Image into a numpy array.
    - Handles encodings, endianess, alpha channels, Bayer, YUV422.
    - Returns RGB arrays for color images, grayscale for mono, raw floats for depth.
    """
    dtype = np.dtype(dtype)
    np_arr = np.frombuffer(img.data, dtype=dtype)

    # Endian correction
    if img.is_bigendian and (
        np_arr.dtype.byteorder == "<"
        or (np_arr.dtype.byteorder == "=" and sys.byteorder == "little")
    ):
        np_arr = np_arr.byteswap().newbyteorder()

    # Reshape
    if num_channels == 1:
        np_arr = np.ndarray(
            shape=(img.height, int(img.step / dtype.itemsize)),
            dtype=dtype,
            buffer=np_arr,
        )
        np_arr = np.ascontiguousarray(np_arr[: img.height, : img.width])
    else:
        np_arr = np.ndarray(
            shape=(
                img.height,
                int(img.step / dtype.itemsize / num_channels),
                num_channels,
            ),
            dtype=dtype,
            buffer=np_arr,
        )
        np_arr = np.ascontiguousarray(np_arr[: img.height, : img.width, :])

        # Drop alpha channel if present
        if num_channels == 4:
            np_arr = np_arr[:, :, :3]

    enc = img.encoding.lower()

    # Handle Bayer patterns
    if enc.startswith("bayer_"):
        bayer_map = {
            "bayer_rggb8": cv2.COLOR_BAYER_RG2RGB,
            "bayer_bggr8": cv2.COLOR_BAYER_BG2RGB,
            "bayer_gbrg8": cv2.COLOR_BAYER_GB2RGB,
            "bayer_grbg8": cv2.COLOR_BAYER_GR2RGB,
            "bayer_rggb16": cv2.COLOR_BAYER_RG2RGB,
            "bayer_bggr16": cv2.COLOR_BAYER_BG2RGB,
            "bayer_gbrg16": cv2.COLOR_BAYER_GB2RGB,
            "bayer_grbg16": cv2.COLOR_BAYER_GR2RGB,
        }
        if enc in bayer_map:
            np_arr = cv2.cvtColor(np_arr, bayer_map[enc])
        return np_arr  # already RGB

    # Handle YUV422
    if "yuv422" in enc:
        # Annoying edge case: Assume these formats to be stored rgb
        if "yuy2" in enc:
            np_arr = cv2.cvtColor(np_arr, cv2.COLOR_YUV2RGB_YUYV)
        elif "uyvy" in enc:
            np_arr = cv2.cvtColor(np_arr, cv2.COLOR_YUV2RGB_UYVY)
        else:  # generic fallback
            np_arr = cv2.cvtColor(np_arr, cv2.COLOR_YUV2RGB_YUYV)
        return np_arr

    # Handle BGR/BGRA
    if enc.startswith("bgr"):
        np_arr = cv2.cvtColor(np_arr, cv2.COLOR_BGR2RGB)

    return np_arr


def parse_format(fmt: str):
    """
    Parse the CompressedImage.format field into components.
    Returns dict: { orig: str, codec: Optional['jpeg'|'png'|'rvl'], comp: Optional[str], is_depth: bool }
    """
    fmt = (fmt or "").strip()
    if not fmt:
        return {"orig": "", "codec": None, "comp": None, "is_depth": False}

    parts = [p.strip() for p in fmt.split(";") if p.strip()]
    orig = parts[0] if parts else ""
    rest = ";".join(parts[1:]).lower() if len(parts) > 1 else ""

    is_depth = (
        "compresseddepth" in rest
        or "compresseddept" in rest
        or "compresseddepth" in fmt.lower()
    )

    # find codec (jpeg/png/rvl)
    codec_match = re.search(r"\b(jpeg|png|rvl)\b", rest)
    codec = codec_match.group(1) if codec_match else None

    # find compressed pixel format if present (examples: bgr8, rgb8, bgr16, rgb16, mono8, rgba8, bgra8)
    comp_match = re.search(
        r"\b(bgra8|rgba8|bgr16|rgb16|bgr8|rgb8|mono16|mono8)\b", fmt.lower()
    )
    comp = comp_match.group(0) if comp_match else None

    return {"orig": orig, "codec": codec, "comp": comp, "is_depth": is_depth}


def read_compressed_image(img, parsed_fmt: Dict, prefer_rgb: bool = True) -> np.ndarray:
    """
    Read a ROS sensor_msgs/CompressedImage (or similar) message into a numpy array.

      - For color images (jpeg/png) decodes the compressed bytes and returns:
          - HxWx3 RGB (uint8) by default (prefer_rgb=True).
          - If prefer_rgb=False the raw ordering (OpenCV BGR/BGRA) is preserved.
      - For png color with 16-bit (rgb16/bgr16) the function preserves dtype (uint16).
      - For grayscale/mono images returns 2D array (HxW).
      - For depth images (format contains 'compressedDepth'):
          - PNG path: strips the compression config header (12 bytes on common builds) and decodes
            the embedded PNG with cv2.IMREAD_UNCHANGED (preserves uint16 / float32 if present).
          - RVL path: *not* implemented in this helper — see notes below and links.
    :param img: a message-like object with attributes `.format` (string) and `.data` (bytes/bytearray/list[int])
    :param fmt: fmt as a dict.
    :param prefer_rgb: if True convert color images to RGB; otherwise leave as OpenCV order (BGR/BGRA).
    :returns: numpy array (H x W x C) or (H x W) for mono/depth
    :raises ValueError on decode errors or if RVL decompression is required (with guidance).
    """
    is_depth = parsed_fmt["is_depth"]
    codec = parsed_fmt["codec"]
    comp_pixfmt = parsed_fmt["comp"]  # e.g. 'rgb8', 'bgr8', 'rgb16'

    # build a bytes object from img.data robustly
    if isinstance(img.data, (bytes, bytearray)):
        raw = bytes(img.data)
    else:
        # list of ints or numpy array
        raw = bytes(bytearray(img.data))

    if is_depth:
        # compressed_depth_image_transport prepends a binary ConfigHeader (~12 bytes) to the compressed payload.
        # We therefore strip ~12 bytes before decoding PNG/RVL payload.
        # Reference: compressed_depth_image_transport codec implementation (PNG and RVL paths).
        header_size = 12  # typical (sizeof(ConfigHeader): enum(4) + 2 * float(4) = 12)
        if len(raw) <= header_size:
            raise ValueError(
                "Compressed depth message too small to contain header + payload"
            )

        payload = raw[header_size:]

        # codec might be 'png' or 'rvl'. If codec wasn't parsed, try to detect by peeking bytes.
        if codec is None:
            # quick detect: PNG files start with the PNG signature
            if payload[:8] == b"\x89PNG\r\n\x1a\n":
                codec = "png"
            else:
                # rvl payloads (rows/cols + rvl data) won't have PNG signature
                # fall back to rvl if not png
                codec = "rvl"

        if codec == "png":
            arr = np.frombuffer(payload, dtype=np.uint8)
            im = cv2.imdecode(arr, cv2.IMREAD_UNCHANGED)
            if im is None:
                raise ValueError("Failed to decode PNG payload for compressedDepth")
            # NOTE: im dtype will often be uint16 (CV_16U) for 16UC1 depth; for 32FC1 paths the compressed pipeline
            # may have stored float depth differently (see compressed_depth_image_transport docs).
            return im  # keep shape/dtype as produced

        elif codec == "rvl":
            # RVL is a fast lossless 16-bit depth compression used by compressed_depth_image_transport.
            # The ROS C++ plugin handles RVL (RvlCodec::DecompressRVL). We do not implement RVL here.
            # Helpful references:
            # - ROS compressed_depth_image_transport codec implementation.
            # - RVL paper (algorithm description).
            raise NotImplementedError(
                "RVL-compressed depth detected. RVL decompression is not implemented in this helper. "
                "Use the ROS 'compressed_depth_image_transport' C++/Python bridge, cv_bridge, or port the "
                "RvlCodec. For pointers see: compressed_depth_image_transport codec and RVL paper. "
            )
        else:
            # unknown depth codec
            raise ValueError(f"Unknown compressedDepth codec: {codec}")

    # Color / regular compressed image
    # prepare numpy buffer for cv2.imdecode
    arr = np.frombuffer(raw, dtype=np.uint8)

    # if codec is known, use best flags
    if codec == "jpeg":
        # JPEG is inherently 8-bit; grayscale detection via ORIG/COMP strings
        want_gray = False
        if parsed_fmt["orig"].lower().startswith("mono") or (
            comp_pixfmt and comp_pixfmt.startswith("mono")
        ):
            want_gray = True

        if want_gray:
            # Special handling: YUV422 published as mono8 but actually bgr color
            if "yuv422" in img.format.lower():
                cv_im = cv2.imdecode(arr, cv2.IMREAD_COLOR_BGR)
                if cv_im is None:
                    raise ValueError("Failed to decode JPEG YUV422 image")
                return cv_im

            # grayscale path
            cv_im = cv2.imdecode(arr, cv2.IMREAD_GRAYSCALE)
            if cv_im is None:
                raise ValueError("Failed to decode JPEG grayscale image")
            return cv_im
        else:
            cv_im = cv2.imdecode(arr, cv2.IMREAD_COLOR)  # returns BGR uint8
            if cv_im is None:
                raise ValueError("Failed to decode JPEG color image")

            # Decide whether to convert to RGB:
            convert_to_rgb = False
            if comp_pixfmt:
                # If compressed pixfmt explicitly says 'rgb8' or 'rgb16' -> convert to RGB
                if comp_pixfmt.startswith("rgb"):
                    convert_to_rgb = True
                elif comp_pixfmt.startswith("bgr"):
                    convert_to_rgb = False
            else:
                # fallback: user preference
                convert_to_rgb = bool(prefer_rgb)

            if convert_to_rgb:
                cv_im = cv2.cvtColor(cv_im, cv2.COLOR_BGR2RGB)
            return cv_im

    else:
        # for png and unknown codecs try IMREAD_UNCHANGED (preserves 16-bit / alpha)
        cv_im = cv2.imdecode(arr, cv2.IMREAD_UNCHANGED)
        if cv_im is None:
            # fallback to color
            cv_im = cv2.imdecode(arr, cv2.IMREAD_COLOR)
            if cv_im is None:
                raise ValueError("Failed to decode compressed image (unknown codec)")

        # If image has 3 or 4 channels, optionally convert BGR(A)->RGB(A)
        if cv_im.ndim == 3 and prefer_rgb:
            _, _, c = cv_im.shape
            if c == 3:
                cv_im = cv2.cvtColor(cv_im, cv2.COLOR_BGR2RGB)
            elif c == 4:
                # BGRA -> RGBA
                cv_im = cv2.cvtColor(cv_im, cv2.COLOR_BGRA2RGBA)
        return cv_im


def _np_quaternion_multiply(q1: np.ndarray, q2: np.ndarray) -> np.ndarray:
    """
    Multiplies two quaternions q1 * q2
    Each quaternion is an array [w, x, y, z]
    """
    w0, x0, y0, z0 = q1
    w1, x1, y1, z1 = q2
    return np.array([
        w0 * w1 - x0 * x1 - y0 * y1 - z0 * z1,
        w0 * x1 + x0 * w1 + y0 * z1 - z0 * y1,
        w0 * y1 - x0 * z1 + y0 * w1 + z0 * x1,
        w0 * z1 + x0 * y1 - y0 * x1 + z0 * w1,
    ])


def _np_quaternion_conjugate(q: np.ndarray) -> np.ndarray:
    """
    Returns the conjugate of a quaternion
    """
    w, x, y, z = q
    return np.array([w, -x, -y, -z])


def _rotate_vector_by_quaternion(q: np.ndarray, v: List[float]) -> List[float]:
    """
    Rotate a 3D vector v by a quaternion q.

    :param      q: quaternion [w, x, y, z]
    :param      v: vector [x, y, z]
    :return:    rotated vector
    """
    vq = np.array([0.0, *v])
    q_conj = _np_quaternion_conjugate(q)
    rotated_vq = _np_quaternion_multiply(_np_quaternion_multiply(q, vq), q_conj)
    return rotated_vq[1:].tolist()


def _transform_pose(
    position: np.ndarray,
    orientation: np.ndarray,
    reference_position: np.ndarray,
    reference_orientation: np.ndarray,
) -> np.ndarray:
    """
    Transforms a pose from a local frame to a global frame using a reference pose.
    Equivalent to: global_pose = reference_pose * local_pose

    :param pose: PoseData in local frame
    :type pose: PoseData
    :param reference_pose: PoseData of local frame in global frame
    :type reference_pose: PoseData
    :return: PoseData in global frame
    :rtype: PoseData
    """
    rotated_position = _rotate_vector_by_quaternion(
        reference_orientation, position.tolist()
    )
    translated_position = reference_position + np.array(rotated_position)

    combined_orientation = _np_quaternion_multiply(reference_orientation, orientation)

    return np.array([*translated_position, *combined_orientation])


def _get_position_from_odom(odom_msg: Odometry) -> np.ndarray:
    """
    Gets a numpy array with 3D position coordinates from Odometry message

    :param odom_msg: ROS Odometry message
    :type odom_msg: Odometry

    :return: 3D position [x, y, z]
    :rtype: np.ndarray
    """
    return np.array([
        odom_msg.pose.pose.position.x,
        odom_msg.pose.pose.position.y,
        odom_msg.pose.pose.position.z,
    ])


def _get_odom_from_ndarray(odom_array: np.ndarray) -> Odometry:
    """Convert numpy array to an odometry message.

    :param odom_array:
    :type odom_array: np.ndarray
    :rtype: Odometry
    """
    odom_msg = Odometry()
    odom_msg.pose.pose.position.x = odom_array[0]
    odom_msg.pose.pose.position.y = odom_array[1]
    odom_msg.pose.pose.position.z = odom_array[2]
    odom_msg.pose.pose.orientation.w = odom_array[3]
    odom_msg.pose.pose.orientation.x = odom_array[4]
    odom_msg.pose.pose.orientation.y = odom_array[5]
    odom_msg.pose.pose.orientation.z = odom_array[6]

    return odom_msg


def _get_orientation_from_odom(odom_msg: Odometry) -> np.ndarray:
    """
    Gets a rotation quaternion from Odometry message

    :param odom_msg: ROS Odometry message
    :type odom_msg: Odometry

    :return: Rotation quaternion (qw, qx, qy, qz)
    :rtype: np.ndarray
    """
    return np.array([
        odom_msg.pose.pose.orientation.w,
        odom_msg.pose.pose.orientation.x,
        odom_msg.pose.pose.orientation.y,
        odom_msg.pose.pose.orientation.z,
    ])


def odom_from_frame1_to_frame2(
    pose_1_in_2: Odometry, pose_target_in_1: Odometry
) -> Odometry:
    """
    get the pose of a target in frame 2 instead of frame 1

    :param      pose_1_in_2:        pose of frame 1 in frame 2
    :type       pose_1_in_2:        PoseData
    :param      pose_target_in_1:   pose of target in frame 1
    :type       pose_target_in_1:   PoseData

    :return:    pose of target in frame 2
    :rtype:     PoseData
    """
    transformed_pose = _transform_pose(
        _get_position_from_odom(pose_target_in_1),
        _get_orientation_from_odom(pose_target_in_1),
        _get_position_from_odom(pose_1_in_2),
        _get_orientation_from_odom(pose_1_in_2),
    )
    return _get_odom_from_ndarray(transformed_pose)


def _parse_array_type(arr: np.ndarray, ros_msg_cls: type) -> np.ndarray:
    """Parses a numpy array to the data type of an std_msg

    :param arr: Data array
    :type arr: np.ndarray
    :param ros_msg_cls: Ros2 std_msg multi array class
    :type ros_msg_cls: type
    :return: Parsed data
    :rtype: np.ndarray
    """
    if ros_msg_cls == std_msg.Float32MultiArray:
        arr = arr.astype(np.float32)
    elif ros_msg_cls == std_msg.Float64MultiArray:
        arr = arr.astype(np.float64)
    elif ros_msg_cls == std_msg.Int16MultiArray:
        arr = arr.astype(np.int16)
    elif ros_msg_cls == std_msg.Int32MultiArray:
        arr = arr.astype(np.int32)
    elif ros_msg_cls == std_msg.Int64MultiArray:
        arr = arr.astype(np.int64)
    return arr


def numpy_to_multiarray(arr: np.ndarray, ros_msg_cls: type, labels=None):
    """
    Convert a numpy array to a ROS2 ___MultiArray message.
    """
    if not isinstance(arr, np.ndarray):
        arr = np.array(arr)

    arr = _parse_array_type(arr, ros_msg_cls)

    msg = ros_msg_cls()

    # Calculate strides (assuming C-order, row-major)
    strides = [1]
    for i in range(len(arr.shape) - 1, 0, -1):
        strides.insert(0, strides[0] * arr.shape[i])

    # Create dimension labels if not provided
    if labels is None:
        labels = [f"dim{i}" for i in range(len(arr.shape))]
    elif len(labels) != len(arr.shape):
        raise ValueError("Number of labels must match number of dimensions")

    # Set up the layout
    msg.layout.dim = []
    for size, stride, label in zip(arr.shape, strides, labels, strict=True):
        dim = std_msg.MultiArrayDimension()
        dim.label = label
        dim.size = size
        dim.stride = stride
        msg.layout.dim.append(dim)

    # Flatten the array and convert to list for the message
    msg.data = arr.flatten().tolist()

    return msg


def run_external_processor(
    logger_name: str, topic_name: str, processor: Union[Callable, socket], *output
) -> Any:
    """
    Execute external processing using a callable or a Unix socket.

    This utility function is designed to handle two scenarios:
    1. When the processor is a callable (e.g., a function), it invokes the callable with the provided `*output` arguments.
    2. When the processor is a Unix socket, it sends the `*output` data packed in msgpack format to the connected process and waits for a response.

    :param logger_name: The name of the logger to use for logging messages.
    :type logger_name: str

    :param topic_name: A descriptive name for the processing topic, used in log messages.
    :type topic_name: str

    :param processor: The external processor, which can be either a callable or a Unix socket.
                      If it's a callable, it will be directly invoked with `*output`.
                      If it's a Unix socket, data will be sent and received over this socket.
    :type processor: Union[Callable, socket]

    :param output: Variable length argument list to be passed to the external processor.
    :type output: Any

    :return: The result of the external processing. This can vary depending on the type of processor used.
             For a callable, it's whatever the function returns.
             For a Unix socket, it's the unpacked response received from the connected process.
    :rtype: Any

    :raises Exception: If an error occurs during the execution of the external processor or communication over the socket,
                       an exception is logged with an appropriate error message.
    """
    if isinstance(processor, Callable):
        return processor(*output)

    try:
        out_dict = {"output": output}
        payload = msgpack.packb(out_dict)
        if payload:
            processor.sendall(payload)
        else:
            get_logger(logger_name).error(
                f"Could not pack arguments for external processor in external function provided for {topic_name}"
            )
        result_b = processor.recv(1024)
        result = msgpack.unpackb(result_b)
        return result
    except Exception as e:
        get_logger(logger_name).error(
            f"Error in external processor for {topic_name}: {e}"
        )
