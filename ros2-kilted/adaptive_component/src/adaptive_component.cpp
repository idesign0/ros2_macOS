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

AdaptiveComponent is a ROS 2 Node subclass programmed as a "Component" to build
adaptive computations. Adaptive ROS 2 Nodes are able to perform computations in
the CPU, the FPGA or the GPU, adaptively. Adaptive behavior is controlled
through the "adaptive" ROS 2 parameter.

Adaptive ROS 2 Nodes give roboticists a finer-grained control over the
resources their computional graphs use in the underlying hardware.
*/

#include <chrono>  // NOLINT
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp/clock.hpp"
#include "adaptive_component.hpp"

using namespace std::chrono_literals;  // NOLINT

namespace composition
{

// std::shared_ptr<std::thread> AdaptiveComponent::hilo_;

void
AdaptiveComponent::initialize()
{
  const char * param_name = "adaptive";

  // Declare "adaptive" param from "adaptive_value_" and add Node
  // (defaults to CPU, unless indicated differently in the constructor)
  this->declare_parameter<int>(param_name, adaptive_value_);
  if (compute_resources_[adaptive_value_])
    exec_.add_node(compute_resources_[adaptive_value_]);

  // // Spin internal executor in a detached thread kept internally for synchronization
  // hilo_ = std::make_shared<std::thread>(std::bind(&AdaptiveComponent::spin, this));   // spawn thread
  // hilo_->detach();

  // Spin internal executor in a detached thread
  std::thread (&AdaptiveComponent::spin, this).detach();  // a more synthetic version, but locally


  // Parameter callback to adapt computational behavior
  // NOTE: not available in Foxy
  //
  // TODO: re-evaluate in Rolling or Humble, see
  // https://github.com/ros2/demos/blob/master/demo_nodes_cpp/src/parameters/parameter_event_handler.cpp#L83-L91
  //
  // // Now, create a parameter subscriber that can be used to monitor parameter changes on
  // // our own local node as well as other remote nodes
  // auto param_subscriber = std::make_shared<rclcpp::ParameterEventHandler>(this);
  //
  // // First, set a callback for the local integer parameter. In this case, we don't
  // // provide a node name (the third, optional, parameter).
  // auto cb1 = [this](const rclcpp::Parameter & p) {
  //     RCLCPP_INFO(
  //       this->get_logger(),
  //       "cb1: Received an update to parameter \"%s\" of type %s: \"%" PRId64 "\"",
  //       p.get_name().c_str(),
  //       p.get_type_name().c_str(),
  //       p.as_int());
  //   };
  // auto handle1 = param_subscriber->add_parameter_callback(param_name, cb1);

  // Use a timer instead to adapt computational behavior
  // TODO: replace this by the parameter callback
  timer_ = create_wall_timer(1s, std::bind(&AdaptiveComponent::on_timer, this));
}

void
AdaptiveComponent::spin(void)
{
  exec_.spin();
}

void
AdaptiveComponent::on_timer(void)
{
  try{
    int new_adaptive_value_;
    this->get_parameter("adaptive", new_adaptive_value_);

    if (new_adaptive_value_ != adaptive_value_) {  // if there's change

      // Remove nodes
      if (adaptive_value_ == Hardware::FPGA) {
        if (fpga_node_)
          exec_.remove_node(fpga_node_);
      } else if (adaptive_value_ == Hardware::CPU) {
        if (cpu_node_)
          exec_.remove_node(cpu_node_);
      } else if (adaptive_value_ == Hardware::GPU) {
        if (gpu_node_)
          exec_.remove_node(gpu_node_);
      }

      // Add new ones
      if (new_adaptive_value_ == Hardware::FPGA) {
        if (fpga_node_)
          exec_.add_node(fpga_node_);
        else
          RCLCPP_ERROR(this->get_logger(), "No FPGA Node available for computations.");
      } else if (new_adaptive_value_ == Hardware::CPU) {
        if (cpu_node_)
          exec_.add_node(cpu_node_);
        else
          RCLCPP_ERROR(this->get_logger(), "No CPU Node available for computations.");
      } else if (new_adaptive_value_ == Hardware::GPU) {
        if (gpu_node_)
          exec_.add_node(gpu_node_);
        else
          RCLCPP_ERROR(this->get_logger(), "No GPU Node available for computations.");

      } else {
        RCLCPP_ERROR(
          this->get_logger(),
          "Invalid new 'adaptive' parameter value: %d", new_adaptive_value_);
      }

      // // TODO: More synthetic implementation
      // //    re-engage if compute options start exploding,
      // //    needs to properly consider bounds and avoid nullptr issues.
      // //
      //
      // // Remove Nodes in current hardware
      // if (compute_resources_[adaptive_value_])
      //   exec_.remove_node(compute_resources_[adaptive_value_]);
      //
      // // Add new nodes
      // if (compute_resources_[new_adaptive_value_])
      //   exec_.add_node(compute_resources_[new_adaptive_value_]);
      // else
      //   RCLCPP_ERROR(this->get_logger(),
      //     "No Node available for computational resource: %d (%s)",
      //     new_adaptive_value_,
      //     compute_resources_names_[new_adaptive_value_]
      //   );

      adaptive_value_ = new_adaptive_value_;
    }
  } catch (const rclcpp::exceptions::InvalidParameterTypeException& ex) {
    RCLCPP_ERROR(
      this->get_logger(),
      "Invalid parameter value provided: %s", ex.what());
  }

}  // on_timer

}  // namespace composition

#include "rclcpp_components/register_node_macro.hpp"

// Register the component with class_loader.
// This acts as a sort of entry point, allowing the component to be discoverable when its library
// is being loaded into a running process.
RCLCPP_COMPONENTS_REGISTER_NODE(composition::AdaptiveComponent)
