from io import BytesIO
from typing import Dict, Union
import time
import pickle
import threading
from .model_base import ModelClient
from ..models import LeRobotPolicy

from .lerobot_transport.utils import (
    RemotePolicyConfig,
    TimedObservation,
)

__all__ = ["LeRobotClient"]

LEROBOT_MIN_VERSION = "0.4.2"


class LeRobotClient(ModelClient):
    """A GRPC client for interaction with robot action policies served on LeRobot"""

    def __init__(
        self,
        model: Union[LeRobotPolicy, Dict],
        host: str = "127.0.0.1",
        port: int = 8080,
        inference_timeout: int = 30,  # Currently not used in this async client
        init_on_activation: bool = True,
        logging_level: str = "info",
        **kwargs,
    ):
        try:
            from .lerobot_transport import services_pb2, services_pb2_grpc

            self.services_pb2 = services_pb2
            self.services_pb2_grpc = services_pb2_grpc
            from grpc import insecure_channel

            self.channel = insecure_channel(f"{host}:{port}")
            self.stub = self.services_pb2_grpc.AsyncInferenceStub(self.channel)

            # TODO: Remove torch depenedency here once server can send numpy arrays
            from torch import load, storage

            self.torch_load = load
            self.torch_storage = storage

        except ModuleNotFoundError as e:
            raise ModuleNotFoundError(
                """In order to use the LeRobotClient, you need grpc and torch installed. This client uses grpc and LeRobot's policy server returns torch tensors. You can install it with:
            'pip install grpcio protobuf'
    And a lightweight CPU version (recommended) of torch with
            'pip install torch --index-url https://download.pytorch.org/whl/cpu'"""
            ) from e

        if (
            not isinstance(model, LeRobotPolicy)
            and model["model_type"] != "LeRobotPolicy"
        ):
            raise TypeError("LeRobotClient can only be used with LeRobotPolicy")

        super().__init__(
            model=model,
            host=host,
            port=port,
            inference_timeout=inference_timeout,
            init_on_activation=init_on_activation,
            logging_level=logging_level,
            **kwargs,
        )

        self.running = False
        self.lock = threading.Lock()
        self.latest_actions = []
        self._check_connection()

    def _check_connection(self) -> None:
        """Check if the platfrom is being served on specified IP and port"""
        # Ping remote server to check connection
        self.logger.info("Checking connection with remote LeRobot Policy Server")
        try:
            self.stub.Ready(self.services_pb2.Empty())
        except Exception:
            self.logger.error(
                f"""Failed to connect to LeRobot policy server at {self.host}:{self.port}

                Make sure a policy server is running on the given host and port by installing LeRobot >= {LEROBOT_MIN_VERSION} and executing the following command:

                `python -m lerobot.async_inference.policy_server --host={self.host} --port={self.port}`

                To install LeRobot >= {LEROBOT_MIN_VERSION}, follow instructions on https://huggingface.co/docs/lerobot/installation
                """
            )
            raise

    def _initialize(self):
        """Initialize policy on the server"""

        self.logger.info(f"Initializing {self.model_name} on LeRobot")

        # NOTE: server expects 'PolicyFeature' class.
        # However we are passing features as a dict which works
        config = RemotePolicyConfig(
            policy_type=self.model_init_params["policy_type"],
            pretrained_name_or_path=self.model_init_params["checkpoint"],
            lerobot_features=self.model_init_params["features"],
            actions_per_chunk=self.model_init_params["actions_per_chunk"],
            device=self.model_init_params["device"],
        )

        try:
            setup_msg = self.services_pb2.PolicySetup(data=pickle.dumps(config))
            self.stub.SendPolicyInstructions(setup_msg)

        except Exception as e:
            self.logger.error(
                f"Failed to initialize model {self.model_init_params['checkpoint']} on LeRobot Policy Server, please ensure that the checkpoint name or path is correct. Received the following error: {e}"
            )
            raise
        self.logger.info(
            f"{self.model_name} with {self.model_init_params['checkpoint']} model initialized"
        )

    def _inference(self, inference_input):
        """Send inference input to policy server"""

        # Create the object (using LeRobot spoofed class)
        inf_inp_obj = TimedObservation(
            timestamp=inference_input.pop("timestamp"),
            timestep=inference_input.pop("timestep"),
            observation=inference_input,
            must_go=True,  # Force server to process it
        )

        self.logger.debug(
            f"Sending to lerobot server on timestep: {inf_inp_obj.timestep}"
        )
        try:
            inf_inp_bytes = pickle.dumps(inf_inp_obj)

            # Generator for streaming
            def inf_gen():
                yield from self._send_bytes_in_chunks(
                    inf_inp_bytes, self.services_pb2.Observation
                )

            self.stub.SendObservations(inf_gen())
        except Exception as e:
            self.logger.error(str(e))
            return

    def _deinitialize(self):
        """Deinitialize the policy"""

        # TODO: Implement policy deinit when available on server api
        # for now we reset the server's internal state with Ready method
        self.logger.error(f"Deinitializing {self.model_name} model on LeRobot")
        try:
            self.stub.Ready(self.services_pb2.Empty())
        except Exception as e:
            self.logger.error(str(e))

    def _send_bytes_in_chunks(
        self, data_bytes, message_type, chunk_size=3 * 1024 * 1024
    ):
        """Splits large pickled bytes into gRPC messages."""
        total_len = len(data_bytes)
        offset = 0

        while True:
            end = min(offset + chunk_size, total_len)
            chunk = data_bytes[offset:end]

            # Check if this is the last chunk
            is_last = end == total_len

            # Determine the state
            if is_last:
                state = self.services_pb2.TransferState.TRANSFER_END
            elif offset == 0:
                state = self.services_pb2.TransferState.TRANSFER_BEGIN
            else:
                state = self.services_pb2.TransferState.TRANSFER_MIDDLE

            yield message_type(
                transfer_state=state,
                data=chunk,
            )

            offset += chunk_size

            # Stop if we have sent everything
            if offset >= total_len:
                break

    def receive_actions(self):
        """Receive actions from the server"""

        # HACK: Patch function: Load from bytes to cpu
        def safe_cpu_load_from_bytes(b):
            return self.torch_load(BytesIO(b), map_location="cpu")

        try:
            # Retrieve bytes
            actions_response = self.stub.GetActions(self.services_pb2.Empty())
            if not actions_response.data:
                return

            # HACK: Monkey patch torch.storge._load_from_bytes as pickle.dumps
            # on server side expects it. And torch will expect server's device
            # device to be present locally. We replace it with our own function

            # TODO: Remove torch dependency on the robot once policy server
            # can send numpy arrays

            # 1. Keep original function
            original_load_from_bytes = self.torch_storage._load_from_bytes

            try:
                # 2. Apply the Patch
                # Replace the internal function that pickle calls with our version
                self.torch_storage._load_from_bytes = safe_cpu_load_from_bytes

                # 3. Deserialize
                timed_actions = pickle.loads(actions_response.data)

            finally:
                # 4. Restore original function
                self.torch_storage._load_from_bytes = original_load_from_bytes

            return timed_actions

        except Exception as e:
            print(f"Receiver Error: {e}")
            time.sleep(0.3)  # Sleep for 300ms before trying again
