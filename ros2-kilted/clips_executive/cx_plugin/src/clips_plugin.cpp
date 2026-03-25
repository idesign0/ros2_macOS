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

#include "cx_plugin/clips_plugin.hpp"

#include <string>
namespace cx
{

ClipsPlugin::ClipsPlugin() {}
ClipsPlugin::~ClipsPlugin() {}

std::string ClipsPlugin::get_plugin_name() const { return plugin_name_; }

void ClipsPlugin::initialize() {}

void ClipsPlugin::finalize() {}

void ClipsPlugin::initialize(
  const rclcpp_lifecycle::LifecycleNode::WeakPtr parent, const std::string & plugin_name)
{
  parent_ = parent;
  plugin_name_ = plugin_name;
  initialize();
}

}  // namespace cx
