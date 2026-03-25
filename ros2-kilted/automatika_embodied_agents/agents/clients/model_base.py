from abc import ABC, abstractmethod
from typing import Any, Optional, Dict, Union, Generator, MutableMapping

from rclpy import logging

from ..models import Model
from ..utils import validate_func_args


class ModelClient(ABC):
    """MLClient."""

    @validate_func_args
    def __init__(
        self,
        model: Union[Model, Dict],
        host: Optional[str] = None,
        port: Optional[int] = None,
        inference_timeout: int = 30,
        init_on_activation: bool = True,
        logging_level: str = "info",
        **_,
    ):
        """__init__.
        :param model:
        :type model: Model
        :param host:
        :type host: Optional[str]
        :param port:
        :type port: Optional[int]
        :param inference_timeout:
        :type inference_timeout: int
        :param logging_level:
        :type logging_level: str
        """
        if isinstance(model, Model):
            self._model = model
            self.model_type = model.__class__.__name__
            self.model_name = model.name
            self.init_timeout = model.init_timeout
            self.model_init_params = model._get_init_params()

        else:
            self.model_type = model["model_type"]
            self.model_name = model["model_name"]
            self.init_timeout = model["init_timeout"]
            self.model_init_params = model["model_init_params"]

        self.host = host
        self.port = port
        self.init_on_activation = init_on_activation
        self.logger = logging.get_logger(self.model_name)
        logging.set_logger_level(
            self.model_name, logging.get_logging_severity_from_string(logging_level)
        )
        self.inference_timeout = inference_timeout

    def serialize(self) -> Dict:
        """Get client json
        :rtype: Dict
        """
        model = {
            "model_name": self.model_name,
            "model_type": self.model_type,
            "init_timeout": self.init_timeout,
            "model_init_params": self.model_init_params,
        }

        return {
            "client_type": self.__class__.__name__,
            "model": model,
            "host": self.host,
            "port": self.port,
            "init_on_activation": self.init_on_activation,
            "logging_level": self.logger.get_effective_level().name,
            "inference_timeout": self.inference_timeout,
        }

    @property
    def supports_tool_calls(self) -> bool:
        """
        Check if the client supports tool calling (function calling).
        Defaults to False. Override in subclasses if supported.

        :rtype: bool
        """
        return False

    def check_connection(self) -> None:
        """initialize.
        :rtype: None
        """
        self._check_connection()

    def initialize(self) -> None:
        """initialize.
        :rtype: None
        """
        if self.init_on_activation:
            self._initialize()

    def inference(self, inference_input: Dict[str, Any]) -> Optional[MutableMapping]:
        """inference.
        :param inference_input:
        :type inference_input: dict[str, Any]
        :rtype: dict | None
        """
        return self._inference(inference_input)

    def deinitialize(self):
        """deinitialize."""
        # TODO: Add check for model initialization by keeping model
        # state in client
        if self.init_on_activation:
            self._deinitialize()

    @abstractmethod
    def _check_connection(self) -> None:
        """check_connection.
        :rtype: None
        """
        raise NotImplementedError(
            "_check_connection method needs to be implemented in a child class"
        )

    @abstractmethod
    def _initialize(self) -> None:
        """initialize.
        :rtype: None
        """
        raise NotImplementedError(
            "_initialize method needs to be implemented in a child class"
        )

    @abstractmethod
    def _inference(
        self, inference_input: Dict[str, Any]
    ) -> Optional[MutableMapping[str, Union[str, Generator]]]:
        """inference.
        :param inference_input:
        :type inference_input: dict[str, Any]
        :rtype: dict | None
        """
        raise NotImplementedError(
            "_inference method needs to be implemented in a child class"
        )

    @abstractmethod
    def _deinitialize(self):
        """deinitialize."""
        raise NotImplementedError(
            "_deinitialize method needs to be implemented in a child class"
        )
