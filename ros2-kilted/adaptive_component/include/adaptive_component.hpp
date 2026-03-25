/*
      ____  ____
     /   /\/   /
    /___/  \  /   Copyright (c) 2021, Xilinx®.
    \   \   \/    Author: Víctor Mayoral Vilches <victorma@xilinx.com>
     \   \
     /   /
    /___/   /\
    \   \  /  \
     \___\/\___\

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef ADAPTIVE_COMPONENT_HPP_
#define ADAPTIVE_COMPONENT_HPP_

#include "rclcpp/rclcpp.hpp"
#include "rclcpp/executor.hpp"
#include "rclcpp/visibility_control.hpp"

namespace composition
{

/// A composable container for Adaptive ROS 2 Node computations.
/**
 * Select between FPGA, CPU or GPU at run-time.
 *
 * Nodes using hardware acceleration are able to perform computations faster
 * relying on FPGAs or GPUs, improving performance. Adaptive ROS 2 Nodes
 * leverage hardware acceleration at run-time, allowing to select which
 * computational resource the Node uses on-the-go, providing with a
 * finer-grained control over the resources the computional graphs use in
 * the underlying hardware.
 *
 * AdaptiveComponent is a ROS 2 Node subclass programmed as a Component and
 * including its own single threaded executor to build adaptive computations.
 * Adaptive ROS 2 Nodes can then be built easily with it. Adaptive behavior
 * is controlled through the "adaptive" ROS 2 parameter.
 */
class AdaptiveComponent : public rclcpp::Node
{
public:
  // Define within the class to avoid polluting the global namespace
  enum Hardware {
    CPU  = 0,
    FPGA = 1,
    GPU  = 2,
  };

  /// Constructor.
  /**
   * \param[in] options Options used to configure the executor.
   */
  RCLCPP_PUBLIC
  AdaptiveComponent(const rclcpp::NodeOptions & options)
      : Node("adaptive_component", options) {};

  /// Constructor.
  /**
   * \param[in] options Options used to configure the executor.
   * \param[in] cpu_node std::shared_ptr<rclcpp::Node> Node for CPU computations.
   * \param[in] fpga_node std::shared_ptr<rclcpp::Node> Node for FPGA computations.
   * \param[in] fpga_node std::shared_ptr<rclcpp::Node> Node for GPU computations.
   * \param[in] adaptive_value int initial value for "adaptive" ROS parameter.
   * Defines what's the initial compute substrate that's used.
   */
  RCLCPP_PUBLIC
  AdaptiveComponent(
    const rclcpp::NodeOptions & options,
    std::shared_ptr<rclcpp::Node> cpu_node,
    std::shared_ptr<rclcpp::Node> fpga_node,
    std::shared_ptr<rclcpp::Node> gpu_node,
    const int adaptive_value
  )
      : Node("adaptive_component", options), cpu_node_(cpu_node),
        fpga_node_(fpga_node), gpu_node_(gpu_node),
        adaptive_value_(adaptive_value)
  {
    initialize();
  }

  /// Default constructor.
  /**
   * \param[in] node_name std::string Adaptive Node name.
   * \param[in] options Options used to configure the executor.
   * \param[in] cpu_node std::shared_ptr<rclcpp::Node> Node for CPU computations.
   * \param[in] fpga_node std::shared_ptr<rclcpp::Node> Node for FPGA computations.
   * \param[in] fpga_node std::shared_ptr<rclcpp::Node> Node for GPU computations.
   * \param[in] adaptive_value int initial value for "adaptive" ROS parameter and
   * defines what's the initial compute substrate that's used.
   */
  RCLCPP_PUBLIC
  AdaptiveComponent(
    const std::string &node_name,
    const rclcpp::NodeOptions & options,
    std::shared_ptr<rclcpp::Node> cpu_node = nullptr,
    std::shared_ptr<rclcpp::Node> fpga_node = nullptr,
    std::shared_ptr<rclcpp::Node> gpu_node = nullptr,
    const int adaptive_value = 0
  )
      : Node(node_name, options), cpu_node_(cpu_node),
        fpga_node_(fpga_node), gpu_node_(gpu_node),
        adaptive_value_(adaptive_value),
        compute_resources_{cpu_node_, fpga_node_, gpu_node_}
  {
    initialize();
  }

  /// Add CPU Node manually
  RCLCPP_PUBLIC
  void add_cpu(std::shared_ptr<rclcpp::Node> node_ptr) {cpu_node_ = node_ptr;};

  /// Add FPGA Node manually
  RCLCPP_PUBLIC
  void add_fpga(std::shared_ptr<rclcpp::Node> node_ptr) {fpga_node_ = node_ptr;};

  /// Add GPU Node manually
  RCLCPP_PUBLIC
  void add_gpu(std::shared_ptr<rclcpp::Node> node_ptr) {gpu_node_ = node_ptr;};

protected:
  /// Timer callback at 1 second, used to update "adaptive" parameter
  // and reconfigure computations appropriately.
  RCLCPP_PUBLIC
  void on_timer(void);

  /// Spin internal single threaded executor.
  // Actioned in a detached thread
  RCLCPP_PUBLIC
  void spin(void);

  // /// Internal thread to spin executor
  // std::shared_ptr<std::thread> hilo_;

  /// Internal AdaptiveComponent container's executor
  rclcpp::executors::SingleThreadedExecutor exec_;

  /// Computational Node for the CPU
  // Should be nullptr by default, C++11 spec.
  std::shared_ptr<rclcpp::Node> cpu_node_;

  /// Computational Node for the FPGA
  std::shared_ptr<rclcpp::Node> fpga_node_;

  /// Computational Node for the GPU
  std::shared_ptr<rclcpp::Node> gpu_node_;

  /// AdaptiveCoponent internal timer
  rclcpp::TimerBase::SharedPtr timer_;

  /// Mirrors the Node's "adaptive" ROS 2 parameter
  int adaptive_value_;

  /// Pointers to nodes, simplify interactions
  std::shared_ptr<rclcpp::Node> compute_resources_[3];

  // TODO: reconsider if a more synthetic implementation is used (see cpp)
  // std::string compute_resources_names_[3] = {"CPU", "FPGA", "GPU"};

private:
  /// Initialize AdaptiveComponent, creates callbacks and threads
  RCLCPP_PUBLIC
  void initialize(void);

};

}  // namespace composition

#endif  // ADAPTIVE_COMPONENT_HPP_
