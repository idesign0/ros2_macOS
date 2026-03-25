// Copyright (c) 2026 Carologistics
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

#ifdef HAVE_GENERIC_ACTION_CLIENT
#include "action_msgs/msg/goal_status.hpp"
#include "cx_ros_msgs_plugin/ros_msgs_plugin.hpp"
#include "cx_utils/clips_env_context.hpp"
#include "rclcpp_action/create_generic_client.hpp"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"

namespace cx
{
void RosMsgsPlugin::finalize_generic_action_client_bindings()
{
  if (client_goal_handles_.size() > 0) {
    client_goal_handles_.clear();
  }
  if (action_clients_.size() > 0) {
    for (const auto & client_map : action_clients_) {
      for (const auto & client : client_map.second) {
        RCLCPP_WARN(
          *logger_, "Environment %s has open %s action clients, cleaning up ...",
          client_map.first.c_str(), client.first.c_str());
      }
    }
    action_clients_.clear();
  }
}

void RosMsgsPlugin::init_generic_action_client_bindings(std::shared_ptr<clips::Environment> env)
{
  std::string fun_name = "ros-msgs-create-action-client";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";sy;sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue action_server;
      clips::UDFValue type;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &action_server);
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &type);

      instance->create_new_action_client(
        env, action_server.lexemeValue->contents, type.lexemeValue->contents);
    },
    "create_new_action_client", this);

  fun_name = "ros-msgs-destroy-action-client";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue action_server;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &action_server);

      instance->destroy_action_client(env, action_server.lexemeValue->contents);
    },
    "destroy_action_client", this);

  fun_name = "ros-msgs-create-goal-request";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "e", 1, 1, ";sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue type;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &type);
      *out = instance->create_goal(env, type.lexemeValue->contents);
    },
    "create_goal", this);

  fun_name = "ros-msgs-async-send-goal";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";e;sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      (void)out;
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue action_server;
      clips::UDFValue goal_request;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &goal_request);
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &action_server);
      instance->async_send_new_goal(
        env, action_server.lexemeValue->contents, goal_request.externalAddressValue->contents);
    },
    "async_send_goal", this);

  fun_name = "ros-msgs-async-cancel-goal";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";sy;e",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue goal_handle_val, server_name_val;
      using namespace clips;  // NOLINT

      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &server_name_val);
      clips::UDFNthArgument(udfc, 2, EXTERNAL_ADDRESS_BIT, &goal_handle_val);

      std::string server_name = server_name_val.lexemeValue->contents;

      std::scoped_lock lock{instance->map_mtx_};

      auto gh_it =
        instance->client_goal_handles_.find(goal_handle_val.externalAddressValue->contents);
      if (gh_it == instance->client_goal_handles_.end()) {
        RCLCPP_ERROR(*(instance->logger_), "ros-msgs-async-cancel-goal: Unknown goal handle");
        clips::UDFThrowError(udfc);
        return;
      }

      auto goal_handle = gh_it->second;
      auto context = CLIPSEnvContext::get_context(env);
      std::string env_name = context->env_name_;

      instance->action_clients_[env_name][server_name]->async_cancel_goal(
        goal_handle, [goal_handle, env, instance, server_name](auto response) {
          instance->process_cancel_response(env, server_name, goal_handle, response);
        });
    },
    "async_cancel_goal", this);

  fun_name = "ros-msgs-destroy-client-goal-handle";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";e",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      (void)env;
      (void)out;
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue client_goal_handle;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &client_goal_handle);
      instance->destroy_client_goal_handle(client_goal_handle.externalAddressValue->contents);
    },
    "destroy_client_goal_handle", this);

  fun_name = "ros-msgs-client-goal-handle-get-goal-id";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "y", 1, 1, ";e",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue client_goal_handle;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &client_goal_handle);
      *out = instance->client_goal_handle_get_goal_id(
        env, udfc, client_goal_handle.externalAddressValue->contents);
    },
    "client_goal_handle_get_goal_id", this);

  fun_name = "ros-msgs-client-goal-handle-get-goal-stamp";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "d", 1, 1, ";e",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue client_goal_handle;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &client_goal_handle);
      *out = instance->client_goal_handle_get_goal_stamp(
        env, udfc, client_goal_handle.externalAddressValue->contents);
    },
    "client_goal_handle_get_goal_stamp", this);

  fun_name = "ros-msgs-client-goal-handle-get-status";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "y", 1, 1, ";e",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue client_goal_handle;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &client_goal_handle);
      *out = instance->client_goal_handle_get_status(
        env, udfc, client_goal_handle.externalAddressValue->contents);
    },
    "client_goal_handle_get_status", this);

  fun_name = "ros-msgs-client-goal-handle-is-feedback-aware";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "b", 1, 1, ";e",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue client_goal_handle;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &client_goal_handle);
      auto gh_it =
        instance->client_goal_handles_.find(client_goal_handle.externalAddressValue->contents);
      if (gh_it == instance->client_goal_handles_.end()) {
        RCLCPP_ERROR(
          *(instance->logger_),
          "ros-msgs-client-goal-handle-is-feedback-aware: unknown goal handle");
        clips::UDFThrowError(udfc);
        return;
      }

      auto goal_handle = gh_it->second;
      clips::UDFValue res;
      res.lexemeValue = clips::CreateBoolean(env, goal_handle->is_feedback_aware());
      res.begin = 0;
      res.range = -1;
      *out = res;
    },
    "client_goal_handle_is_feedback_aware", this);

  fun_name = "ros-msgs-client-goal-handle-is-result-aware";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "b", 1, 1, ";e",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue client_goal_handle;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &client_goal_handle);
      auto gh_it =
        instance->client_goal_handles_.find(client_goal_handle.externalAddressValue->contents);
      if (gh_it == instance->client_goal_handles_.end()) {
        RCLCPP_ERROR(
          *(instance->logger_),
          "ros-msgs-client-goal-handle-is-feedback-aware: unknown goal handle");
        clips::UDFThrowError(udfc);
        return;
      }

      auto goal_handle = gh_it->second;
      clips::UDFValue res;
      res.lexemeValue = clips::CreateBoolean(env, goal_handle->is_result_aware());
      res.begin = 0;
      res.range = -1;
      *out = res;
    },
    "client_goal_handle_is_result_aware", this);

  clips::Build(
    env.get(),
    R"((deftemplate ros-msgs-action-client
            (slot server (type STRING))
            (slot type (type STRING))))");
  clips::Build(
    env.get(),
    R"((deftemplate ros-msgs-goal-response
            (slot server (type STRING))
            (slot client-goal-handle-ptr (type EXTERNAL-ADDRESS))
            ))");
  clips::Build(
    env.get(),
    R"((deftemplate ros-msgs-wrapped-result
            (slot server (type STRING))
            (slot goal-id (type SYMBOL))
            (slot code (type SYMBOL) (allowed-values UNKNOWN SUCCEEDED CANCELED ABORTED))
            (slot result-ptr (type EXTERNAL-ADDRESS))
            ))");
  clips::Build(
    env.get(),
    R"((deftemplate ros-msgs-feedback
            (slot server (type STRING))
            (slot client-goal-handle-ptr (type EXTERNAL-ADDRESS))
            (slot feedback-ptr (type EXTERNAL-ADDRESS))
            ))");
  clips::Build(
    env.get(),
    R"((deftemplate ros-msgs-cancel-response
            (slot server (type STRING))
            (slot client-goal-handle-ptr (type EXTERNAL-ADDRESS))
            (slot cancel-response-ptr (type EXTERNAL-ADDRESS))
            ))");
}

void RosMsgsPlugin::destroy_generic_action_client_bindings(std::shared_ptr<clips::Environment> env)
{
  clips::Deftemplate * curr_tmpl = clips::FindDeftemplate(env.get(), "ros-msgs-action-client");
  if (curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_, "ros-msgs-action-client cant be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "ros-msgs-wrapped-result");
  if (curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_, "ros-msgs-wrapped-result cant be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "ros-msgs-feedback");
  if (curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_, "ros-msgs-feedback cant be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "ros-msgs-goal-response");
  if (curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_, "ros-msgs-goal-response cant be undefined");
  }
}

void RosMsgsPlugin::create_new_action_client(
  clips::Environment * env, const std::string & action_server_name, const std::string & action_type)
{
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;

  auto node = parent_.lock();
  if (!node) {
    RCLCPP_ERROR(*logger_, "Invalid reference to parent node");
  }

  std::map<
    std::__cxx11::basic_string<char>, std::shared_ptr<rclcpp_action::GenericClient>>::iterator it;
  {
    map_mtx_.lock();
    it = action_clients_[env_name].find(action_server_name);
  }

  if (it != action_clients_[env_name].end()) {
    map_mtx_.unlock();
    RCLCPP_WARN(*logger_, "Already registered to action client for %s", action_server_name.c_str());
  } else {
    RCLCPP_DEBUG(*logger_, "Creating action client for %s", action_server_name.c_str());

    if (!libs_.contains(action_type)) {
      libs_[action_type] = rclcpp::get_typesupport_library(action_type, "rosidl_typesupport_cpp");
    }
    if (!action_type_support_cache_.contains(action_type)) {
      action_type_support_cache_[action_type] = rclcpp::get_action_typesupport_handle(
        action_type, "rosidl_typesupport_cpp", *libs_[action_type]);
    }
    action_types_[env_name][action_server_name] = action_type;

    action_clients_[context->env_name_][action_server_name] = rclcpp_action::create_generic_client(
      node->get_node_base_interface(), node->get_node_graph_interface(),
      node->get_node_logging_interface(), node->get_node_waitables_interface(), action_server_name,
      action_type, cb_group_);
    map_mtx_.unlock();
    clips::AssertString(
      env, ("(ros-msgs-action-client (server \"" + action_server_name + "\") (type \"" +
            action_type + "\"))")
             .c_str());
  }
}

clips::UDFValue RosMsgsPlugin::create_goal(
  clips::Environment * env, const std::string & action_type)
{
  auto scoped_lock = std::scoped_lock{map_mtx_};
  std::shared_ptr<MessageInfo> ptr;

  if (!libs_.contains(action_type)) {
    RCLCPP_DEBUG(*logger_, "Create new action information on goal request creation");
    libs_[action_type] = rclcpp::get_typesupport_library(action_type, "rosidl_typesupport_cpp");
    action_type_support_cache_[action_type] = rclcpp::get_action_typesupport_handle(
      action_type, "rosidl_typesupport_cpp", *libs_[action_type]);
  }
  auto * introspection_type_support = get_service_typesupport_handle(
    action_type_support_cache_[action_type]->goal_service_type_support,
    rosidl_typesupport_introspection_cpp::typesupport_identifier);
  auto * members = static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
    introspection_type_support->request_typesupport->data);
  for (size_t i = 0; i < members->member_count_; ++i) {
    const auto & member = members->members_[i];
    if (strcmp(member.name_, "goal") == 0) {
      const rosidl_message_type_support_t * type_support = member.members_;
      if (type_support) {
        // Get the members of the nested message
        const rosidl_typesupport_introspection_cpp::MessageMembers * nested_members =
          static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
            type_support->data);
        std::string message_type = get_msg_type(nested_members);
        if (!libs_.contains(message_type)) {
          libs_[message_type] =
            rclcpp::get_typesupport_library(message_type, "rosidl_typesupport_cpp");
        }
        if (!type_support_cache_.contains(message_type)) {
          type_support_cache_[message_type] = rclcpp::get_message_typesupport_handle(
            message_type, "rosidl_typesupport_cpp", *libs_[message_type]);
        }
        ptr = std::make_shared<MessageInfo>(nested_members);
      }
    }
  }
  clips::UDFValue res;
  messages_[ptr.get()] = ptr;
  res.externalAddressValue =
    clips::CreateCExternalAddress(env, reinterpret_cast<void *>(ptr.get()));
  return res;
}

void RosMsgsPlugin::deep_copy_msg(
  const void * src, void * dest,
  const rosidl_typesupport_introspection_cpp::MessageMembers * members)
{
  for (size_t i = 0; i < members->member_count_; ++i) {
    const auto & member = members->members_[i];

    // Obtain introspection information for the type of the submessage
    const rosidl_typesupport_introspection_cpp::MessageMembers * sub_members = members;

    // Move fields from the source sub-message to the target sub-message in
    // parent
    for (uint32_t i = 0; i < sub_members->member_count_; ++i) {
      const rosidl_typesupport_introspection_cpp::MessageMember * sub_member =
        &sub_members->members_[i];

      // Calculate the offset for the source and target members
      const void * source_field_ptr = reinterpret_cast<const uint8_t *>(src) + sub_member->offset_;
      void * target_field_ptr = reinterpret_cast<uint8_t *>(dest) + sub_member->offset_;

      switch (sub_member->type_id_) {
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array of bool (std::vector<bool>)
              const auto * source_vector =
                reinterpret_cast<const std::vector<bool> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<bool> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array
              std::memcpy(
                target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(bool));
            }
          } else {
            // Single bool
            std::memcpy(target_field_ptr, source_field_ptr, sizeof(bool));
          }
          break;
        }

        case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array of int8 (std::vector<int8_t>)
              const auto * source_vector =
                reinterpret_cast<const std::vector<int8_t> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<int8_t> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array
              std::memcpy(
                target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(int8_t));
            }
          } else {
            // Single int8
            std::memcpy(target_field_ptr, source_field_ptr, sizeof(int8_t));
          }
          break;
        }

        case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_OCTET: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array of uint8 (std::vector<uint8_t>)
              new (target_field_ptr) std::vector<uint8_t>();

              const auto * source_vector =
                reinterpret_cast<const std::vector<uint8_t> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<uint8_t> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array
              std::memcpy(
                target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(uint8_t));
            }
          } else {
            // Single uint8
            std::memcpy(target_field_ptr, source_field_ptr, sizeof(uint8_t));
          }
          break;
        }

        case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array of int16 (std::vector<int16_t>)
              const auto * source_vector =
                reinterpret_cast<const std::vector<int16_t> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<int16_t> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array
              std::memcpy(
                target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(int16_t));
            }
          } else {
            // Single int16
            std::memcpy(target_field_ptr, source_field_ptr, sizeof(int16_t));
          }
          break;
        }

        case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array of uint16 (std::vector<uint16_t>)
              const auto * source_vector =
                reinterpret_cast<const std::vector<uint16_t> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<uint16_t> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array
              std::memcpy(
                target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(uint16_t));
            }
          } else {
            // Single uint16
            std::memcpy(target_field_ptr, source_field_ptr, sizeof(uint16_t));
          }
          break;
        }

        case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array of int32 (std::vector<int32_t>)
              const auto * source_vector =
                reinterpret_cast<const std::vector<int32_t> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<int32_t> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array
              std::memcpy(
                target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(int32_t));
            }
          } else {
            // Single int32
            std::memcpy(target_field_ptr, source_field_ptr, sizeof(int32_t));
          }
          break;
        }

        case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array of int32 (std::vector<uint32_t>)
              const auto * source_vector =
                reinterpret_cast<const std::vector<uint32_t> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<uint32_t> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array
              std::memcpy(
                target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(uint32_t));
            }
          } else {
            // Single int32
            std::memcpy(target_field_ptr, source_field_ptr, sizeof(uint32_t));
          }
          break;
        }

        case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array of int64 (std::vector<int64_t>)
              const auto * source_vector =
                reinterpret_cast<const std::vector<int64_t> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<int64_t> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array
              std::memcpy(
                target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(int64_t));
            }
          } else {
            // Single int64
            std::memcpy(target_field_ptr, source_field_ptr, sizeof(int64_t));
          }
          break;
        }

        case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array of int64 (std::vector<uint64_t>)
              const auto * source_vector =
                reinterpret_cast<const std::vector<uint64_t> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<uint64_t> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array
              std::memcpy(
                target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(uint64_t));
            }
          } else {
            // Single int64
            std::memcpy(target_field_ptr, source_field_ptr, sizeof(uint64_t));
          }
          break;
        }

        case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array of int64 (std::vector<float>)
              const auto * source_vector =
                reinterpret_cast<const std::vector<float> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<float> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array
              std::memcpy(
                target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(float));
            }
          } else {
            // Single int64
            std::memcpy(target_field_ptr, source_field_ptr, sizeof(float));
          }
          break;
        }

        case rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array of int64 (std::vector<double>)
              const auto * source_vector =
                reinterpret_cast<const std::vector<double> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<double> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array
              std::memcpy(
                target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(double));
            }
          } else {
            // Single int64
            std::memcpy(target_field_ptr, source_field_ptr, sizeof(double));
          }
          break;
        }

        case rosidl_typesupport_introspection_cpp::ROS_TYPE_LONG_DOUBLE: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array of int64 (std::vector<long double>)
              const auto * source_vector =
                reinterpret_cast<const std::vector<long double> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<long double> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array
              std::memcpy(
                target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(long double));
            }
          } else {
            // Single int64
            std::memcpy(target_field_ptr, source_field_ptr, sizeof(long double));
          }
          break;
        }

        case rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array of strings (std::vector<std::string>)
              const auto * source_vector =
                reinterpret_cast<const std::vector<std::string> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<std::string> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array of strings
              const auto source_array = reinterpret_cast<const std::string *>(source_field_ptr);
              auto target_array = reinterpret_cast<std::string *>(target_field_ptr);
              for (size_t i = 0; i < sub_member->array_size_; ++i) {
                target_array[i] = source_array[i];
              }
            }
          } else {
            // Single string (std::string)
            const auto * source_str = reinterpret_cast<const std::string *>(source_field_ptr);
            auto * target_str = reinterpret_cast<std::string *>(target_field_ptr);
            *target_str = *source_str;
          }
          break;
        }
        case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array (std::vector<char>)
              const auto * source_vector =
                reinterpret_cast<const std::vector<char> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<char> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array
              std::memcpy(
                target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(char));
            }
          } else {
            // Single char
            std::memcpy(target_field_ptr, source_field_ptr, sizeof(char));
          }
          break;
        }

        case rosidl_typesupport_introspection_cpp::ROS_TYPE_WCHAR: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array (std::vector<wchar_t>)
              const auto * source_vector =
                reinterpret_cast<const std::vector<wchar_t> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<wchar_t> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array
              std::memcpy(
                target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(wchar_t));
            }
          } else {
            // Single wchar_t
            std::memcpy(target_field_ptr, source_field_ptr, sizeof(wchar_t));
          }
          break;
        }

        case rosidl_typesupport_introspection_cpp::ROS_TYPE_WSTRING: {
          if (sub_member->is_array_) {
            if (!sub_member->is_upper_bound_) {
              // Dynamically-sized array (std::vector<wchar_t>)
              const auto * source_vector =
                reinterpret_cast<const std::vector<wchar_t> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<wchar_t> *>(target_field_ptr);
              *target_vector = *source_vector;
            } else {
              // Fixed-size array
              std::memcpy(
                target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(wchar_t));
            }
          } else {
            // Single wstring (using std::wstring)
            const auto * source_string = reinterpret_cast<const std::wstring *>(source_field_ptr);
            auto * target_string = reinterpret_cast<std::wstring *>(target_field_ptr);
            *target_string = *source_string;
          }
          break;
        }

        case rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE: {
          const auto * sub_members =
            static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
              sub_member->members_->data);

          if (member.is_array_) {
            if (!member.is_upper_bound_) {
              // Dynamically-sized array of sub-messages
              const auto * source_vector =
                reinterpret_cast<const std::vector<void *> *>(source_field_ptr);
              auto * target_vector = reinterpret_cast<std::vector<void *> *>(target_field_ptr);
              target_vector->resize(source_vector->size());

              for (size_t i = 0; i < source_vector->size(); ++i) {
                deep_copy_msg(source_vector->at(i), target_vector->at(i), sub_members);
              }
            } else {
              // Fixed-size array of sub-messages
              const auto source_array = reinterpret_cast<const void * const *>(source_field_ptr);
              auto target_array = reinterpret_cast<void **>(target_field_ptr);

              for (size_t i = 0; i < member.array_size_; ++i) {
                deep_copy_msg(source_array[i], target_array[i], sub_members);
              }
            }
          } else {
            // Single nested sub-message
            for (size_t j = 0; j < sub_members->member_count_; ++j) {
              deep_copy_msg(source_field_ptr, target_field_ptr, sub_members);
            }
          }
          break;
        }

        default:
          throw std::runtime_error("Unsupported field type.");
      }
    }
  }
}

void RosMsgsPlugin::async_send_new_goal(
  clips::Environment * env, const std::string & action_server, void * deserialized_goal)
{
  using namespace std::chrono_literals;
  // Handle the request asynchronously to not block clips engine potentially
  // endlessly
  std::thread([this, env, deserialized_goal, action_server]() {
    auto context = CLIPSEnvContext::get_context(env);
    std::shared_ptr<MessageInfo> msg_info;
    std::shared_ptr<rclcpp_action::GenericClient> client;
    std::string env_name = context->env_name_;
    {
      auto scoped_lock = std::scoped_lock{map_mtx_};
      if (!messages_.contains(deserialized_goal)) {
        RCLCPP_ERROR(*logger_, "Failed to send goal, invalid msg pointer");
      }
      msg_info = messages_[deserialized_goal];
      auto env_it = action_clients_.find(env_name);
      if (env_it == action_clients_.end()) {
        RCLCPP_ERROR(*logger_, "No clients for env %s", env_name.c_str());
        return;
      }
      auto server_it = env_it->second.find(action_server);
      if (server_it == env_it->second.end()) {
        RCLCPP_ERROR(*logger_, "No client for server %s", action_server.c_str());
        return;
      }
      client = server_it->second;
    }

    std::string msg_type = get_msg_type(msg_info->members);
    bool print_warning = true;
    while (!stop_flag_ && !client->wait_for_action_server(1s)) {
      if (stop_flag_ || !rclcpp::ok()) {
        RCLCPP_ERROR(*logger_, "Interrupted while waiting for the server. Exiting.");
      }
      if (print_warning) {
        RCLCPP_WARN(*logger_, "server %s not available, start waiting", action_server.c_str());
        print_warning = false;
      }
      RCLCPP_DEBUG(*logger_, "server %s not available, waiting again...", action_server.c_str());
    }
    if (!print_warning) {
      RCLCPP_INFO(*logger_, "server %s is finally reachable", action_server.c_str());
    }
    std::lock_guard<std::mutex> guard(context->env_mtx_);
    if (stop_flag_) {
      RCLCPP_DEBUG(*logger_, "Shutdown during async call.");
    }
    rclcpp_action::GenericClient::SendGoalOptions send_goal_options;
    send_goal_options.goal_response_callback =
      [this, context, env,
       action_server](const std::shared_ptr<rclcpp_action::GenericClientGoalHandle> & goal_handle) {
        std::scoped_lock map_lock{map_mtx_};
        task_queue_.push([this, context, env, action_server, goal_handle]() {
          {
            std::scoped_lock map_lock{map_mtx_};
            client_goal_handles_.try_emplace(goal_handle.get(), goal_handle);
          }
          std::lock_guard<std::mutex> guard(context->env_mtx_);
          clips::FactBuilder * fact_builder =
            clips::CreateFactBuilder(env, "ros-msgs-goal-response");
          clips::FBPutSlotString(fact_builder, "server", action_server.c_str());
          clips::FBPutSlotCLIPSExternalAddress(
            fact_builder, "client-goal-handle-ptr",
            clips::CreateCExternalAddress(env, goal_handle.get()));
          clips::FBAssert(fact_builder);
          clips::FBDispose(fact_builder);
        });
        cv_.notify_one();  // Notify the worker thread
      };

    send_goal_options.feedback_callback =
      [this, context, env, action_server](
        std::shared_ptr<rclcpp_action::GenericClientGoalHandle> goal_handle,
        const void * feedback) {
        // the feedback ptr will be freed after this function is called
        // process it before deferring CLIPS logic to worker thread
        std::shared_ptr<MessageInfo> ptr;
        {
          std::scoped_lock map_lock{map_mtx_};

          std::string action_type = action_types_[context->env_name_][action_server];
          auto * introspection_type_support = get_message_typesupport_handle(
            action_type_support_cache_[action_type]->feedback_message_type_support,
            rosidl_typesupport_introspection_cpp::typesupport_identifier);

          auto * members =
            static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
              introspection_type_support->data);

          // the introspection information wraps the actual feedback message
          // with a message containing goal_id and the feedback as members
          for (size_t i = 0; i < members->member_count_; ++i) {
            const auto & member = members->members_[i];
            if (strcmp(member.name_, "feedback") == 0) {
              const rosidl_message_type_support_t * type_support = member.members_;
              const rosidl_typesupport_introspection_cpp::MessageMembers * nested_members =
                static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
                  type_support->data);
              std::string message_type = get_msg_type(nested_members);
              if (!libs_.contains(message_type)) {
                libs_[message_type] =
                  rclcpp::get_typesupport_library(message_type, "rosidl_typesupport_cpp");
              }
              if (!type_support_cache_.contains(message_type)) {
                type_support_cache_[message_type] = rclcpp::get_message_typesupport_handle(
                  message_type, "rosidl_typesupport_cpp", *libs_[message_type]);
              }
              ptr = std::make_shared<MessageInfo>(nested_members);
              deep_copy_msg(feedback, ptr->msg_ptr, nested_members);
              messages_[ptr.get()] = ptr;
            }
          }
        }
        // this indirection is necessary as the feedback callback is called
        // while an internal lock is acquired. The same lock is acquired in
        // calls of get_status(). This can cause a deadlock if get_status()
        // is called from within clips right after a callback is received.
        // clips lock is still held, hence callback can't proceed, but
        // internal lock is already acquired when this callback is invoked.
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_queue_.push([this, context, env, action_server, goal_handle, ptr]() {
          // Enqueue the task to avoid directly locking the handle_mutex_ in a
          // callback
          std::lock_guard<std::mutex> clips_guard(context->env_mtx_);
          clips::FactBuilder * fact_builder = clips::CreateFactBuilder(env, "ros-msgs-feedback");
          clips::FBPutSlotString(fact_builder, "server", action_server.c_str());
          clips::FBPutSlotCLIPSExternalAddress(
            fact_builder, "client-goal-handle-ptr",
            clips::CreateCExternalAddress(env, goal_handle.get()));
          clips::FBPutSlotCLIPSExternalAddress(
            fact_builder, "feedback-ptr", clips::CreateCExternalAddress(env, ptr.get()));
          clips::FBAssert(fact_builder);
          clips::FBDispose(fact_builder);
        });
        cv_.notify_one();  // Notify the worker thread
      };

    send_goal_options.result_callback =
      [this, context, env, action_server](
        const rclcpp_action::GenericClientGoalHandle::WrappedResult & wrapped_result) {
        task_queue_.push([this, context, env, action_server, wrapped_result]() {
          std::shared_ptr<MessageInfo> result_msg;
          {
            std::scoped_lock map_lock{map_mtx_};
            std::string action_type = action_types_[context->env_name_][action_server];

            auto * introspection_type_support = get_service_typesupport_handle(
              action_type_support_cache_[action_type]->result_service_type_support,
              rosidl_typesupport_introspection_cpp::typesupport_identifier);

            auto * members =
              static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
                introspection_type_support->response_typesupport->data);
            for (size_t i = 0; i < members->member_count_; ++i) {
              const auto & member = members->members_[i];
              if (strcmp(member.name_, "result") == 0) {
                const rosidl_message_type_support_t * type_support = member.members_;
                // Get the members of the nested message
                const rosidl_typesupport_introspection_cpp::MessageMembers * nested_members =
                  static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
                    type_support->data);
                std::string message_type = get_msg_type(nested_members);
                if (!libs_.contains(message_type)) {
                  libs_[message_type] =
                    rclcpp::get_typesupport_library(message_type, "rosidl_typesupport_cpp");
                }
                if (!type_support_cache_.contains(message_type)) {
                  type_support_cache_[message_type] = rclcpp::get_message_typesupport_handle(
                    message_type, "rosidl_typesupport_cpp", *libs_[message_type]);
                }
                result_msg = std::make_shared<RosMsgsPlugin::MessageInfo>(nested_members);
                deep_copy_msg(wrapped_result.result, result_msg->msg_ptr, nested_members);
              }
            }
            messages_.try_emplace(result_msg.get(), result_msg);
          }
          std::lock_guard<std::mutex> guard(context->env_mtx_);
          clips::FactBuilder * fact_builder =
            clips::CreateFactBuilder(env, "ros-msgs-wrapped-result");
          clips::FBPutSlotString(fact_builder, "server", action_server.c_str());
          clips::FBPutSlotSymbol(
            fact_builder, "goal-id", rclcpp_action::to_string(wrapped_result.goal_id).c_str());
          std::string code_str = "UNKNOWN";
          switch (wrapped_result.code) {
            case rclcpp_action::GenericClientGoalHandle::ResultCode::UNKNOWN:
              break;
            case rclcpp_action::GenericClientGoalHandle::ResultCode::SUCCEEDED:
              code_str = "SUCCEEDED";
              break;
            case rclcpp_action::GenericClientGoalHandle::ResultCode::CANCELED:
              code_str = "CANCELED";
              break;
            case rclcpp_action::GenericClientGoalHandle::ResultCode::ABORTED:
              code_str = "ABORTED";
              break;
          }
          clips::FBPutSlotSymbol(fact_builder, "code", code_str.c_str());
          clips::FBPutSlotCLIPSExternalAddress(
            fact_builder, "result-ptr", clips::CreateCExternalAddress(env, result_msg.get()));
          clips::FBAssert(fact_builder);
          clips::FBDispose(fact_builder);
        });
        cv_.notify_one();  // Notify the worker thread
      };
    std::shared_ptr<MessageInfo> goal_info = messages_[deserialized_goal];
    RCLCPP_DEBUG(
      *logger_, "Sending Goal for %s of size %li", action_server.c_str(),
      goal_info->members->size_of_);
    client->async_send_goal(
      messages_[deserialized_goal]->msg_ptr, goal_info->members->size_of_, send_goal_options);
  }).detach();
}

void RosMsgsPlugin::destroy_action_client(clips::Environment * env, const std::string & server_name)
{
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;
  map_mtx_.lock();
  auto outer_it = action_clients_.find(env_name);
  if (outer_it != action_clients_.end()) {
    // Check if server_name exists in the inner map
    auto & inner_map = outer_it->second;
    auto inner_it = inner_map.find(server_name);
    if (inner_it != inner_map.end()) {
      // Remove the server_name entry from the inner map
      RCLCPP_DEBUG(*logger_, "Destroying action client for server %s", server_name.c_str());
      inner_map.erase(inner_it);
      map_mtx_.unlock();
      clips::Eval(
        env,
        ("(do-for-all-facts ((?f ros-msgs-action-client)) (eq (str-cat "
         "?f:server) (str-cat \"" +
         server_name + "\"))  (retract ?f))")
          .c_str(),
        NULL);
    } else {
      map_mtx_.unlock();
      RCLCPP_WARN(
        *logger_, "Action client %s not found in environment %s", server_name.c_str(),
        env_name.c_str());
    }
  } else {
    map_mtx_.unlock();
    RCLCPP_WARN(*logger_, "Environment %s not found", env_name.c_str());
  }
}

void RosMsgsPlugin::destroy_client_goal_handle(void * client_goal_handle)
{
  client_goal_handles_.erase(client_goal_handle);
}

clips::UDFValue RosMsgsPlugin::client_goal_handle_get_goal_id(
  clips::Environment * env, clips::UDFContext * udfc, void * client_goal_handle)
{
  std::scoped_lock map_lock{map_mtx_};
  clips::UDFValue res;
  auto typed_goal_handle = client_goal_handles_.at(client_goal_handle);
  if (!typed_goal_handle) {
    RCLCPP_ERROR(*logger_, "client-goal-handle-get-goal-id: Invalid pointer to handle");
    clips::UDFThrowError(udfc);
    return res;
  }
  rclcpp_action::GoalUUID goal_id = typed_goal_handle->get_goal_id();
  res.lexemeValue = clips::CreateSymbol(env, rclcpp_action::to_string(goal_id).c_str());
  res.begin = 0;
  res.range = -1;
  return res;
}

clips::UDFValue RosMsgsPlugin::client_goal_handle_get_goal_stamp(
  clips::Environment * env, clips::UDFContext * udfc, void * client_goal_handle)
{
  std::scoped_lock map_lock{map_mtx_};
  clips::UDFValue res;
  auto typed_goal_handle = client_goal_handles_.at(client_goal_handle);
  if (!typed_goal_handle) {
    RCLCPP_ERROR(*logger_, "client-goal-handle-get-goal-stamp: Invalid pointer to handle");
    clips::UDFThrowError(udfc);
    return res;
  }
  res.floatValue = clips::CreateFloat(env, typed_goal_handle->get_goal_stamp().seconds());
  res.begin = 0;
  res.range = -1;
  return res;
}

clips::UDFValue RosMsgsPlugin::client_goal_handle_get_status(
  clips::Environment * env, clips::UDFContext * udfc, void * client_goal_handle)
{
  std::scoped_lock map_lock{map_mtx_};
  clips::UDFValue res;
  auto typed_goal_handle = client_goal_handles_.at(client_goal_handle);
  if (!typed_goal_handle) {
    RCLCPP_ERROR(*logger_, "client-goal-handle-get-status: Invalid pointer to handle");
    clips::UDFThrowError(udfc);
    return res;
  }
  std::string code_str = "UNKNOWN";
  switch (typed_goal_handle->get_status()) {
    case action_msgs::msg::GoalStatus::STATUS_UNKNOWN:
      break;
    case action_msgs::msg::GoalStatus::STATUS_SUCCEEDED:
      code_str = "SUCCEEDED";
      break;
    case action_msgs::msg::GoalStatus::STATUS_CANCELED:
      code_str = "CANCELED";
      break;
    case action_msgs::msg::GoalStatus::STATUS_ABORTED:
      code_str = "ABORTED";
      break;
    case action_msgs::msg::GoalStatus::STATUS_ACCEPTED:
      code_str = "ACCEPTED";
      break;
    case action_msgs::msg::GoalStatus::STATUS_EXECUTING:
      code_str = "EXECUTING";
      break;
    case action_msgs::msg::GoalStatus::STATUS_CANCELING:
      code_str = "CANCELING";
      break;
  }
  res.lexemeValue = clips::CreateSymbol(env, code_str.c_str());
  res.begin = 0;
  res.range = -1;
  return res;
}

void RosMsgsPlugin::process_cancel_response(
  clips::Environment * env, const std::string & server_name,
  std::shared_ptr<rclcpp_action::GenericClientGoalHandle> goal_handle,
  const action_msgs::srv::CancelGoal::Response::SharedPtr & response)
{
  std::shared_ptr<RosMsgsPlugin::MessageInfo> ptr = store_generic_action_cancel_response(response);
  auto context = CLIPSEnvContext::get_context(env);
  std::lock_guard<std::mutex> lock(queue_mutex_);
  task_queue_.push([this, context, env, server_name, goal_handle, ptr]() {
    // Enqueue the task to avoid directly locking the handle_mutex_ in a
    // callback
    std::lock_guard<std::mutex> clips_guard(context->env_mtx_);
    clips::FactBuilder * fact_builder = clips::CreateFactBuilder(env, "ros-msgs-cancel-response");
    clips::FBPutSlotString(fact_builder, "server", server_name.c_str());
    clips::FBPutSlotCLIPSExternalAddress(
      fact_builder, "client-goal-handle-ptr",
      clips::CreateCExternalAddress(env, goal_handle.get()));
    clips::FBPutSlotCLIPSExternalAddress(
      fact_builder, "cancel-response-ptr", clips::CreateCExternalAddress(env, ptr.get()));
    clips::FBAssert(fact_builder);
    clips::FBDispose(fact_builder);
  });
  cv_.notify_one();  // Notify the worker thread
}

std::shared_ptr<RosMsgsPlugin::MessageInfo> RosMsgsPlugin::store_generic_action_cancel_response(
  std::shared_ptr<action_msgs::srv::CancelGoal::Response> cancel_response)
{
  std::string service_type = "action_msgs/srv/CancelGoal";

  const rosidl_typesupport_introspection_cpp::MessageMembers * introspection_info;

  {
    std::scoped_lock lock(map_mtx_);

    if (!libs_.contains(service_type)) {
      libs_[service_type] = rclcpp::get_typesupport_library(service_type, "rosidl_typesupport_cpp");
    }

    if (!service_type_support_cache_.contains(service_type)) {
      service_type_support_cache_[service_type] = rclcpp::get_service_typesupport_handle(
        service_type, "rosidl_typesupport_cpp", *libs_[service_type]);
    }

    auto * introspection_type_support = get_service_typesupport_handle(
      service_type_support_cache_[service_type],
      rosidl_typesupport_introspection_cpp::typesupport_identifier);

    introspection_info = static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
      introspection_type_support->response_typesupport->data);
  }

  // Copy the message content
  auto response_copy = std::make_shared<action_msgs::srv::CancelGoal::Response>(*cancel_response);

  // Wrap in MessageInfo
  auto msg_info = std::make_shared<MessageInfo>(introspection_info, response_copy);

  {
    std::scoped_lock lock(map_mtx_);
    messages_[msg_info.get()] = msg_info;
  }

  return msg_info;
}

}  // namespace cx
#endif
