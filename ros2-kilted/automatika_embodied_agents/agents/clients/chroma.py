from typing import Any, Optional, Dict, Union, cast, List

import httpx

from ..vectordbs import DB
from ..models import OllamaModel
from .db_base import DBClient
from .ollama import OllamaClient

__all__ = ["ChromaClient"]

# Define constants for default tenant and database in ChromaDB
DEFAULT_TENANT = "default_tenant"
DEFAULT_DATABASE = "default_database"


class ChromaClient(DBClient):
    """An HTTP client for interaction with a ChromaDB instance running as a server."""

    def __init__(
        self,
        db: Union[DB, Dict],
        host: str = "127.0.0.1",
        port: int = 8000,
        response_timeout: int = 30,
        init_on_activation: bool = True,
        logging_level: str = "info",
        **kwargs,
    ):
        super().__init__(
            db=db,
            host=host,
            port=port,
            response_timeout=response_timeout,
            init_on_activation=init_on_activation,
            logging_level=logging_level,
            **kwargs,
        )
        self.url = f"http://{self.host}:{self.port}/api/v2"

        # create httpx client
        self.client = httpx.Client(base_url=self.url, timeout=self.response_timeout)

        # create ollama client if using ollama embeddings
        if self.db_init_params["embeddings"] == "ollama":
            self.embeddings_client = OllamaClient(
                model=OllamaModel(
                    name="internal_ollama_embeddings",
                    checkpoint=self.db_init_params["checkpoint"],
                    init_timeout=self.db_init_params["init_timeout"],
                ),
                host=self.db_init_params["ollama_host"],
                port=self.db_init_params["ollama_port"],
                logging_level=logging_level,
            )
        else:
            try:
                from sentence_transformers import SentenceTransformer

                self.SentenceTransformer = SentenceTransformer
                self.embeddings_client: SentenceTransformer
            except ModuleNotFoundError as e:
                raise ModuleNotFoundError(
                    "In order to use the local models from sentence-transformers as your embedding model, you need sentence-transformers package installed. You can install it with 'pip install sentence-transformers'"
                ) from e

        self._check_connection()

    def _api_call(self, method: str, endpoint: str, **kwargs) -> Any:
        """Helper function to make API calls to ChromaDB server."""

        try:
            response = self.client.request(
                method, endpoint, **kwargs
            ).raise_for_status()
            # Handle responses that might not have content
            if (
                response.status_code == httpx.codes.OK
                or response.status_code == httpx.codes.CREATED
            ):
                if (
                    response.headers.get("content-type") == "application/json"
                    and response.content
                ):
                    return response.json()
                # For successful calls like DELETE collection or ADD to collection which Chroma might return 200/201 with no/empty JSON body
                return None
            return response.text  # Fallback for other types or non-empty non-JSON
        except httpx.RequestError as e:
            self.logger.error(f"ChromaDB request error for {method} {endpoint}: {e}")
            raise
        except httpx.HTTPStatusError as e:
            error_body = e.response.text
            try:  # Try to parse if error body is JSON
                error_json = e.response.json()
                if (
                    isinstance(error_json, dict) and "message" in error_json
                ):  # get any nested messages
                    error_message = error_json["message"]
                else:
                    error_message = error_json.get("error", str(e))
            except Exception:
                error_message = error_body if error_body else str(e)
            self.logger.error(
                f"ChromaDB API error for {method} {endpoint}: {e.response.status_code} - {error_message}"
            )
            raise
        except Exception as e:
            self.logger.error(
                f"Unexpected error during API call to {method} {endpoint}: {e}"
            )
            raise

    def _check_connection(self):
        """Check if the platfrom is being served on specified IP and port"""
        # Ping remote server to check connection
        if self.db_init_params.get("username") or self.db_init_params.get("password"):
            self.logger.warning(
                "Username/password authentication is not directly handled by this HTTP client. "
                "Ensure your ChromaDB server is secured appropriately (e.g., via network or a reverse proxy for auth)."
            )
        self.logger.info("Checking connection with ChromaDB")
        try:
            # check db heartbeat response
            self._api_call("GET", "/heartbeat")
        except Exception as e:
            self.logger.error(
                f"""Failed to connect to ChromaDB server at {self.url}: {e}

                Make sure an instance of chromaDB is running on the given url by executing the following command:

                `chroma run --path /db_path`

                To install ChromaDB, follow instructions on https://docs.trychroma.com/docs/overview/getting-started
                """
            )
            self.client.close()
            raise

    def _initialize(self) -> None:
        """
        Initialize any custom encoding model
        """
        if self.db_init_params["embeddings"] == "ollama":
            self.embeddings_client.initialize()
        else:
            try:
                self.embeddings_client = self.SentenceTransformer(
                    self.db_init_params["checkpoint"]
                )
            except Exception as e:
                self.logger.error(
                    f"Failed to load model local checkpoint {self.db_init_params['checkpoint']} from sentence_transformers. Please ensure that checkpoint name is correct. Got the following error: {e}"
                )
                raise

    def _get_or_create_collection(
        self,
        collection_name: str,
        distance_func: Optional[str] = None,
        metadata: Optional[Dict[str, Any]] = None,
        get_only: bool = False,
    ):
        effective_metadata = metadata or {}
        effective_metadata["hnsw:space"] = distance_func

        payload = {
            "name": collection_name,
            "metadata": effective_metadata,
            "get_or_create": True,
        }
        try:
            if get_only:
                collection_info = self._api_call(
                    "GET",
                    f"/tenants/{DEFAULT_TENANT}/databases/{DEFAULT_DATABASE}/collections/{collection_name}",
                )
            else:
                collection_info = self._api_call(
                    "POST",
                    f"/tenants/{DEFAULT_TENANT}/databases/{DEFAULT_DATABASE}/collections",
                    json=payload,
                )
            return cast(str, collection_info["id"])
        except Exception:
            self.logger.error(f"Failed to get or create collection '{collection_name}'")
            raise

    def _add(self, db_input: Dict[str, Any]) -> Optional[Dict]:
        """Add data.
        :param db_input:
        :type db_input: dict[str, Any]
        :rtype: dict | None
        """
        try:
            if db_input.get("reset_collection"):
                try:
                    self.logger.info(
                        f"Resetting collection: {db_input['collection_name']}"
                    )
                    collection_id = self._get_or_create_collection(
                        db_input["collection_name"],
                        get_only=True,
                    )
                    self._api_call(
                        "DELETE",
                        f"/tenants/{DEFAULT_TENANT}/databases/{DEFAULT_DATABASE}/collections/{db_input['collection_name']}",
                    )
                    self.logger.info(
                        f"Successfully deleted collection {db_input['collection_name']} as part of reset."
                    )
                except httpx.HTTPStatusError as e:
                    if e.response.status_code == 404:
                        self.logger.warning(
                            f"Collection {db_input['collection_name']} not found during reset. No need to delete."
                        )
                except Exception as e:
                    self.logger.warning(
                        f"Could not verify or delete collection {db_input['collection_name']} during reset (it might not exist or another error occurred): {e}"
                    )

            collection_id = self._get_or_create_collection(
                db_input["collection_name"], db_input.get("distance_func")
            )

            # Create embeddings
            if (embeddings := self._embed(db_input["documents"])) is None:
                return

            add_payload = {
                "embeddings": embeddings,
                "documents": db_input["documents"],
                "metadatas": db_input["metadatas"],
                "ids": db_input["ids"],
            }
            self._api_call(
                "POST",
                f"/tenants/{DEFAULT_TENANT}/databases/{DEFAULT_DATABASE}/collections/{collection_id}/add",
                json=add_payload,
            )
            self.logger.debug(
                f"Successfully added {len(db_input['ids'])} items to collection '{db_input['collection_name']}'."
            )

        except Exception as e:
            self.logger.error(f"Exception occurred in _add: {e}")
            return

        return {"output": "Success"}

    def _conditional_add(self, db_input: Dict[str, Any]) -> Optional[Dict]:
        """Add data only if the ids dont exist. Otherwise update metadatas
        :param db_input:
        :type db_input: dict[str, Any]
        :rtype: dict | None
        """
        collection_id = self._get_or_create_collection(
            db_input["collection_name"], db_input.get("distance_func")
        )

        # Create embeddings
        if (embeddings := self._embed(db_input["documents"])) is None:
            return

        upsert_payload = {
            "ids": db_input["ids"],
            "metadatas": db_input["metadatas"],
            "documents": db_input["documents"],
            "embeddings": embeddings,
        }
        try:
            self._api_call(
                "POST",
                f"/tenants/{DEFAULT_TENANT}/databases/{DEFAULT_DATABASE}/collections/{collection_id}/upsert",
                json=upsert_payload,
            )
            self.logger.debug(
                f"Successfully upserted {len(db_input['ids'])} items into collection '{db_input['collection_name']}' (ID: {collection_id})."
            )
        except Exception as e:
            self.logger.error(f"Exception occurred in _conditional_add (upsert): {e}")
            return

        return {"output": "Success"}

    def _metadata_query(self, db_input: Dict[str, Any]) -> Optional[Dict]:
        """Query based on given metadata.
        :param db_input:
        :type db_input: dict[str, Any]
        :rtype: dict | None
        """
        all_filters = []
        for metadata_item in db_input["metadatas"]:
            if len(metadata_item) > 1:
                current_filter = {
                    "$and": [{k: {"$eq": v}} for k, v in metadata_item.items()]
                }
            elif len(metadata_item) == 1:
                key, value = list(metadata_item.items())[0]
                current_filter = {key: {"$eq": value}}
            else:
                continue
            all_filters.append(current_filter)

        if not all_filters:
            self.logger.warning(
                "The metadata filters received were empty. Returning empty list."
            )
            return {"output": []}

        final_filters = {"$or": all_filters} if len(all_filters) > 1 else all_filters[0]

        output = []
        try:
            collection_id = self._get_or_create_collection(
                db_input["collection_name"], get_only=True
            )
            get_payload = {
                "where": final_filters,
                "include": ["documents", "metadatas"],
            }
            output = self._api_call(
                "POST",
                f"/tenants/{DEFAULT_TENANT}/databases/{DEFAULT_DATABASE}/collections/{collection_id}/get",
                json=get_payload,
            )
            self.logger.debug(
                f"Metadata query returned {len(output.get('ids', [])) if output else 0} results from '{db_input['collection_name']}' (ID: {collection_id})."
            )
        except Exception as e:
            self.logger.error(f"Exception occurred in _metadata_query: {e}")

        return {"output": output} if output else None

    def _query(self, db_input: Dict[str, Any]) -> Optional[Dict]:
        """Query using a query string.
        :param db_input:
        :type db_input: dict[str, Any]
        :rtype: dict | None
        """
        output = []
        try:
            collection_id = self._get_or_create_collection(
                db_input["collection_name"], get_only=True
            )

            # Create embeddings
            if (query_embeddings := self._embed(db_input["query"])) is None:
                return

            query_payload = {
                "query_embeddings": query_embeddings,
                "n_results": db_input["n_results"],
                "include": ["documents", "metadatas", "distances"],
            }
            api_result = self._api_call(
                "POST",
                f"/tenants/{DEFAULT_TENANT}/databases/{DEFAULT_DATABASE}/collections/{collection_id}/query",
                json=query_payload,
            )
            output = api_result if api_result else []
            self.logger.debug(
                f"Vector query returned results from '{db_input['collection_name']}' (ID: {collection_id}). Count: {len(output.get('ids', [[]])[0]) if output and output.get('ids') else 0}"
            )
        except Exception as e:
            self.logger.error(f"Exception occurred in _query: {e}")

        return {"output": output} if output else None

    def _deinitialize(self) -> None:
        """Deinitialize DB client"""
        if self.db_init_params["ollama"]:
            self.embeddings_client.initialize()
        self.client.close()
        self.logger.info("ChromaDB HTTP client closed.")

    def _embed(self, input: Union[str, List[str]]) -> Optional[List[List]]:
        # Create embeddings
        if self.db_init_params["embeddings"] == "ollama":
            embeddings = self.embeddings_client._embed(input)
        else:
            if isinstance(
                input, str
            ):  # ChromaDB requires a list even for one embedding
                input = [input]
            embeddings = self.embeddings_client.encode(input).tolist()
        if embeddings is None:
            self.logger.error(
                f"Could not create embeddings using {self.db_init_params['embeddings']}"
            )
            return

        return embeddings
