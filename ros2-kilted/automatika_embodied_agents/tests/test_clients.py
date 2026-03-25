import logging
import time
import subprocess
import shutil

import cv2
import pytest

_IMPORTS_AVAILABLE = True
try:
    from agents.models import Idefics2, OllamaModel
    from agents.vectordbs import ChromaDB
    from agents.clients.roboml import (
        HTTPModelClient,
        HTTPDBClient,
        RESPDBClient,
        RESPModelClient,
    )
    from agents.clients.ollama import OllamaClient
except ImportError:
    _IMPORTS_AVAILABLE = False

pytestmark = pytest.mark.skipif(
    not _IMPORTS_AVAILABLE,
    reason="Required packages for client integration tests not available",
)

HOST = "http://localhost"
RAY_PORT = 8000
RESP_PORT = 6379


@pytest.fixture(scope="class")
def http_clients():
    """Fixture to run roboml ray and make its clients before tests are run"""

    # start server
    p = subprocess.Popen(["roboml"])
    # give it 20 seconds to start before sending request
    time.sleep(20)
    model = Idefics2(name="idefics")
    model_client = HTTPModelClient(model, port=RAY_PORT, logging_level="debug")
    db = ChromaDB(name="chroma", db_location="./http_data")
    db_client = HTTPDBClient(db, port=RAY_PORT, logging_level="debug")

    yield {"model": model_client, "db": db_client}

    # terminate server process - kill to remove ray monitoring child
    p.kill()
    shutil.rmtree("./http_data")


@pytest.fixture(scope="class")
def resp_clients():
    """Fixture to run roboml-resp and make its clients before tests are run"""

    # start server
    p = subprocess.Popen(["roboml-resp"])
    # give it 20 seconds to start before sending request
    time.sleep(20)
    model = Idefics2(name="idefics")
    model_client = RESPModelClient(model, logging_level="debug")
    db = ChromaDB(name="chroma", db_location="./resp_data")
    db_client = RESPDBClient(db, logging_level="debug")

    yield {"model": model_client, "db": db_client}

    # terminate server process
    p.terminate()
    shutil.rmtree("./resp_data")


@pytest.fixture(scope="class")
def ollama_client():
    """Fixture to create client ollama before tests are run"""

    model = OllamaModel(name="llava", checkpoint="llava")
    ollama_client = OllamaClient(model, logging_level="debug")
    yield ollama_client


@pytest.fixture
def loaded_img():
    """Fixture to load test image"""
    return cv2.imread("agents/resources/test.jpeg", cv2.COLOR_BGR2RGB)


@pytest.fixture
def data():
    return {
        "ids": ["a"],
        "metadatas": [{"something": "about a"}],
        "documents": ["description of a"],
        "collection_name": "alphabets",
    }


@pytest.mark.local_only
class TestRobomlHTTPClient:
    """
    Test roboml http client
    """

    def test_model_init(self, http_clients):
        """
        Test roboml http model client init
        """
        try:
            http_clients["model"].check_connection()
        except Exception:
            logging.error(
                "Make sure roboml is installed on this machine before running these tests. roboml can be installed with `pip install roboml`"
            )
            raise
        http_clients["model"].initialize()

    def test_model_inference(self, http_clients, loaded_img):
        """
        Test roboml http model client inference
        """
        inference_input = {"query": "What do you see?", "images": [loaded_img]}
        result = http_clients["model"].inference(inference_input)
        assert result is not None
        assert result["output"] is not None
        logging.info(result["output"])

    def test_model_deinit(self, http_clients):
        """
        Test roboml http model client deinit
        """
        http_clients["model"].deinitialize()

    def test_db_init(self, http_clients):
        """
        Test roboml http db client init
        """
        http_clients["db"].check_connection()
        http_clients["db"].initialize()

    def test_db_add(self, http_clients, data):
        """
        Test roboml http db client add
        """
        result = http_clients["db"].add(data)
        assert result is not None
        assert result["output"] is not None
        logging.info(result["output"])

    def test_db_conditional_add(self, http_clients, data):
        """
        Test roboml http db client conditional add
        """
        result = http_clients["db"].conditional_add(data)
        assert result is not None
        assert result["output"] is not None
        logging.info(result["output"])

    def test_db_metadata_query(self, http_clients, data):
        """
        Test roboml http db client metadata query
        """
        metadata_query = {
            "metadatas": data["metadatas"],
            "collection_name": data["collection_name"],
        }
        result = http_clients["db"].metadata_query(metadata_query)
        assert result is not None
        assert result["output"] is not None
        logging.info(result["output"])

    def test_db_query(self, http_clients, data):
        """
        Test roboml http db client query
        """
        metadata_query = {
            "query": "what is a",
            "collection_name": data["collection_name"],
        }
        result = http_clients["db"].query(metadata_query)
        assert result is not None
        assert result["output"] is not None
        logging.info(result["output"])

    def test_db_deinit(self, http_clients):
        """
        Test roboml http db client deinit
        """
        http_clients["db"].deinitialize()


@pytest.mark.local_only
class TestRobomlRESPClient:
    """
    Test roboml resp client
    """

    def test_model_init(self, resp_clients):
        """
        Test roboml resp model client init
        """
        try:
            resp_clients["model"].check_connection()
        except Exception:
            logging.error(
                "Make sure roboml is installed on this machine before running these tests. roboml can be installed with `pip install roboml`"
            )
            raise
        resp_clients["model"].initialize()

    def test_model_inference(self, resp_clients, loaded_img):
        """
        Test roboml resp model client inference
        """
        inference_input = {"query": "What do you see?", "images": [loaded_img]}
        result = resp_clients["model"].inference(inference_input)
        assert result is not None
        assert result["output"] is not None
        logging.info(result["output"])

    def test_model_deinit(self, resp_clients):
        """
        Test roboml resp model client deinit
        """
        resp_clients["model"].deinitialize()

    def test_db_init(self, resp_clients):
        """
        Test roboml resp db client init
        """
        resp_clients["db"].check_connection()
        resp_clients["db"].initialize()

    def test_db_add(self, resp_clients, data):
        """
        Test roboml resp db client add
        """
        result = resp_clients["db"].add(data)
        assert result is not None
        assert result["output"] is not None
        logging.info(result["output"])

    def test_db_conditional_add(self, resp_clients, data):
        """
        Test roboml resp db client conditional add
        """
        result = resp_clients["db"].conditional_add(data)
        assert result is not None
        assert result["output"] is not None
        logging.info(result["output"])

    def test_db_metadata_query(self, resp_clients, data):
        """
        Test roboml resp db client metadata query
        """
        metadata_query = {
            "metadatas": data["metadatas"],
            "collection_name": data["collection_name"],
        }
        result = resp_clients["db"].metadata_query(metadata_query)
        assert result is not None
        assert result["output"] is not None
        logging.info(result["output"])

    def test_db_query(self, resp_clients, data):
        """
        Test roboml resp db client query
        """
        metadata_query = {
            "query": "what is a",
            "collection_name": data["collection_name"],
        }
        result = resp_clients["db"].query(metadata_query)
        assert result is not None
        assert result["output"] is not None
        logging.info(result["output"])

    def test_db_deinit(self, resp_clients):
        """
        Test roboml resp db client deinit
        """
        resp_clients["db"].deinitialize()


@pytest.mark.local_only
class TestOllamaClient:
    """
    Test ollama client
    """

    def test_model_init(self, ollama_client):
        """
        Test ollama model client init
        """
        try:
            ollama_client.check_connection()
        except Exception:
            logging.error(
                "Make sure Ollama is installed on this machine before running these tests. Visit https://ollama.com for installation instructions."
            )
            raise
        ollama_client.initialize()

    def test_model_inference(self, ollama_client, loaded_img):
        """
        Test ollama model client inference
        """
        inference_input = {"query": "What do you see?", "images": [loaded_img]}
        result = ollama_client.inference(inference_input)
        assert result is not None
        assert result["output"] is not None
        logging.info(result["output"])

    def test_model_deinit(self, ollama_client):
        """
        Test ollama model client deinit
        """
        ollama_client.deinitialize()
