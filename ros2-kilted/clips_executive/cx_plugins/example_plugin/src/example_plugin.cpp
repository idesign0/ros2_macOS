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

#include "cx_example_plugin/example_plugin.hpp"

#include <string>

#include "cx_utils/clips_env_context.hpp"

// To export as plugin
#include "pluginlib/class_list_macros.hpp"

using std::placeholders::_1;
namespace cx
{
ExamplePlugin::ExamplePlugin() {}

ExamplePlugin::~ExamplePlugin() {}

void ExamplePlugin::initialize()
{
  logger_ = std::make_unique<rclcpp::Logger>(rclcpp::get_logger(plugin_name_));
}

bool ExamplePlugin::clips_env_init(std::shared_ptr<clips::Environment> & env)
{
  auto context = CLIPSEnvContext::get_context(env.get());
  RCLCPP_INFO(*logger_, "Initializing plugin for environment %s", context->env_name_.c_str());
  return true;
}

bool ExamplePlugin::clips_env_destroyed(std::shared_ptr<clips::Environment> & env)
{
  auto context = CLIPSEnvContext::get_context(env.get());
  RCLCPP_INFO(*logger_, "Destroying plugin for environment %s", context->env_name_.c_str());
  return true;
}
}  // namespace cx

PLUGINLIB_EXPORT_CLASS(cx::ExamplePlugin, cx::ClipsPlugin)
