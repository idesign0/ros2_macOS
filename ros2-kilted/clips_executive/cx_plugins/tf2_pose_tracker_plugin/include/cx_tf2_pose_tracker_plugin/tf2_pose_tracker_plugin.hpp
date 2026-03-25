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

#ifndef CX_TF2_POSE_TRACKER_PLUGIN__TF2_POSE_TRACKER_PLUGIN_HPP_
#define CX_TF2_POSE_TRACKER_PLUGIN__TF2_POSE_TRACKER_PLUGIN_HPP_

#include <memory>
#include <string>
#include <vector>

#include "cx_plugin/clips_plugin.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"

namespace cx
{

class Tf2PoseTrackerPlugin : public ClipsPlugin
{
public:
  Tf2PoseTrackerPlugin();
  ~Tf2PoseTrackerPlugin();

  void initialize() override;
  void finalize() override;

  bool clips_env_init(std::shared_ptr<clips::Environment> & env) override;
  bool clips_env_destroyed(std::shared_ptr<clips::Environment> & env) override;

private:
  struct PoseTracker
  {
    rclcpp::TimerBase::SharedPtr timer;
    clips::Fact * pose_fact;
    clips::Environment * env;
  };

  std::unique_ptr<rclcpp::Logger> logger_;

  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::unique_ptr<tf2_ros::TransformListener> tf_listener_;

  rclcpp::CallbackGroup::SharedPtr cb_group_;

  std::vector<std::shared_ptr<PoseTracker>> pose_trackers_;

  void start_periodic_lookup(
    clips::Environment * env, const std::string & parent, const std::string & child,
    double frequency);
  bool stop_periodic_lookup(PoseTracker * pose_tracker);
};
}  // namespace cx

#endif  // CX_TF2_POSE_TRACKER_PLUGIN__TF2_POSE_TRACKER_PLUGIN_HPP_
