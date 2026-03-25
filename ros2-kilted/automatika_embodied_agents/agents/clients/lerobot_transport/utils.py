import sys
import types
from typing import Any, Dict
from dataclasses import dataclass, field


# HACK: We define helper classes here, but set their __module__ to match
# what the server expects. This tricks pickle.


@dataclass
class RemotePolicyConfig:
    policy_type: str
    pretrained_name_or_path: str
    lerobot_features: Dict
    actions_per_chunk: int
    device: str = "cpu"
    rename_map: Dict = field(default_factory=dict)


# Tell pickle this class actually belongs to 'lerobot.async_inference.helpers'
RemotePolicyConfig.__module__ = "lerobot.async_inference.helpers"


@dataclass
class TimedObservation:
    timestamp: float
    observation: Any
    timestep: int
    must_go: bool = False


# Tell pickle this class actually belongs to 'lerobot.async_inference.helpers'
TimedObservation.__module__ = "lerobot.async_inference.helpers"


@dataclass
class TimedAction:
    timestamp: float
    timestep: int
    action: Any


# Tell pickle this class actually belongs to 'lerobot.async_inference.helpers'
TimedAction.__module__ = "lerobot.async_inference.helpers"

# Create module chain for pickle
mod_helpers = types.ModuleType("lerobot.async_inference.helpers")
mod_helpers.RemotePolicyConfig = RemotePolicyConfig
mod_helpers.TimedObservation = TimedObservation
mod_helpers.TimedAction = TimedAction

# Add modules to sys path
sys.modules["lerobot"] = types.ModuleType("lerobot")
sys.modules["lerobot.async_inference"] = types.ModuleType("lerobot.async_inference")
sys.modules["lerobot.async_inference.helpers"] = mod_helpers
