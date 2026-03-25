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

#include "cx_file_load_plugin/file_load_plugin.hpp"

#include <string>

#include "cx_utils/clips_env_context.hpp"
#include "cx_utils/format.hpp"
#include "cx_utils/param_utils.hpp"

// To export as plugin
#include "pluginlib/class_list_macros.hpp"

using std::placeholders::_1;
namespace cx
{
FileLoadPlugin::FileLoadPlugin() {}

FileLoadPlugin::~FileLoadPlugin() {}

void FileLoadPlugin::initialize()
{
  logger_ = std::make_unique<rclcpp::Logger>(rclcpp::get_logger(plugin_name_));
  auto node = parent_.lock();
  if (!node) {
    return;
  }

  cx::cx_utils::declare_parameter_if_not_declared(
    node, plugin_name_ + ".pkg_share_dirs", rclcpp::ParameterValue(std::vector<std::string>()));
  cx::cx_utils::declare_parameter_if_not_declared(
    node, plugin_name_ + ".load", rclcpp::ParameterValue(std::vector<std::string>()));
  cx::cx_utils::declare_parameter_if_not_declared(
    node, plugin_name_ + ".batch", rclcpp::ParameterValue(std::vector<std::string>()));
  cx::cx_utils::declare_parameter_if_not_declared(
    node, plugin_name_ + ".cleanup_batch", rclcpp::ParameterValue(std::vector<std::string>()));
  std::vector<std::string> share_dirs;
  std::vector<std::string> files;
  std::vector<std::string> batch_files;
  std::vector<std::string> cleanup_files;
  node->get_parameter(plugin_name_ + ".pkg_share_dirs", share_dirs);
  node->get_parameter(plugin_name_ + ".load", files);
  node->get_parameter(plugin_name_ + ".batch", batch_files);
  node->get_parameter(plugin_name_ + ".cleanup_batch", cleanup_files);
  try {
    cx_utils::resolve_files(files, share_dirs, init_files_);
    cx_utils::resolve_files(batch_files, share_dirs, init_batch_files_);
    cx_utils::resolve_files(cleanup_files, share_dirs, cleanup_files_);
  } catch (std::exception & e) {
    RCLCPP_ERROR(*logger_, "%s", e.what());
  }
}

bool FileLoadPlugin::clips_env_init(std::shared_ptr<clips::Environment> & env)
{
  auto context = CLIPSEnvContext::get_context(env.get());
  RCLCPP_INFO(*logger_, "Initializing plugin for environment %s", context->env_name_.c_str());
  for (const auto & f : init_files_) {
    if (clips::EE_NO_ERROR != clips::Eval(env.get(), cx::format("(load* {})", f).c_str(), NULL)) {
      clips::Writeln(env.get(), cx::format("Failed to load file {}", f).c_str());
      RCLCPP_ERROR(*logger_, "Failed to load file '%s' failed!, aborting...", f.c_str());
      return false;
    }
  }
  for (const auto & f : init_batch_files_) {
    if (!clips::BatchStar(env.get(), f.c_str())) {
      clips::Writeln(env.get(), cx::format("Failed to initialize bach file {}", f).c_str());
      RCLCPP_ERROR(
        *logger_,
        "Failed to initialize"
        "batch file '%s' failed!, aborting...",
        f.c_str());
      return false;
    }
  }
  return true;
}

bool FileLoadPlugin::clips_env_destroyed(std::shared_ptr<clips::Environment> & env)
{
  for (const auto & f : cleanup_files_) {
    if (!clips::BatchStar(env.get(), f.c_str())) {
      clips::Writeln(env.get(), cx::format("Failed to initialize bach file {}", f).c_str());
      RCLCPP_ERROR(
        *logger_,
        "Failed to initialize"
        "batch file '%s' failed!, aborting...",
        f.c_str());
      return false;
    }
  }
  return true;
}
}  // namespace cx

PLUGINLIB_EXPORT_CLASS(cx::FileLoadPlugin, cx::ClipsPlugin)
