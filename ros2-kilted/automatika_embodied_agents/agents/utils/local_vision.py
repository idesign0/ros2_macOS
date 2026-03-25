import cv2
from typing import Dict, Optional
import numpy as np
import logging

try:
    import onnxruntime as ort
except ModuleNotFoundError as e:
    raise ModuleNotFoundError(
        """enable_local_classifier in Vision component requires onnxruntime to be installed. Please install them with `pip install onnxruntime` or `pip install onnxruntime-gpu` for cpu or gpu based deployment.

        For Jetson devices you can download the pre-built ONNX runtime wheels corresponding to your Jetpack version at https://elinux.org/Jetson_Zoo#ONNX_Runtime"""
    ) from e

from .voice import _get_onnx_providers


class LocalVisionModel:
    """Implements inference for a fast local detection model. The default model selected model is:
          @misc{huang2024deim,
          title={DEIM: DETR with Improved Matching for Fast Convergence},
          author={Shihua Huang, Zhichao Lu, Xiaodong Cun, Yongjun Yu, Xiao Zhou, and Xi Shen},
          booktitle={Proceedings of the IEEE/CVF Conference on Computer Vision and Pattern Recognition},
          year={2025},
    }
    """

    def __init__(
        self,
        model_path: str,
        ncpu: int = 1,
        device: str = "cpu",
        input_height: int = 640,
        input_width: int = 640,
        dataset_labels: Optional[Dict] = None,
    ):
        # Initialize the ONNX model
        sessionOptions = ort.SessionOptions()
        sessionOptions.inter_op_num_threads = ncpu
        sessionOptions.intra_op_num_threads = ncpu

        providers = _get_onnx_providers(device, "local_classifier")
        self.model = ort.InferenceSession(
            model_path, sess_options=sessionOptions, providers=providers
        )
        self.input_height = input_height
        self.input_width = input_width
        self.dataset_labels = dataset_labels or {}
        self.original_w: int = 0
        self.original_h: int = 0

    def __resize_with_aspect_ratio(
        self, image, height, width, interpolation=cv2.INTER_LINEAR
    ):
        """Resizes an image while maintaining aspect ratio and pads it."""
        ratio = min(width / self.original_w, height / self.original_h)
        new_width = int(self.original_w * ratio)
        new_height = int(self.original_h * ratio)

        # Resize the image
        resized_image = cv2.resize(
            image, (new_width, new_height), interpolation=interpolation
        )

        # Create a new image with the desired size and paste the resized image onto it
        new_image = np.full((height, width, 3), 128, np.uint8)  # Create a gray canvas
        pad_top = (height - new_height) // 2
        pad_left = (width - new_width) // 2
        new_image[pad_top : pad_top + new_height, pad_left : pad_left + new_width] = (
            resized_image
        )

        im_t = np.transpose(new_image, (2, 0, 1))  # HWC to CHW

        return im_t, ratio, pad_left, pad_top

    def __scale_boxes(
        self,
        boxes: np.ndarray,
        original_width: int,
        original_height: int,
        ratio: float,
        pad_left: int,
        pad_top: int,
    ) -> np.ndarray:
        """
        Rescales bounding boxes from the model's input size back to the original image size.
        """
        # Adjust for padding
        boxes[:, [0, 2]] -= pad_left
        boxes[:, [1, 3]] -= pad_top

        # Adjust for scaling ratio
        boxes /= ratio

        # Clip coordinates to ensure they are within the original image boundaries
        boxes[:, [0, 2]] = boxes[:, [0, 2]].clip(0, original_width)
        boxes[:, [1, 3]] = boxes[:, [1, 3]].clip(0, original_height)

        return boxes

    def __call__(
        self,
        inference_input: Dict,
    ) -> Optional[Dict]:
        """
        Inference for vision model.

        :param inference_input: Dictionary containing images and inference parameters.
        :param stream: Ignored. Vision does not support streaming. Present for
            signature compatibility with ``_call_inference``.
        """
        try:
            # Create the size array using NumPy, matching the expected int64 dtype
            orig_size_np = np.array(
                [[self.input_height, self.input_width]], dtype=np.int64
            )

            # NOTE: Handles only one image in the input
            input_image = inference_input["images"][0]
            self.original_h, self.original_w = input_image.shape[:2]

            # Preprocess the image and retrieve the scaling parameters
            processed_image, ratio, pad_left, pad_top = self.__resize_with_aspect_ratio(
                input_image, self.input_height, self.input_width
            )

            # Normalize the image and add a batch dimension
            im_data_np = (
                np.array([processed_image], dtype=np.float32)
                / 255.0  # Normalize to [0, 1]
            )

            results = []

            detections = self.model.run(
                output_names=None,
                input_feed={"images": im_data_np, "orig_target_sizes": orig_size_np},
            )

            # format results
            labels, boxes, scores = detections
            result = {}
            if boxes.size > 0:
                # filter for threshold
                mask = scores >= inference_input["threshold"]
                scores, labels, boxes = scores[mask], labels[mask], boxes[mask]
                # Check if predictions survived thresholding
                if scores.size > 0:
                    # Scale the boxes to match the original image dimensions
                    boxes = self.__scale_boxes(
                        boxes,
                        self.original_w,
                        self.original_h,
                        ratio,
                        pad_left,
                        pad_top,
                    )
                    # if labels are requested in text
                    if inference_input["get_dataset_labels"]:
                        # get text labels from model dataset info
                        labels = np.vectorize(
                            lambda x: self.dataset_labels[str(x)],
                        )(labels)

                result = {
                    "bboxes": boxes.tolist(),
                    "labels": labels.tolist(),
                    "scores": scores.tolist(),
                }

            if result:
                results.append(result)

        except Exception as e:
            logging.getLogger("local_classifier").error(
                f"Error in local_classifier: {e}"
            )
            return

        return {"output": results}


_MS_COCO_LABELS = {
    "0": "person",
    "1": "bicycle",
    "2": "car",
    "3": "motorcycle",
    "4": "airplane",
    "5": "bus",
    "6": "train",
    "7": "truck",
    "8": "boat",
    "9": "traffic light",
    "10": "fire hydrant",
    "11": "stop sign",
    "12": "parking meter",
    "13": "bench",
    "14": "bird",
    "15": "cat",
    "16": "dog",
    "17": "horse",
    "18": "sheep",
    "19": "cow",
    "20": "elephant",
    "21": "bear",
    "22": "zebra",
    "23": "giraffe",
    "24": "backpack",
    "25": "umbrella",
    "26": "handbag",
    "27": "tie",
    "28": "suitcase",
    "29": "frisbee",
    "30": "skis",
    "31": "snowboard",
    "32": "sports ball",
    "33": "kite",
    "34": "baseball bat",
    "35": "baseball glove",
    "36": "skateboard",
    "37": "surfboard",
    "38": "tennis racket",
    "39": "bottle",
    "40": "wine glass",
    "41": "cup",
    "42": "fork",
    "43": "knife",
    "44": "spoon",
    "45": "bowl",
    "46": "banana",
    "47": "apple",
    "48": "sandwich",
    "49": "orange",
    "50": "broccoli",
    "51": "carrot",
    "52": "hot dog",
    "53": "pizza",
    "54": "donut",
    "55": "cake",
    "56": "chair",
    "57": "couch",
    "58": "potted plant",
    "59": "bed",
    "60": "dining table",
    "61": "toilet",
    "62": "tv",
    "63": "laptop",
    "64": "mouse",
    "65": "remote",
    "66": "keyboard",
    "67": "cell phone",
    "68": "microwave",
    "69": "oven",
    "70": "toaster",
    "71": "sink",
    "72": "refrigerator",
    "73": "book",
    "74": "clock",
    "75": "vase",
    "76": "scissors",
    "77": "teddy bear",
    "78": "hair drier",
    "79": "toothbrush",
}
