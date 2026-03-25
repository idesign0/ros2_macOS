// Copyright (c) 2024-2026 Carologistics
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// Read the full text in the LICENSE.GPL file in the main directory.

/***************************************************************************
 *  protobuf_plugin.cpp
 *
 *  Created: October 13th, 2023
 *  Copyright  2023 Daniel Swoboda
 *  Coipyright 2024 Tarik Viehmann
 ****************************************************************************/

#include "cx_protobuf_plugin/protobuf_plugin.hpp"

#include <map>
#include <memory>
#include <string>

#include "ament_index_cpp/get_package_share_directory.hpp"
#include "cx_plugin/clips_plugin.hpp"
#include "cx_protobuf_plugin/communicator.hpp"
#include "cx_utils/clips_env_context.hpp"
#include "cx_utils/param_utils.hpp"

// To export as plugin
#include "pluginlib/class_list_macros.hpp"

namespace cx
{

ProtobufPlugin::ProtobufPlugin() {}
ProtobufPlugin::~ProtobufPlugin() {}

void ProtobufPlugin::initialize()
{
  plugin_path_ = ament_index_cpp::get_package_share_directory("cx_protobuf_plugin");
  auto node = parent_.lock();
  if (node) {
    std::vector<std::string> package_share_dirs, input_proto_paths;
    cx::cx_utils::declare_parameter_if_not_declared(
      node, plugin_name_ + ".proto_paths", rclcpp::ParameterValue(std::vector<std::string>()));
    node->get_parameter(plugin_name_ + ".proto_paths", input_proto_paths);
    cx::cx_utils::declare_parameter_if_not_declared(
      node, plugin_name_ + ".pkg_share_dirs", rclcpp::ParameterValue(std::vector<std::string>()));
    node->get_parameter(plugin_name_ + ".pkg_share_dirs", package_share_dirs);
    logger_ = std::make_unique<rclcpp::Logger>(rclcpp::get_logger(plugin_name_));
    try {
      cx_utils::resolve_files(input_proto_paths, package_share_dirs, paths_);
    } catch (std::exception & e) {
      RCLCPP_ERROR(*logger_, e.what());
    }
  }
}

bool ProtobufPlugin::clips_env_init(std::shared_ptr<clips::Environment> & env)
{
  RCLCPP_DEBUG(*logger_, "Initializing context for plugin %s", plugin_name_.c_str());
  auto context = CLIPSEnvContext::get_context(env.get());
  protobuf_communicator_[context->env_name_] =
    std::make_unique<protobuf_clips::ClipsProtobufCommunicator>(
      env.get(), context->env_mtx_, paths_, parent_);

  std::vector<std::string> files{plugin_path_ + "/clips/cx_protobuf_plugin/protobuf.clp"};
  for (const auto & f : files) {
    if (!clips::BatchStar(env.get(), f.c_str())) {
      RCLCPP_ERROR(
        *logger_,
        "Failed to initialize CLIPS environment, "
        "batch file '%s' failed!, aborting...",
        f.c_str());
      return false;
    }
  }

  RCLCPP_INFO(rclcpp::get_logger(plugin_name_), "Initialised context!");

  return true;
}

bool ProtobufPlugin::clips_env_destroyed(std::shared_ptr<clips::Environment> & env)
{
  RCLCPP_INFO(rclcpp::get_logger(plugin_name_), "Destroying clips context!");

  auto context = CLIPSEnvContext::get_context(env.get());
  protobuf_communicator_.erase(context->env_name_);
  return true;
}
}  // namespace cx
PLUGINLIB_EXPORT_CLASS(cx::ProtobufPlugin, cx::ClipsPlugin)
