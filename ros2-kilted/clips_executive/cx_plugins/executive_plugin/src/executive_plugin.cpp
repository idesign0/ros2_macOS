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

#include "cx_executive_plugin/executive_plugin.hpp"

#include <string>

#include "ament_index_cpp/get_package_share_directory.hpp"
#include "cx_utils/clips_env_context.hpp"
#include "cx_utils/param_utils.hpp"

// To export as plugin
#include "pluginlib/class_list_macros.hpp"

using std::placeholders::_1;
namespace cx
{
ExecutivePlugin::ExecutivePlugin() {}

ExecutivePlugin::~ExecutivePlugin()
{
  if (agenda_refresh_timer_) {
    agenda_refresh_timer_->cancel();
  }
}
void ExecutivePlugin::finalize()
{
  agenda_refresh_timer_->cancel();
  {
    std::scoped_lock<std::mutex> set_guard(envs_mutex_);
    managed_envs.clear();
  }
  clips_agenda_refresh_pub_.reset();
}

void ExecutivePlugin::initialize()
{
  logger_ = std::make_unique<rclcpp::Logger>(rclcpp::get_logger(plugin_name_));
  auto node = parent_.lock();
  if (!node) {
    return;
  }
  plugin_path_ = ament_index_cpp::get_package_share_directory("cx_executive_plugin");

  cx::cx_utils::declare_parameter_if_not_declared(
    node, plugin_name_ + ".publish_on_refresh", rclcpp::ParameterValue(false));
  cx::cx_utils::declare_parameter_if_not_declared(
    node, plugin_name_ + ".assert_time", rclcpp::ParameterValue(true));
  cx::cx_utils::declare_parameter_if_not_declared(
    node, plugin_name_ + ".refresh_rate", rclcpp::ParameterValue(10));
  node->get_parameter(plugin_name_ + ".publish_on_refresh", publish_on_refresh_);
  node->get_parameter(plugin_name_ + ".assert_time", assert_time_);
  node->get_parameter(plugin_name_ + ".refresh_rate", refresh_rate_);
  if (publish_on_refresh_) {
    clips_agenda_refresh_pub_ = node->create_publisher<std_msgs::msg::Empty>(
      "clips_executive/refresh_agenda", rclcpp::QoS(10));
  }
  double rate = 1.0 / refresh_rate_;
  // Sets the time between each clips agenda refresh in ns
  publish_rate_ = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
    std::chrono::duration<double>(rate));
  RCLCPP_DEBUG(
    *logger_, "Publishing rate set to: %ldns",
    std::chrono::duration_cast<std::chrono::nanoseconds>(publish_rate_).count());
  agenda_refresh_timer_ = node->create_wall_timer(publish_rate_, [this]() {
    std::scoped_lock<std::mutex> set_guard(envs_mutex_);
    for (auto & env : managed_envs) {
      auto context = CLIPSEnvContext::get_context(env);
      std::scoped_lock<std::mutex> env_guard(context->env_mtx_);

      if (assert_time_) {
        clips::AssertString(env.get(), "(time (now))");
      }

      clips::RefreshAllAgendas(env.get());
      clips::Run(env.get(), -1);
    }
    if (publish_on_refresh_) {
      clips_agenda_refresh_pub_->publish(std_msgs::msg::Empty());
    }
  });
}

std::shared_ptr<rclcpp_lifecycle::LifecycleNode> ExecutivePlugin::get_node()
{
  return parent_.lock();
}

bool ExecutivePlugin::clips_env_init(std::shared_ptr<clips::Environment> & env)
{
  if (publish_on_refresh_) {
    clips_agenda_refresh_pub_->on_activate();
  }
  {
    clips::AddUDF(
      env.get(), "now", "d", 0, 0, NULL,
      [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
        ExecutivePlugin * instance = static_cast<ExecutivePlugin *>(udfc->context);
        double currentTime = instance->get_node()->get_clock()->now().seconds();
        out->floatValue = clips::CreateFloat(env, currentTime);
      },
      "clips_now", this);

    clips::AddUDF(
      env.get(), "now-systime", "d", 0, 0, NULL,
      [](clips::Environment * env, clips::UDFContext * /*udfc*/, clips::UDFValue * out) {
        auto now = std::chrono::time_point_cast<std::chrono::duration<double>>(
          std::chrono::system_clock::now());

        out->floatValue = clips::CreateFloat(env, now.time_since_epoch().count());
      },
      "clips_now_systime", NULL);
    std::vector<std::string> files{plugin_path_ + "/clips/cx_executive_plugin/time.clp"};
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
  }
  {
    std::scoped_lock envs_guard(envs_mutex_);
    managed_envs.push_back(env);
  }
  return true;
}

bool ExecutivePlugin::clips_env_destroyed(std::shared_ptr<clips::Environment> & env)
{
  {
    clips::RemoveUDF(env.get(), "now");
    clips::RemoveUDF(env.get(), "now-systime");

    clips::Defrule * curr_rule = clips::FindDefrule(env.get(), "time-retract");
    if (curr_rule) {
      clips::Undefrule(curr_rule, env.get());
    }
    clips::Defglobal * curr_glob = clips::FindDefglobal(env.get(), "*PRIORITY-TIME-RETRACT*");
    if (curr_glob) {
      clips::Undefglobal(curr_glob, env.get());
    }
  }

  {
    std::scoped_lock envs_guard(envs_mutex_);
    managed_envs.erase(
      std::remove_if(
        managed_envs.begin(), managed_envs.end(),
        [&env](std::shared_ptr<clips::Environment> & obj) { return obj.get() == env.get(); }),
      managed_envs.end());
  }

  return true;
}
}  // namespace cx

PLUGINLIB_EXPORT_CLASS(cx::ExecutivePlugin, cx::ClipsPlugin)
