from abc import ABC, abstractmethod
from typing import Any, Optional, Dict, Union

from rclpy import logging

from ..vectordbs import DB
from ..utils import validate_func_args


class DBClient(ABC):
    """DBClient."""

    @validate_func_args
    def __init__(
        self,
        db: Union[DB, Dict],
        host: Optional[str] = None,
        port: Optional[int] = None,
        response_timeout: int = 30,
        init_on_activation: bool = True,
        logging_level: str = "info",
        **_,
    ):
        """__init__.
        :param db:
        :type db: DB
        :param host:
        :type host: Optional[str]
        :param port:
        :type port: Optional[int]
        :param init_on_activation:
        :type init_on_activation: bool
        :param logging_level:
        :type logging_level: str
        """
        if isinstance(db, DB):
            self.db_type = db.__class__.__name__
            self.init_timeout = db.init_timeout
            self.db_init_params = db._get_init_params()

        else:
            self.db_type = db["db_type"]
            self.init_timeout = db["init_timeout"]
            self.db_init_params = db["db_init_params"]

        self.host = host
        self.port = port
        self.init_on_activation = init_on_activation
        self.logger = logging.get_logger(self.db_type)
        logging.set_logger_level(
            self.db_type, logging.get_logging_severity_from_string(logging_level)
        )
        self.response_timeout = response_timeout

    def serialize(self) -> Dict:
        """Get client json
        :rtype: Dict
        """
        db = {
            "db_type": self.db_type,
            "init_timeout": self.init_timeout,
            "db_init_params": self.db_init_params,
        }

        return {
            "client_type": self.__class__.__name__,
            "db": db,
            "host": self.host,
            "port": self.port,
            "init_on_activation": self.init_on_activation,
            "logging_level": self.logger.get_effective_level().name,
            "response_timeout": self.response_timeout,
        }

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

    def add(self, db_input: Dict[str, Any]) -> Optional[Dict]:
        """add data.
        :param db_input:
        :type db_input: dict[str, Any]
        :rtype: dict | None
        """
        return self._add(db_input)

    def conditional_add(self, db_input: Dict[str, Any]) -> Optional[Dict]:
        """add data if given ids dont exist. Update metadatas of the ids that exist
        :param db_input:
        :type db_input: dict[str, Any]
        :rtype: dict | None
        """
        return self._conditional_add(db_input)

    def metadata_query(self, db_input: Dict[str, Any]) -> Optional[Dict]:
        """Query based on given metadata.
        :param db_input:
        :type db_input: dict[str, Any]
        :rtype: dict | None
        """
        return self._metadata_query(db_input)

    def query(self, db_input: Dict[str, Any]) -> Optional[Dict]:
        """Query based on query string.
        :param db_input:
        :type db_input: dict[str, Any]
        :rtype: dict | None
        """
        return self._query(db_input)

    def deinitialize(self) -> None:
        """deinitialize."""
        # TODO: Add check for db initialization by keeping db
        # state in client. Currently always deinitialize db client
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
    def _add(self, db_input: Dict[str, Any]) -> Optional[Dict]:
        """add data.
        :param db_input:
        :type db_input: dict[str, Any]
        :rtype: dict | None
        """
        raise NotImplementedError(
            "_add method needs to be implemented in a child class"
        )

    @abstractmethod
    def _conditional_add(self, db_input: Dict[str, Any]) -> Optional[Dict]:
        """add data if given ids dont exist. Update metadatas of the ids that exist
        :param db_input:
        :type db_input: dict[str, Any]
        :rtype: dict | None
        """
        raise NotImplementedError(
            "_conditional_add method needs to be implemented in a child class"
        )

    @abstractmethod
    def _metadata_query(self, db_input: Dict[str, Any]) -> Optional[Dict]:
        """Query based on given metadata.
        :param db_input:
        :type db_input: dict[str, Any]
        :rtype: dict | None
        """
        raise NotImplementedError(
            "_metadata_query method needs to be implemented in a child class"
        )

    @abstractmethod
    def _query(self, db_input: Dict[str, Any]) -> Optional[Dict]:
        """Query based on query string.
        :param db_input:
        :type db_input: dict[str, Any]
        :rtype: dict | None
        """
        raise NotImplementedError(
            "_query method needs to be implemented in a child class"
        )

    @abstractmethod
    def _deinitialize(self) -> None:
        """deinitialize."""
        raise NotImplementedError(
            "_deinitialize method needs to be implemented in a child class"
        )
