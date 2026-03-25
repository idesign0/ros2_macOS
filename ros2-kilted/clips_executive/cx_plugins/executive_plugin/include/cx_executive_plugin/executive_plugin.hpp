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

#ifndef CX_EXECUTIVE_PLUGIN__EXECUTIVE_PLUGIN_HPP_
#define CX_EXECUTIVE_PLUGIN__EXECUTIVE_PLUGIN_HPP_

#include <memory>
#include <string>
#include <vector>

#include "cx_plugin/clips_plugin.hpp"
#include "std_msgs/msg/empty.hpp"

namespace cx
{

class ExecutivePlugin : public ClipsPlugin
{
public:
  ExecutivePlugin();
  ~ExecutivePlugin();

  void initialize() override;
  void finalize() override;

  bool clips_env_init(std::shared_ptr<clips::Environment> & clips) override;
  bool clips_env_destroyed(std::shared_ptr<clips::Environment> & clips) override;

  std::shared_ptr<rclcpp_lifecycle::LifecycleNode> get_node();

private:
  int refresh_rate_;
  bool assert_time_;
  bool publish_on_refresh_;

  std::string plugin_path_;

  rclcpp::TimerBase::SharedPtr agenda_refresh_timer_;

  rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::Empty>::SharedPtr clips_agenda_refresh_pub_;

  std::chrono::nanoseconds publish_rate_;

  std::unique_ptr<rclcpp::Logger> logger_;

  std::vector<std::shared_ptr<clips::Environment>> managed_envs;
  std::mutex envs_mutex_;
};
}  // namespace cx

#endif  // CX_EXECUTIVE_PLUGIN__EXECUTIVE_PLUGIN_HPP_
