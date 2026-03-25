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

#ifndef CX_CLIPS_ENV_MANAGER__CLIPS_ENV_MANAGER_HPP_
#define CX_CLIPS_ENV_MANAGER__CLIPS_ENV_MANAGER_HPP_

#include <spdlog/spdlog.h>

#include <algorithm>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>

#include "bond/msg/constants.hpp"
#include "bondcpp/bond.hpp"
#include "cx_clips_env_manager/clips_plugin_manager.hpp"
#include "cx_msgs/srv/create_clips_env.hpp"
#include "cx_msgs/srv/destroy_clips_env.hpp"
#include "cx_msgs/srv/list_clips_envs.hpp"
#include "lifecycle_msgs/msg/state.hpp"
#include "lifecycle_msgs/msg/transition.hpp"
#include "rclcpp/clock.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"

namespace cx
{
class ClipsPluginManager;
class ClipsExecutive;

class CLIPSEnvManager : public rclcpp_lifecycle::LifecycleNode
{
  friend ClipsPluginManager;
  friend ClipsExecutive;

public:
  explicit CLIPSEnvManager(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
  ~CLIPSEnvManager();
  using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;
  // using EnvsMap = std::unordered_map<std::string,
  // cx::LockSharedPtr<clips::Environment>>;

  CallbackReturn on_configure(const rclcpp_lifecycle::State & state);
  CallbackReturn on_activate(const rclcpp_lifecycle::State & state);
  CallbackReturn on_deactivate(const rclcpp_lifecycle::State & state);
  CallbackReturn on_shutdown(const rclcpp_lifecycle::State & state);
  CallbackReturn on_error(const rclcpp_lifecycle::State & state);

  // std::map<std::string, LockSharedPtr<clips::Environment>>
  // getEnvironments() const;

  // LockSharedPtr<clips::Environment>
  // getEnvironmentByName(const std::string &env_name);
  void list_envs_callback(
    const std::shared_ptr<rmw_request_id_t> request_header,
    const std::shared_ptr<cx_msgs::srv::ListClipsEnvs::Request> request,
    const std::shared_ptr<cx_msgs::srv::ListClipsEnvs::Response> response);

  void create_env_callback(
    const std::shared_ptr<rmw_request_id_t> request_header,
    const std::shared_ptr<cx_msgs::srv::CreateClipsEnv::Request> request,
    const std::shared_ptr<cx_msgs::srv::CreateClipsEnv::Response> response);

  void destroy_env_callback(
    const std::shared_ptr<rmw_request_id_t> request_header,
    const std::shared_ptr<cx_msgs::srv::DestroyClipsEnv::Request> request,
    const std::shared_ptr<cx_msgs::srv::DestroyClipsEnv::Response> response);

private:
  std::shared_ptr<clips::Environment> new_env(const std::string & env_name);

  bool delete_env(const std::string & env_name);
  // void assert_plugins(LockSharedPtr<clips::Environment> &clips,
  //                     bool immediate_assert);
  // void add_functions(const std::string &env_name);
  // bool guarded_load(LockSharedPtr<clips::Environment> &env, const std::string
  // &filename);

  // ROS2 SERVICES
  rclcpp::Service<cx_msgs::srv::ListClipsEnvs>::SharedPtr list_envs_service_;
  rclcpp::Service<cx_msgs::srv::CreateClipsEnv>::SharedPtr create_env_service_;
  rclcpp::Service<cx_msgs::srv::DestroyClipsEnv>::SharedPtr destroy_env_service_;

private:
  void create_bond();
  void destroy_bond();
  void autostart();
  void register_rcl_preshutdown_callback();
  void run_cleanups();
  void on_rcl_preshutdown();

  ClipsPluginManager plugin_manager_;

  std::shared_ptr<EnvsMap> envs_;
  std::shared_ptr<std::mutex> map_mtx_;

  // Connection to tell that server is still up
  std::unique_ptr<bond::Bond> bond_{nullptr};
  double bond_heartbeat_period;
  rclcpp::TimerBase::SharedPtr autostart_timer_;
  std::unique_ptr<rclcpp::PreShutdownCallbackHandle> rcl_preshutdown_cb_handle_{nullptr};
};
}  // namespace cx

#endif  // CX_CLIPS_ENV_MANAGER__CLIPS_ENV_MANAGER_HPP_
