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

#ifndef CX_PLUGIN__CLIPS_PLUGIN_HPP_
#define CX_PLUGIN__CLIPS_PLUGIN_HPP_

#include <map>
#include <memory>
#include <string>

#include "clips_ns/clips.h"
#include "pluginlib/class_loader.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"

namespace cx
{

/// Clips Plugin
/**
 * This base class provides the interface for clips plugins that are managed
 * by the ClipsPluginManager.
 *
 * Inherit from this class and export your class as plugin via pluginlib.
 */
class ClipsPlugin
{
  friend class ClipsPluginManager;
  using Ptr = pluginlib::UniquePtr<cx::ClipsPlugin>;

public:
  ClipsPlugin();
  virtual ~ClipsPlugin();

  /// Called once for each plugin when it is loaded.
  virtual void initialize();
  /// Called once for each plugin when it is unloaded.
  virtual void finalize();

  /// Called after initialize once for every managed Clips environment.
  /**
   * Register your user-defined functions here and store the references
   * of the environment for usage.
   * The clips environment should not be locked in this function as the plugin
   * manager takes care of that. Only lock the Clips environment when accessing
   * it from separate threads, such as callbacks.
   *
   * \param[in] env a pointer to a Clips environment including a mutex to lock
   * before usage in other threads.
   *
   * \return true iff the initialization succeeded
   */
  virtual bool clips_env_init(std::shared_ptr<clips::Environment> & env) = 0;

  /// Called once for every managed Clips environment on shutting down the
  /// environment.
  /**
   * \param[in] env a pointer to a Clips environment including a mutex to lock
   * before usage in other threads.
   * \return true iff the initialization succeeded
   */
  virtual bool clips_env_destroyed(std::shared_ptr<clips::Environment> & env) = 0;

  std::string get_plugin_name() const;

protected:
  /// Configured name of the plugin
  std::string plugin_name_;

  /// Reference to parent node in case ROS interaction is needed
  rclcpp_lifecycle::LifecycleNode::WeakPtr parent_;

private:
  /// \internal pass parent and name to the instance.
  void initialize(
    const rclcpp_lifecycle::LifecycleNode::WeakPtr parent, const std::string & plugin_name);
};

}  // namespace cx

#endif  // CX_PLUGIN__CLIPS_PLUGIN_HPP_
