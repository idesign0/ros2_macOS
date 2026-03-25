"""
The following vector DB specification classes are meant to define a comman interface for initialization of vector DBs. Currently the only supported vector DB is Chroma.
"""

from typing import Optional, Dict

from agents.ros import base_validators
from attrs import define, field
from .ros import BaseAttrs

__all__ = ["ChromaDB"]


@define(kw_only=True)
class DB(BaseAttrs):
    """This class describes a database initialization configuration."""

    username: Optional[str] = field(default=None)
    password: Optional[str] = field(default=None)
    init_timeout: int = field(default=600)  # 10 minutes

    def get_init_params(self) -> Dict:
        """Get init params from models"""
        return self._get_init_params()

    def _get_init_params(self) -> Dict:
        raise NotImplementedError(
            "_get_init_params method needs to be implemented by vectordb definition classes"
        )


@define(kw_only=True)
class ChromaDB(DB):
    """[Chroma](https://www.trychroma.com/) is the open-source AI application database. It provides embeddings, vector search, document storage, full-text search, metadata filtering, and multi-modal retreival support.

    :param username: The username for authentication. Defaults to None.
    :type username: Optional[str], optional
    :param password: The password for authentication. Defaults to None.
    :type password: Optional[str], optional
    :param embeddings: Embedding backend to use. Choose from "ollama" or "sentence-transformers".
    :type embeddings: str, optional
    :param checkpoint: The model checkpoint to use for embeddings. For example, "bge-large:latest".
    :type checkpoint: str, optional
    :param ollama_host: Host address for the Ollama service (used if embeddings="ollama").
    :type ollama_host: str, optional
    :param ollama_port: Port number for the Ollama service.
    :type ollama_port: int, optional
    :param init_timeout: The timeout in seconds for the initialization process. Defaults to 10 minutes (600 seconds).
    :type init_timeout: int, optional

    To use ChromaDB with the supported embedding backends, install the following Python packages:

    ```bash
    pip install ollama  # if using ollama (requires separate Ollama runtime)
    pip install sentence-transformers  # if using sentence-transformers
    ```

    If using the Ollama backend, make sure the Ollama server is running and accessible at the specified host and port.

    Example Usage:

    ```python
    from agents.vectordbs import ChromaDB

    chroma_config = ChromaDB(
        embeddings='ollama',
        checkpoint='bge-large:latest',
        ollama_host='localhost',
        ollama_port=11434
    )
    ```
    """

    embeddings: str = field(
        default="ollama",
        validator=base_validators.in_(["ollama", "sentence-transformers"]),
    )
    checkpoint: str = field(default="bge-large:latest")
    ollama_host: str = field(default="127.0.0.1")
    ollama_port: int = field(default=11434)

    def _get_init_params(self) -> Dict:
        params = {
            "username": self.username,
            "password": self.password,
            "embeddings": self.embeddings,
            "checkpoint": self.checkpoint,
            "ollama_host": self.ollama_host,
            "ollama_port": self.ollama_port,
            "init_timeout": self.init_timeout,
        }
        return params
