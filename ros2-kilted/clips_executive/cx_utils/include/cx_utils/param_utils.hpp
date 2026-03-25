// Copyright (c) 2024-2026 Carologistics
// SPDX-License-Identifier: Apache-2.0
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

#ifndef CX_UTILS__PARAM_UTILS_HPP_
#define CX_UTILS__PARAM_UTILS_HPP_

#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
namespace cx
{
namespace cx_utils
{
/// Declares static ROS2 parameter and sets it to a given value if it was not
/// already declared
/* Declares static ROS2 parameter and sets it to a given value
 * if it was not already declared.
 *
 * \param[in] node A node in which given parameter to be declared
 * \param[in] param_name The name of parameter
 * \param[in] default_value Parameter value to initialize with
 * \param[in] parameter_descriptor Parameter descriptor (optional)
 */
template <typename NodeT>
void declare_parameter_if_not_declared(
  NodeT node, const std::string & param_name, const rclcpp::ParameterValue & default_value,
  const rcl_interfaces::msg::ParameterDescriptor & parameter_descriptor =
    rcl_interfaces::msg::ParameterDescriptor())
{
  if (!node->has_parameter(param_name)) {
    RCLCPP_DEBUG(rclcpp::get_logger("cx_utils"), "Creating parameter: %s", param_name.c_str());
    node->declare_parameter(param_name, default_value, parameter_descriptor);
  }
}

/// Declares ROS2 parameter and sets it to a given value
/* Declares static ROS2 parameter with given type if it was not already
 * declared.
 *
 * \param[in] node A node in which given parameter to be declared
 * \param[in] param_type The type of parameter
 * \param[in] default_value Parameter value to initialize with
 * \param[in] parameter_descriptor Parameter descriptor (optional)
 */
template <typename NodeT>
void declare_parameter_if_not_declared(
  NodeT node, const std::string & param_name, const rclcpp::ParameterType & param_type,
  const rcl_interfaces::msg::ParameterDescriptor & parameter_descriptor =
    rcl_interfaces::msg::ParameterDescriptor())
{
  if (!node->has_parameter(param_name)) {
    RCLCPP_DEBUG(rclcpp::get_logger("cx_utils"), "Creating parameter: %s", param_name.c_str());
    node->declare_parameter(param_name, param_type, parameter_descriptor);
  }
}

/// Gets the type of plugin for the selected node and its plugin
/**
 * Gets the type of plugin for the selected node and its plugin.
 * Actually seeks for the value of "<plugin_name>.plugin" parameter.
 *
 * \param[in] node Selected node
 * \param[in] plugin_name The name of plugin the type of which is being searched
 * for \return A string containing the type of plugin (the value of
 * "<plugin_name>.plugin" parameter)
 */
template <typename NodeT>
std::string get_plugin_type_param(NodeT node, const std::string & plugin_name)
{
  declare_parameter_if_not_declared(node, plugin_name + ".plugin", rclcpp::PARAMETER_STRING);
  std::string plugin_type;
  try {
    if (!node->get_parameter(plugin_name + ".plugin", plugin_type)) {
      RCLCPP_FATAL(node->get_logger(), "'plugin' param not defined for %s", plugin_name.c_str());
      exit(-1);
    }
  } catch (rclcpp::exceptions::ParameterUninitializedException & ex) {
    RCLCPP_FATAL(
      node->get_logger(), "'plugin' param not defined for %s. Error: %s", plugin_name.c_str(),
      ex.what());
    exit(-1);
  }

  return plugin_type;
}
void resolve_files(
  const std::vector<std::string> & files_in, const std::vector<std::string> & share_dirs,
  std::vector<std::string> & files_out);
}  // namespace cx_utils
}  // namespace cx

#endif  // CX_UTILS__PARAM_UTILS_HPP_
