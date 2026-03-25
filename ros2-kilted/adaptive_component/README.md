# `adaptive_component`

**A composable stateless container for Adaptive ROS 2 Node computations. Select between FPGA, CPU or GPU at run-time**.

Nodes using hardware acceleration are able to perform computations faster relying on FPGAs or GPUs, improving performance. <ins>Adaptive ROS 2 Nodes</ins> leverage hardware acceleration at run-time, allowing robotics engineers to <ins>select which computational resource the Node uses *on-the-go*</ins>, giving roboticists a finer-grained control over the resources their computional graphs use in the underlying hardware.

This ROS 2 package provides a composable stateless container for Adaptive ROS 2 Node computations: [`adaptive_component`](https://github.com/ros-acceleration/adaptive_component). It allows building Nodes that can select between FPGA, CPU or GPU, at run-time.

Technically, it's a ROS 2 `Node`[^1] subclass programmed as a `Component`[^2] and including its own [single threaded executor](https://github.com/ros-acceleration/adaptive_component/blob/main/include/adaptive_component.hpp#L68)
to build adaptive computations. Adaptive ROS 2 `Nodes` can then be built easily and are able to perform computations in the CPU, the FPGA or the GPU. Adaptive behavior is controlled through the `adaptive` ROS 2 parameter, with the following values allowed:

- `0`: Hardware::CPU
- `1`: Hardware::FPGA
- `2`: Hardware::GPU

`adaptive_component` is *stateless* by default, if you need your Adaptive Nodes to be *stateful*, feel free to inherit from `composition::AdaptiveComponent` and create your own stateful subclasses[^5].

[^1]: A `Node` is a process that performs computations. ROS 2 is designed to be modular at a fine-grained scale; a robot control system usually comprises many nodes. Nodes execute arbitrary logic that contribute to the overall robotics behavior.

[^2]: A ROS 2 `Component` is a `Node` compiled into a shared library which is then loaded at runtime by a container process. This offers roboticists additional flexibility while building their computational graphs, making the layout process a deploy-time decision. A `Component` is commonly a subclass of `rclcpp::Node`. To maintain flexibility and modularity, `Components` shouldn’t perform any long running or blocking tasks in their constructors. Instead, they can use timers to get periodic notifications and use callbacks for publishers, subscribers, servers, or clients.

[^5]: See [this example](https://github.com/ros-acceleration/acceleration_examples/blob/main/nodes/doublevadd_publisher/src/doublevadd_publisher_adaptive_with_components_and_state.cpp) ([ROS 2 component](https://github.com/ros-acceleration/acceleration_examples/blob/main/nodes/doublevadd_publisher/src/doublevadd_component_adaptive_stateful.cpp)).


## How does it work?

[![asciicast](https://asciinema.org/a/448016.svg)](https://asciinema.org/a/448016)

```cpp
using NodeCPU = composition::DoubleVaddComponent;
using NodeFPGA = composition::DoubleVaddComponentFPGA;

rclcpp::NodeOptions options;

// Create an executor
rclcpp::executors::MultiThreadedExecutor exec;

// Create an adaptive ROS 2 Node using "components", the resulting
// Node is also programed as a "component", retaining composability
auto adaptive_node = std::make_shared<composition::AdaptiveComponent>(
      "doublevadd_publisher_adaptive",        
      options,                                
                                              // CPU
      std::make_shared<NodeCPU>("_doublevadd_publisher_adaptive_cpu", options),
                                              // FPGA
      std::make_shared<NodeFPGA>("_doublevadd_publisher_adaptive_fpga", options),
                                              // GPU
      nullptr);

exec.add_node(adaptive_node);  // fill up the executor
exec.spin();  // spin the executor
```

Then, dynamically, one could switch from CPU to FPGA by setting the `adaptive` parameter in the `/doublevadd_publisher_adaptive` Node:
- To run in the CPU: `ros2 param set /doublevadd_publisher_adaptive adaptive 0`
- To run in the FPGA: `ros2 param set /doublevadd_publisher_adaptive adaptive 1`


## Why should I care as a ROS package maintainer?

The integration of hardware acceleration into ROS often requires rewriting parts of the Node computations to further exploit parallelism. These changes often conflict with CPU-centric architectures and as a maintainer, you're likely to care for "not breaking" CPU-centric implementations.

To consistently integrate hardware acceleration, <ins>**avoid unnecessary forks** and discourage package fragmentation</ins>, `composition::AdaptiveComponent` allows to extend ROS 2 CPU-centric Nodes[^3] with their computational counterparts separating concerns at build-time. From a package-maintenance perspective, each Node (across computation options) is written in a separated file and as a separated Component. These can live either within the same package, or in totally different (disconnected) ones. [`adaptive_component`](https://github.com/ros-acceleration/adaptive_component) takes care of putting them together at launch time and no dependency with the package is required at build-time[^4].

From an execution perspective, developers can easily create Adaptive ROS 2 Nodes and compose them together as desired at launch-time, with capabilities to adaptively switch between compute alternatives at run-time.

[^3]: Assumes Nodes are written as components, but it they are not, it's a great chance to do so ;).

[^4]: Though `adaptive_component` allows to disconnect nodes across packages, to facilitate source code maintenance across CPU-centric implementations and counterparts, <ins>it should be encouraged to keep the source code within the same ROS 2 package with suffixes indicating of the compute substrate (e.g. `_fpga.cpp`, etc.)</ins>. This will facilitate maintaining implementations across different compute substrates, avoid versioning issues and fragmentation issues.


## Some examples

Examples of using `adaptive_component`:

- [An Adaptive ROS 2 Node](https://github.com/ros-acceleration/acceleration_examples/blob/main/nodes/doublevadd_publisher/src/doublevadd_publisher_adaptive.cpp)
- [An Adaptive stateless ROS 2 Component](https://github.com/ros-acceleration/acceleration_examples/blob/main/nodes/doublevadd_publisher/src/doublevadd_component_adaptive.cpp) ([Node example using it](https://github.com/ros-acceleration/acceleration_examples/blob/main/nodes/doublevadd_publisher/src/doublevadd_publisher_adaptive_with_components.cpp))
- [An Adaptive stateful ROS 2 Component](https://github.com/ros-acceleration/acceleration_examples/blob/main/nodes/doublevadd_publisher/src/doublevadd_component_adaptive_stateful.cpp)  ([Node example using it](https://github.com/ros-acceleration/acceleration_examples/blob/main/nodes/doublevadd_publisher/src/doublevadd_publisher_adaptive_with_components_and_state.cpp))


## Conventions and recommendations

The following conventions and recommendations are meant to facilitate the integration of hardware acceleration in existing ROS packages

1. **Component-oriented**: [`AdaptiveComponent`](https://github.com/ros-acceleration/adaptive_component/blob/main/src/adaptive_component.cpp) is built as a component and should be used as such to maintain composability of Nodes.
2. **Naming**: The Adaptive Node should be suffix with `_adaptive` to identify in the computational graph which Nodes have adaptive capabilities and which do not.
3. **Hidden sub-Nodes**: Adaptive Node components (compute-specific ones e.g. CPU's or FPGA's) should be named with a hyphen (`_`) as a prefix, which will make them *hidden* Nodes by default.
4. **File names**: When possible, source code file names should adhere to the following guidelines:
   - CPU-based computational Nodes can <ins>optionally</ins> add the `_cpu` suffix
   - FPGA-based computational Nodes shall add the `_fpga` suffix
   - GPU-based computational Nodes shall add the `_gpu` suffix

## Quality Declaration

This package claims to be in the **Quality Level 4** category, see the [Quality Declaration](./QUALITY_DECLARATION.md) for more details.
