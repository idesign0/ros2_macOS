<div align="center">

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/_static/SUGARCOAT_DARK.png">
  <source media="(prefers-color-scheme: light)" srcset="docs/_static/SUGARCOAT_LIGHT.png">
  <img alt="Sugarcoat Logo" src="docs/_static/SUGARCOAT_DARK.png" width="600">
</picture>

<br/>

Part of the [EMOS](https://github.com/automatika-robotics/emos) ecosystem

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Python](https://img.shields.io/badge/python-3.8+-blue.svg)](https://www.python.org/downloads/)
[![ROS2](https://img.shields.io/badge/ROS2-Humble%2B-green)](https://docs.ros.org/en/humble/index.html)
[![Discord](https://img.shields.io/badge/Discord-%235865F2.svg?logo=discord&logoColor=white)](https://discord.gg/B9ZU6qjzND)

**The orchestration layer for event-driven ROS 2 systems**

[**EMOS Documentation**](https://emos.automatikarobotics.com) | [**Developer Docs**](https://sugarcoat.automatikarobotics.com) | [**Discord**](https://discord.gg/B9ZU6qjzND)

</div>

---

## What is Sugarcoat?

**Sugarcoat** is the orchestration layer of the [EMOS](https://github.com/automatika-robotics/emos) (Embodied Operating System) ecosystem by [Automatika Robotics](https://automatikarobotics.com/). It is a meta-framework that replaces fragmented ROS2 development with a unified workflow, providing a high-level Python API to build robust lifecycle-managed components and orchestrate them into cohesive, self-healing systems using an event-driven architecture.

For full documentation, tutorials, and recipes, visit [emos.automatikarobotics.com](https://emos.automatikarobotics.com).

---

## Key Features & Core Pillars

| Feature                       | Description                                                                                                                                                                                                                    |
| :---------------------------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Smart Components**          | Every component is a managed lifecycle node (Configure, Activate, Deactivate) out of the box. It features type-safe configurations via `attrs` and declarative auto-wiring for inputs/outputs.                                 |
| **Active Resilience**         | <br> Built-in "Immune System" for ROS2 nodes. Components actively report their **Health Status** (Algorithm, Component, or System failures) and automatically trigger distributed **Fallbacks** to self-heal without crashing. |
| **Event-Driven Behavior**     | Define global **Events** (e.g., `Event(battery < 10.0)`) and **Actions** in pure, readable Python. These act as triggers that monitor ROS2 topics natively and execute instantly regardless of current system state.           |
| **Centralized Orchestration** | A powerful **Launcher** acts as a pythonic alternative to `ros2 launch`. It supports multi-threaded or multi-process execution, actively supervising component lifecycles at runtime.                                          |
| **Universal Applications**    | **Robot Plugins** act as a translation layer. This allows you to write generic, portable automation logic (recipes) that run on any robot without code changes.                                                                |
| **Dynamic Web UI**            | Auto-generates a fully functional web frontend for every topic, parameter, and event instantly.                                                                                                                                |


## Dynamic Web UI for Sugarcoat Recipes

The **Dynamic Web UI** feature takes system visibility and control to the next level. Built with **[FastHTML](https://www.fastht.ml/)** and **[MonsterUI](https://monsterui.answer.ai/)**, it is designed to automatically generate a fully dynamic, extensible web interface for any Sugarcoat recipe, completely eliminating the need for manual front-end development.

### Automatic UI Generation in Action

See how the Web UI effortlessly generates interfaces for different types of Sugarcoat recipes:

- **Example 1: General Q&A MLLM Recipe**
  A fully functional interface generated for an MLLM agent recipe from **[EmbodiedAgents](https://github.com/automatika-robotics/embodied-agents)**, automatically providing controls for settings and real-time text I/O with the robot.

<p align="center">
<picture align="center">
<img alt="EmbodiedAgents UI Example GIF" src="./docs/_static/videos/ui_agents.gif" width="60%">
</picture>
</p>

- **Example 2: Point Navigation Recipe**
  An example for an automatically generated UI for a point navigation system from **[Kompass](https://github.com/automatika-robotics/kompass)**. The UI automatically renders map data, and sends navigation goals to the robot.

<p align="center">
<picture align="center">
<img alt="Navigation System UI Example GIF" src="./docs/_static/videos/ui_navigation.gif" width="60%">
</picture>
</p>

### What's Inside?

- **Automatic Settings UI**: Interfaces for configuring the settings of all **Components** used in your recipe are generated on the fly.
- **Auto I/O Visualization**: Front-end controls and data visualizations for UI **Inputs** and **Outputs** are created automatically.
- **WebSocket-Based Streaming**: Features bidirectional, low-latency communication for streaming **text, image, and audio** messages.
- **Responsive Layouts**: Input and output elements are presented in clear, adaptable grid layouts.
- **Extensible Design**: Easily add support for new message types and custom visualizations through extensions.

## Documentation

| Resource | URL |
|:---------|:----|
| **Usage Docs (EMOS)** | [emos.automatikarobotics.com](https://emos.automatikarobotics.com/) |
| **Developer Docs** | [sugarcoat.automatikarobotics.com](https://sugarcoat.automatikarobotics.com/) |
| **API Reference** | [sugarcoat.automatikarobotics.com/apidocs](https://sugarcoat.automatikarobotics.com/apidocs/index.html) |

## Installation

Sugarcoat is available for ROS versions **Humble**.

### Using your Package Manager (Recommended)

On Ubuntu, for example:

`sudo apt install ros-$ROS_DISTRO-automatika-ros-sugar`

### Building from Source

```bash
mkdir -p ros-sugar-ws/src
cd ros-sugar-ws/src
git clone https://github.com/automatika-robotics/sugarcoat && cd ..

# Install dependencies (ensure attrs>=23.2.0 is included)
pip install numpy opencv-python-headless 'attrs>=23.2.0' jinja2 msgpack msgpack-numpy setproctitle pyyaml toml

colcon build
source install/setup.bash
```

## Development

### Running Tests

```bash
# Full test suite
colcon test --packages-select automatika_ros_sugar
colcon test-result --verbose

# Individual tests with pytest
python -m pytest tests/ -v
```

### Developer Docs

For contributors and developers extending Sugarcoat:

- [Architecture Overview](https://sugarcoat.automatikarobotics.com/development/architecture.html) -- Core module structure, component lifecycle, IO system, and process graph.
- [Extending the Type System](https://sugarcoat.automatikarobotics.com/development/custom_types.html) -- How to add custom `SupportedType` subclasses and register them.
- [Event & Action System Internals](https://sugarcoat.automatikarobotics.com/development/event_system.html) -- Condition trees, action dispatch, and the fallback system.
- [Testing Guide](https://sugarcoat.automatikarobotics.com/development/testing.html) -- Unit testing, integration testing with `launch_testing`, and running the test suite.

## Contributing

Sugarcoat has been developed in collaboration between [Automatika Robotics](https://automatikarobotics.com/) and [Inria](https://inria.fr/). Contributions from the community are most welcome.

Please open an issue or pull request on [GitHub](https://github.com/automatika-robotics/sugarcoat).

## Hat Tip

The **Dynamic Web UI** is powered by two awesome open-source projects. A big thank you to Answers.ai for their work on:

- **[FastHTML](https://www.fastht.ml/)**: The HTMX based framework that enables automatic generation of our dynamic web interfaces.
- **[MonsterUI](https://monsterui.answer.ai/)**: The styled UI components that make the interface intuitive.

## License

**Sugarcoat** is a collaboration between [Automatika Robotics](https://automatikarobotics.com/) and [Inria](https://inria.fr/).

The code is available under the **MIT License**. See [LICENSE](LICENSE) for details.
Copyright (c) 2024 Automatika Robotics unless explicitly indicated otherwise.
