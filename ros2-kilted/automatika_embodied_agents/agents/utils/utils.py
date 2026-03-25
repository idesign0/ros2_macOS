import base64
import json
import inspect
import uuid
from functools import wraps
from enum import Enum
from io import BytesIO
from pathlib import Path
from typing import (
    List,
    Dict,
    Literal,
    Optional,
    Union,
    get_args,
    get_origin,
    _GenericAlias,
)
from collections.abc import Iterable
import xml.etree.ElementTree as ET

import cv2
import httpx
import numpy as np
from attrs import Attribute
from jinja2 import Environment, FileSystemLoader
from jinja2.environment import Template
from .pluralize import pluralize


def draw_detection_bounding_boxes(
    img: np.ndarray, bounding_boxes: List, labels: List, handle_bbox2d_msg: bool = True
) -> np.ndarray:
    """Draw bounding boxes and labels"""

    for i, bbox in enumerate(bounding_boxes):
        # Bounding box expected format: (x1, y1, x2, y2)
        if handle_bbox2d_msg:
            bbox = [
                bbox.top_left_x,
                bbox.top_left_y,
                bbox.bottom_right_x,
                bbox.bottom_right_y,
            ]  # reading from BBox2D msg
        x1, y1, x2, y2 = map(int, bbox)

        # Choose color based on label if available
        if labels and i < len(labels):
            label_text = str(labels[i])
            color_seed = abs(hash(label_text)) % 0xFFFFFF
            color = (
                color_seed & 255,
                (color_seed >> 8) & 255,
                (color_seed >> 16) & 255,
            )
        else:
            label_text = ""
            color = (0, 255, 0)

        # Draw rectangle
        cv2.rectangle(img, (x1, y1), (x2, y2), color, 2)

        # Draw label text background and text
        if label_text:
            font = cv2.FONT_HERSHEY_SIMPLEX
            font_scale = 0.5
            thickness = 1
            (tw, th), _ = cv2.getTextSize(label_text, font, font_scale, thickness)
            cv2.rectangle(img, (x1, y1 - th - 4), (x1 + tw + 4, y1), color, -1)
            cv2.putText(
                img,
                label_text,
                (x1 + 2, y1 - 2),
                font,
                font_scale,
                (255, 255, 255),
                thickness,
                cv2.LINE_AA,
            )
    return img


def draw_points_2d(img: np.ndarray, points: np.ndarray, radius: int = 3) -> np.ndarray:
    """Draw 2D points on an image."""
    if points is None or len(points) == 0:
        return img

    # Ensure points is a np array of shape (N, 2)
    points = np.asarray(points)
    if points.ndim != 2 or points.shape[1] != 2:
        raise ValueError(f"Expected points of shape (N, 2), got {points.shape}")

    for x, y in points:
        cv2.circle(img, (int(x), int(y)), radius, (255, 0, 0), -1)  # red filled circle

    return img


def create_detection_context(obj_list: Optional[List]) -> str:
    """
    Creates a context prompt based on detections.
    :param      detections:  The detections
    :type       detections:  str
    :returns:   Context string
    :rtype:     str
    """
    if not obj_list:
        return ""
    context_list = []
    for obj_class in set(obj_list):
        obj_count = obj_list.count(obj_class)
        if obj_count > 1:
            context_list.append(f"{str(obj_count)} {pluralize(obj_class)}")
        else:
            context_list.append(f"{str(obj_count)} {obj_class}")

    if len(obj_list) > 1:
        return f"{', '.join(context_list)}"
    return f"{context_list[0]}"


def get_prompt_template(template: Union[str, Path]) -> Template:
    """Method to read prompt jinja prompt templates
    :param template:
    :type template: str | Path
    :rtype: None
    """
    # check if prompt is a filename
    try:
        path_exists = Path(template).exists()
    except OSError:
        path_exists = False
    if path_exists:
        try:
            env = Environment(
                loader=FileSystemLoader(Path(template).parent), autoescape=True
            )
            return env.get_template(Path(template).name)
        except Exception as e:
            raise Exception(
                f"Exception occurred while reading template from file: {e}"
            ) from e
    else:
        # read from string
        try:
            env = Environment()
            return env.from_string(format(template))
        except Exception as e:
            raise Exception(
                f"Exception occurred while reading template from string: {e}"
            ) from e


def validate_kwargs_from_default(_, attribute: Attribute, value: Dict):
    """Validate kwargs
    :param attribute:
    :type attribute: Attribute
    :param value:
    :type value: dict
    """

    def _create_default_type_str():
        """_create_default_type_str."""
        return "\n".join([f"{k}: {type(v)}" for k, v in attribute.default.items()])

    try:
        value_keys = value.keys()
    except AttributeError as e:
        raise AttributeError(f"{attribute.name} needs to be set with a dict") from e

    for key in value_keys:
        if key not in attribute.default:
            raise AttributeError(
                f"{attribute.name} can be one of the following:\n{_create_default_type_str()}"
            )
    # add missings to final value of attribute
    for k, v in attribute.default.items():
        if k not in value_keys:
            value[k] = v


def _check_type_from_signature(value, fn_param: inspect.Parameter) -> None:
    """Check parameter value type based on parameter signature.
    :param value:
    :param fn_param:
    :type fn_param: inspect.Parameter
    :rtype: None
    """
    # Handles only one layer of Union
    if get_origin(fn_param.annotation) is Union:
        _annotated_types = get_args(fn_param.annotation)
    else:
        _annotated_types = [fn_param.annotation]

    # Handles only the origin of GenericAlias (dict, list)
    _annotated_types = [
        get_origin(t) if isinstance(t, _GenericAlias) else t for t in _annotated_types
    ]

    type_check = any(isinstance(value, t) for t in _annotated_types)
    if not type_check:
        raise TypeError(
            f"Invalid type encountered for {fn_param.name}. Should be of type(s) {fn_param.annotation}. Passed value might be of type {type(value)}"
        )


def _check_type_from_default(value, fn_param: inspect.Parameter) -> None:
    """Check parameter value type based on default value.
    :param value:
    :param fn_param:
    :type fn_param: inspect.Parameter
    :rtype: None
    """
    # for parameters with default value
    _default = fn_param.default
    # Flag to skip parameters with None as default value
    is_none = _default is None or value is None

    if not is_none and not isinstance(value, type(_default)):
        raise TypeError(
            f"Invalid type encountered for {fn_param.name}. Expected type {type(_default)}"
        )


def validate_func_args(func):
    """Decorator for validating function parameters based on function signature
    :param func:
    """

    @wraps(func)
    def wrapper(*args, **kwargs):
        """wrapper.
        :param args:
        :param kwargs:
        """
        args_count = len(args)
        fn_params = inspect.signature(func).parameters

        # for parameters with annotation, preference is given to checking by annotation
        for arg, param in zip(args, list(fn_params)[:args_count], strict=True):
            if fn_params[param].annotation is not fn_params[param].empty:
                _check_type_from_signature(arg, fn_params[param])
            elif fn_params[param].default is not fn_params[param].empty:
                _check_type_from_default(arg, fn_params[param])

        for kwarg, value in kwargs.items():
            param = fn_params.get(kwarg)
            if not param:
                continue
            if fn_params[kwarg].annotation is not fn_params[kwarg].empty:
                _check_type_from_signature(value, fn_params[kwarg])
            elif fn_params[kwarg].default is not fn_params[kwarg].empty:
                _check_type_from_default(value, fn_params[kwarg])

        # Call the function after validation
        result = func(*args, **kwargs)
        return result

    return wrapper


def encode_img_base64(img: np.ndarray) -> str:
    """Encode a numpy array to a base64 str.
    :param img:
    :type img: np.ndarray
    :rtype: str
    """
    # OpenCV expects BGR for encoding
    bgr = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)
    _, buf = cv2.imencode(".png", bgr)
    return base64.b64encode(buf).decode("utf-8")


class VADStatus(Enum):
    """VAD Status for start and end of detected speech"""

    START = 0
    ONGOING = 1
    END = 2


class WakeWordStatus(Enum):
    """WakeWord Status for start and end of detected wake word"""

    START = 0
    ONGOING = 1
    END = 2


def load_model(model_name: str, model_path: str) -> str:
    """Model download utility function"""
    from tqdm import tqdm
    from platformdirs import user_cache_dir

    cachedir = user_cache_dir("ros_agents")
    model_full_path = Path(cachedir) / Path("models") / Path(f"{model_name}.onnx")

    # create cache dir
    model_full_path.parent.mkdir(parents=True, exist_ok=True)

    # return if a file path is provided
    if Path(model_path).exists():
        return str(model_path)

    # check for cached model
    elif model_full_path.is_file():
        return str(model_full_path)

    else:
        # assume model path is a url open stream
        with httpx.stream("GET", model_path, timeout=20, follow_redirects=True) as r:
            r.raise_for_status()
            total_size = int(r.headers.get("content-length", 0))
            progress_bar = tqdm(
                total=total_size, unit="iB", unit_scale=True, desc=f"{model_name}"
            )
            # delete the file if an exception occurs while downloading
            try:
                with open(model_full_path, "wb") as f:
                    for chunk in r.iter_bytes(chunk_size=1024):
                        f.write(chunk)
                        progress_bar.update(len(chunk))
            except Exception:
                import logging

                logging.error(
                    f"Error occurred while downloading model {model_name} from given url. Try restarting your components."
                )
                if model_full_path.exists():
                    model_full_path.unlink()
                raise

    progress_bar.close()
    return str(model_full_path)


def load_model_repo(
    model_name: str,
    repo_id: str,
    allow_patterns: Optional[str] = None,
) -> str:
    """Download a multi-file HuggingFace model repository.

    :param model_name: Local cache name for the model
    :type model_name: str
    :param repo_id: HuggingFace repository ID (e.g. 'onnx-community/Qwen3-0.6B-ONNX')
    :type repo_id: str
    :param allow_patterns: Optional glob pattern to filter which files to download
        (e.g. 'cpu_and_mobile/cpu-int4-rtn-block-32-acc-level-4/*'). When set,
        matched files are flattened into the model directory root so that
        ``genai_config.json`` and model files are directly accessible.
    :type allow_patterns: Optional[str]
    :returns: Path to the downloaded model directory
    :rtype: str
    """
    import shutil
    from pathlib import Path
    from platformdirs import user_cache_dir

    cachedir = user_cache_dir("ros_agents")
    model_dir = Path(cachedir) / "models" / model_name
    model_dir.mkdir(parents=True, exist_ok=True)

    # If already downloaded, return cached path
    if any(model_dir.iterdir()):
        return str(model_dir)

    try:
        from huggingface_hub import snapshot_download
    except ModuleNotFoundError as e:
        raise ModuleNotFoundError(
            "Downloading HuggingFace model repos requires huggingface-hub. "
            "Install it with: pip install huggingface-hub"
        ) from e

    if allow_patterns:
        # Download to a temporary staging directory, then flatten the
        # matching subdirectory into model_dir so relevant files
        # genai_config.json end up at the root level
        staging_dir = model_dir.parent / f".{model_name}_staging"
        staging_dir.mkdir(parents=True, exist_ok=True)
        snapshot_download(
            repo_id=repo_id,
            local_dir=str(staging_dir),
            allow_patterns=allow_patterns,
        )
        # Find the deepest directory that contains the downloaded files
        # and move its contents to model_dir
        pattern_prefix = allow_patterns.rstrip("/*")
        src = staging_dir / pattern_prefix
        if src.is_dir():
            for item in src.iterdir():
                shutil.move(str(item), str(model_dir / item.name))
        shutil.rmtree(staging_dir, ignore_errors=True)
    else:
        snapshot_download(repo_id=repo_id, local_dir=str(model_dir))

    return str(model_dir)


def flatten(xs):
    """Generator to flatten lists of arbitrary types"""
    for x in xs:
        if isinstance(x, Iterable) and not isinstance(x, (str, bytes)):
            yield from flatten(x)
        else:
            yield x


def find_missing_values(check_list, string_list: List) -> List:
    """
    Return strings from `string_list` that do NOT appear in the dictionary's values.
    """
    # Convert values to a set for efficient lookup
    value_set = set(check_list)
    # Collect strings that are not found among the values
    missing = [s for s in string_list if s not in value_set]

    return missing


def _read_spec_file_from_url(url: str, spec_type: Literal["json", "xml"] = "json"):
    """Read from URL"""
    try:
        resp = httpx.get(url, timeout=10, follow_redirects=True).raise_for_status()
    except httpx.RequestError as e:
        raise RuntimeError(f"Failed to connect to URL '{url}'. Error: {e}") from e

    except httpx.HTTPStatusError as e:
        raise RuntimeError(f"Failed to fetch URL '{url}'. ") from e

    if spec_type == "json":
        try:
            return resp.json()
        except json.JSONDecodeError as e:
            raise RuntimeError(
                f"URL '{url}' returned non-JSON content. Error: {e}"
            ) from e
    elif spec_type == "xml":
        try:
            return ET.fromstring(resp.text)
        except ET.ParseError as e:
            raise ET.ParseError(f"Failed to parse XML from URL '{url}': {e}") from e


def _read_spec_file_from_path(path: str, spec_type: Literal["json", "xml"] = "json"):
    """Read from path"""
    p = Path(path)
    if not p.exists():
        raise FileNotFoundError(f"File not found: '{p}'")

    if not p.is_file():
        raise RuntimeError(f"Path exists but is not a file: '{p}'")
    try:
        if spec_type == "json":
            return json.loads(p.read_text(encoding="utf-8"))
        elif spec_type == "xml":
            tree = ET.parse(p)
            return tree.getroot()
    except ET.ParseError as e:
        raise ET.ParseError(f"Failed to parse XML from URL '{path}': {e}") from e
    except json.JSONDecodeError as e:
        raise RuntimeError(f"File '{p}' does not contain valid JSON. Error: {e}") from e
    except PermissionError as e:
        raise PermissionError(f"Permission denied when reading file '{p}'.") from e


def _read_spec_file(path_or_url: str, spec_type: Literal["json", "xml"] = "json"):
    # Load JSON from URL
    if path_or_url.startswith(("http://", "https://")):
        return _read_spec_file_from_url(path_or_url, spec_type)
    else:
        return _read_spec_file_from_path(path_or_url, spec_type)


def _normalize_names(names: Optional[Union[List, Dict]]) -> Optional[List]:
    """Helper for normalizing inside dataset features:
    list, dict, or None -> return list or None"""

    if names is None:
        return None

    if isinstance(names, List):
        return names

    if isinstance(names, Dict):
        result = []

        def recurse(prefix, obj):
            if isinstance(obj, List):
                for item in obj:
                    result.append(f"{prefix}.{item}")
            elif isinstance(obj, Dict):
                for k, v in obj.items():
                    recurse(f"{prefix}.{k}" if prefix else k, v)

        recurse("", names)
        return result


def _normalize_entry(spec: Dict) -> Dict:
    """Helper for normalizing dataset info entries"""

    # Map dtypes -> types
    # NOTE: As of now only dtypes are used by the server for sorting inputs
    TYPE_MAP = {
        "video": "VISUAL",
        "image": "VISUAL",
        "float32": "STATE",
        "float64": "STATE",
    }

    dtype = spec.get("dtype", "").lower()
    shape_raw = spec.get("shape", [])
    shape = tuple(shape_raw) if isinstance(shape_raw, (list, tuple)) else ()

    feature_type = TYPE_MAP.get(dtype, None)
    if not feature_type:
        return {}

    names = _normalize_names(spec.get("names"))

    entry = {
        "dtype": dtype,
        "shape": shape,
        "type": feature_type,
    }
    if names:
        entry["names"] = names

    return entry


def build_lerobot_features_from_dataset_info(
    path_or_url: str,
) -> Dict:
    """
    Load LeRobot dataset info.json from a local file or URL and build
    the feature and actions dict.
    """
    # Read features dict
    dataset_json = _read_spec_file(path_or_url)

    if not dataset_json:
        raise RuntimeError(f"Could not read spec file at {path_or_url}")

    # Build feature dictionary
    raw_features = dataset_json.get("features", {})

    features = {}
    image_keys = []
    actions = {}

    for key, spec in raw_features.items():
        # NOTE: Only checking for state, images and action for now
        if key == "observation.state":
            features[key] = _normalize_entry(spec)
        elif key.startswith("observation.images."):
            features[key] = _normalize_entry(spec)
            image_keys.append(key.removeprefix("observation.images."))

    action_spec = raw_features.get("action", {})
    actions = _normalize_entry(action_spec)

    return {"features": features, "actions": actions, "image_keys": image_keys}


class PDFReader:
    """Load pdf using pdfreader. Used for testing PDF RAG"""

    @validate_func_args
    def __init__(
        self,
        pdf_file: Union[str, Path],
        password: Optional[str] = None,
    ) -> None:
        """__init__.
        :param pdf_file:
        :type pdf_file: Union[str, Path]
        :param password:
        :type password: Optional[str]
        :rtype: None
        """
        try:
            from pypdf import PdfReader
            from PIL import Image
        except ModuleNotFoundError as e:
            raise ModuleNotFoundError(
                "In order to use the PDFReader, you need pypdf and pillow packages installed. You can install them with 'pip install pypdf pillow'"
            ) from e
        if not Path(pdf_file).is_file():
            raise TypeError(f"{pdf_file} is not a valid file")
        try:
            self.reader = PdfReader(pdf_file, password=password)
            self.image_reader = Image
        except Exception as e:
            raise TypeError(f"{pdf_file} is not a valid PDF") from e

    def extract(self, extract_images: bool = False):
        """Extract text from PDF documents.
        :param extract_images
        :type extract_images: bool
        """
        metadatas = []
        documents = []
        images = []
        ids = []

        for page_num, page in enumerate(self.reader.pages):
            # add pdf metadata as reader metadata and add pagenumber to it
            metadata = self.reader.metadata if self.reader.metadata else {}
            metadata["page"] = page_num
            metadatas.append(metadata)
            # get content
            content = page.get_contents()
            documents.append(content)
            # create a unique ID
            ids.append(str(uuid.uuid5(uuid.NAMESPACE_DNS, content)))
            # get images if asked
            if extract_images:
                page_images = [
                    self.image_reader.open(BytesIO(img.data)) for img in page.images
                ]
                images += page_images

        return ids, metadatas, documents


_LANGUAGE_CODES = [
    "af",
    "am",
    "ar",
    "as",
    "az",
    "ba",
    "be",
    "bg",
    "bn",
    "bo",
    "br",
    "bs",
    "ca",
    "cs",
    "cy",
    "da",
    "de",
    "el",
    "en",
    "es",
    "et",
    "eu",
    "fa",
    "fi",
    "fo",
    "fr",
    "gl",
    "gu",
    "ha",
    "haw",
    "he",
    "hi",
    "hr",
    "ht",
    "hu",
    "hy",
    "id",
    "is",
    "it",
    "ja",
    "jw",
    "ka",
    "kk",
    "km",
    "kn",
    "ko",
    "la",
    "lb",
    "ln",
    "lo",
    "lt",
    "lv",
    "mg",
    "mi",
    "mk",
    "ml",
    "mn",
    "mr",
    "ms",
    "mt",
    "my",
    "ne",
    "nl",
    "nn",
    "no",
    "oc",
    "pa",
    "pl",
    "ps",
    "pt",
    "ro",
    "ru",
    "sa",
    "sd",
    "si",
    "sk",
    "sl",
    "sn",
    "so",
    "sq",
    "sr",
    "su",
    "sv",
    "sw",
    "ta",
    "te",
    "tg",
    "th",
    "tk",
    "tl",
    "tr",
    "tt",
    "uk",
    "ur",
    "uz",
    "vi",
    "yi",
    "yo",
    "zh",
    "yue",
]
