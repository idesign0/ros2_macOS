// Copyright 2020 Intelligent Robotics Lab
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RCLCPP_CASCADE_LIFECYCLE__RCLCPP_CASCADE_LIFECYCLE_HPP_
#define RCLCPP_CASCADE_LIFECYCLE__RCLCPP_CASCADE_LIFECYCLE_HPP_

#include <set>
#include <map>
#include <string>
#include <list>

#include "rclcpp/node_options.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/visibility_control.h"
#include "rclcpp_lifecycle/node_interfaces/lifecycle_node_interface.hpp"


#include "lifecycle_msgs/msg/state.hpp"
#include "cascade_lifecycle_msgs/msg/activation.hpp"
#include "cascade_lifecycle_msgs/msg/state.hpp"

#include "rclcpp/macros.hpp"

#include "rclcpp/rclcpp.hpp"

namespace rclcpp_cascade_lifecycle
{


class CascadeLifecycleNode : public rclcpp_lifecycle::LifecycleNode
{
public:
  RCLCPP_SMART_PTR_DEFINITIONS(CascadeLifecycleNode)
  /// Create a new lifecycle node with the specified name.
  /**
   * \param[in] node_name Name of the node.
   * \param[in] namespace_ Namespace of the node.
   * \param[in] options Additional options to control creation of the node.
   */
  RCLCPP_LIFECYCLE_PUBLIC
  explicit CascadeLifecycleNode(
    const std::string & node_name,
    const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

  /// Create a node based on the node name and a rclcpp::Context.
  /**
   * \param[in] node_name Name of the node.
   * \param[in] namespace_ Namespace of the node.
   * \param[in] options Additional options to control creation of the node.
   */
  RCLCPP_LIFECYCLE_PUBLIC
  CascadeLifecycleNode(
    const std::string & node_name,
    const std::string & namespace_,
    const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

  void add_activation(const std::string & node_name);
  void remove_activation(const std::string & node_name);
  void clear_activation();

  const std::set<std::string> & get_activators() const {return activators_;}
  const std::set<std::string> & get_activations() const {return activations_;}
  const std::map<std::string, uint8_t> & get_activators_state() const
  {
    return activators_state_;
  }

private:
  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
  on_configure_internal(const rclcpp_lifecycle::State & previous_state);

  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
  on_cleanup_internal(const rclcpp_lifecycle::State & previous_state);

  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
  on_shutdown_internal(const rclcpp_lifecycle::State & previous_state);

  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
  on_activate_internal(const rclcpp_lifecycle::State & previous_state);

  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
  on_deactivate_internal(const rclcpp_lifecycle::State & previous_state);

  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
  on_error_internal(const rclcpp_lifecycle::State & previous_state);

  rclcpp_lifecycle::LifecyclePublisher<cascade_lifecycle_msgs::msg::State>::SharedPtr states_pub_;
  rclcpp_lifecycle::LifecyclePublisher<cascade_lifecycle_msgs::msg::Activation>::SharedPtr
    activations_pub_;

  rclcpp::Subscription<cascade_lifecycle_msgs::msg::Activation>::SharedPtr activations_sub_;
  rclcpp::Subscription<cascade_lifecycle_msgs::msg::State>::SharedPtr states_sub_;

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::TimerBase::SharedPtr timer_responses_;

  std::set<std::string> activators_;
  std::set<std::string> activations_;
  std::map<std::string, uint8_t> activators_state_;
  std::list<cascade_lifecycle_msgs::msg::Activation> op_pending_;

  void activations_callback(const cascade_lifecycle_msgs::msg::Activation::SharedPtr msg);
  void states_callback(const cascade_lifecycle_msgs::msg::State::SharedPtr msg);
  void update_state();

  void timer_callback();
  void timer_responses_callback();

  bool allow_duplicate_names_ {true};
};

}  // namespace rclcpp_cascade_lifecycle

#endif  // RCLCPP_CASCADE_LIFECYCLE__RCLCPP_CASCADE_LIFECYCLE_HPP_
