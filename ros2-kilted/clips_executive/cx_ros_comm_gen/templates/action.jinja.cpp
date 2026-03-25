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

/***************************************************************************
 *  {{name_camel}}.cpp
 *
 *  Automatically Generated: {{ gen_date }}
 ****************************************************************************/

// clang-format off

#include <chrono>
#include <map>
#include <memory>
#include <string>

#include <cx_plugin/clips_plugin.hpp>
#include "{{name_snake}}.hpp"
#include <cx_utils/clips_env_context.hpp>
#include "cx_utils/format.hpp"

// To export as plugin
#include "pluginlib/class_list_macros.hpp"

using std::placeholders::_1;

namespace cx {

{{name_camel}}::{{name_camel}}() {
    clips_worker_thread_ = std::thread([this] () {
  while (!stop_flag_) {
      std::function<void()> task;
      {
          std::unique_lock<std::mutex> lock(queue_mutex_);
          cv_.wait(lock, [&] { return !task_queue_.empty() || stop_flag_; });
          if(stop_flag_) {
            continue;
          }
          task = std::move(task_queue_.front());
          task_queue_.pop();
      }
      task();
  }});
}

{{name_camel}}::~{{name_camel}}() {
}

void {{name_camel}}::finalize() {
  stop_flag_ = true;
  cv_.notify_all();

  if (clips_worker_thread_.joinable()) {
      clips_worker_thread_.join();
  }
  std::scoped_lock map_lock{map_mtx_};
  if(clients_.size() > 0) {
    for (const auto &client_map : clients_) {
      for (const auto &client : client_map.second) {
        RCLCPP_WARN(*logger_,
                    "Environment %s has open %s client, cleaning up ...",
                    client_map.first.c_str(), client.first.c_str());
      }
    }
    clients_.clear();
  }
  if(servers_.size() > 0) {
    for (const auto &server_map : servers_) {
      for (const auto &server : server_map.second) {
        RCLCPP_WARN(*logger_,
                    "Environment %s has open %s server, cleaning up ...",
                    server_map.first.c_str(), server.first.c_str());
      }
    }
    servers_.clear();
  }
  if(results_.size() > 0) {
    RCLCPP_WARN(*logger_, "Found %li result(s), cleaning up ...", results_.size());
    results_.clear();
  }
  if(feedbacks_.size() > 0) {
    RCLCPP_WARN(*logger_, "Found %li feedback(s), cleaning up ...", feedbacks_.size());
    feedbacks_.clear();
  }
  if(const_feedbacks_.size() > 0) {
    RCLCPP_WARN(*logger_, "Found %li const feedback(s), cleaning up ...", const_feedbacks_.size());
  }
  if(goals_.size() > 0) {
    RCLCPP_WARN(*logger_, "Found %li goal(s), cleaning up ...", goals_.size());
    goals_.clear();
  }
  if(client_goal_handles_.size() > 0) {
    RCLCPP_WARN(*logger_, "Found %li client_goal_handle(s), cleaning up ...", client_goal_handles_.size());
    client_goal_handles_.clear();
  }
  if(server_goal_handles_.size() > 0) {
    RCLCPP_WARN(*logger_, "Found %li server_goal_handle(s), cleaning up ...", server_goal_handles_.size());
    server_goal_handles_.clear();
  }
}


void {{name_camel}}::initialize() {
   logger_ = std::make_unique<rclcpp::Logger>(rclcpp::get_logger(plugin_name_));
   auto node = parent_.lock();
   cb_group_ = node->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
}

bool {{name_camel}}::clips_env_destroyed(std::shared_ptr<clips::Environment> &env) {

  RCLCPP_DEBUG(*logger_,
              "Destroying clips context!");
  for(const auto& fun : function_names_) {
     clips::RemoveUDF(env.get(), fun.c_str());
  }
  clips::Deftemplate *curr_tmpl = clips::FindDeftemplate(env.get(), "{{name_kebab}}-client");
  if(curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_,
              "{{name_kebab}}-client can not be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "{{name_kebab}}-server");
  if(curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_,
              "{{name_kebab}}-server can not be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "{{name_kebab}}-goal-response");
  if(curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_,
              "{{name_kebab}}-goal-response can not be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "{{name_kebab}}-goal-feedback");
  if(curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_,
              "{{name_kebab}}-goal-feedback can not be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "{{name_kebab}}-wrapped-result");
  if(curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_,
              "{{name_kebab}}-wrapped-result can not be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "{{name_kebab}}-accepted-goal");
  if(curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_,
              "{{name_kebab}}-accepted-goal can not be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "{{name_kebab}}-cancel-goal-response");
  if(curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_,
              "{{name_kebab}}-cancel-goal-response can not be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "{{name_kebab}}-cancel-goal-entry");
  if(curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_,
              "{{name_kebab}}-cancel-goal-entry can not be undefined");
  }
  return true;
}

bool {{name_camel}}::clips_env_init(std::shared_ptr<clips::Environment> &env) {
  RCLCPP_INFO(*logger_,
              "Initializing context for plugin %s",
              plugin_name_.c_str());

{% set template_part = "registration" %}
{% set template_type = "Goal" %}
{% include 'get_field.jinja.cpp' with context %}
{% include 'set_field.jinja.cpp' with context %}
{% include 'create.jinja.cpp' with context %}
{% include 'destroy.jinja.cpp' with context %}
{% set template_type = "Feedback" %}
{% include 'get_field.jinja.cpp' with context %}
{% include 'set_field.jinja.cpp' with context %}
{% include 'create.jinja.cpp' with context %}
{% include 'destroy.jinja.cpp' with context %}
{% set template_type = "Result" %}
{% include 'get_field.jinja.cpp' with context %}
{% include 'set_field.jinja.cpp' with context %}
{% include 'create.jinja.cpp' with context %}
{% include 'destroy.jinja.cpp' with context %}
  std::string fun_name;

  fun_name = "{{name_kebab}}-create-server";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";sy",
    [](clips::Environment *env, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue server_name;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &server_name);

      instance->create_new_server(env, server_name.lexemeValue->contents);
    },
    "create_new_server", this);

  fun_name = "{{name_kebab}}-destroy-server";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";sy",
    [](clips::Environment *env, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue server_name;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &server_name);

      instance->destroy_server(env, server_name.lexemeValue->contents);
    },
    "destroy_server", this);

  fun_name = "{{name_kebab}}-create-client";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";s",
    [](clips::Environment *env, clips::UDFContext *udfc, clips::UDFValue * /*out*/) {
        auto *instance = static_cast<{{name_camel}} *>(udfc->context);
        clips::UDFValue server_name;

        using namespace clips;
        clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &server_name);
        instance->create_new_client(env,server_name.lexemeValue->contents);
    },
    "create_new_client", this);

  fun_name = "{{name_kebab}}-destroy-client";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";s",
    [](clips::Environment *env, clips::UDFContext *udfc, clips::UDFValue * /*out*/) {
        auto *instance = static_cast<{{name_camel}} *>(udfc->context);
        clips::UDFValue server_name;

        using namespace clips;
        clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &server_name);
        instance->destroy_client(env, server_name.lexemeValue->contents);
    },
    "destroy_client", this);

  fun_name = "{{name_kebab}}-send-goal";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";e;sy",
    [](clips::Environment *env, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue goal, server_name;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &goal);
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &server_name);

      instance->send_goal(env, static_cast<{{message_type}}::Goal*>(goal.externalAddressValue->contents), server_name.lexemeValue->contents);
    },
    "send_goal", this);

  fun_name = "{{name_kebab}}-server-goal-handle-abort";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";e;e",
    [](clips::Environment * /*env*/, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue goal_handle, result;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &goal_handle);
      clips::UDFNthArgument(udfc, 2, EXTERNAL_ADDRESS_BIT, &result);

       try {
        instance->server_goal_handle_abort(goal_handle.externalAddressValue->contents, result.externalAddressValue->contents, udfc);
       } catch (std::out_of_range &e) {
         RCLCPP_ERROR(*(instance->logger_), "Unknown goal handle or result pointer %s", e.what());
       }
    },
    "server_goal_handle_abort", this);

  fun_name = "{{name_kebab}}-server-goal-handle-succeed";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";e;e",
    [](clips::Environment * /*env*/, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue goal_handle, result;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &goal_handle);
      clips::UDFNthArgument(udfc, 2, EXTERNAL_ADDRESS_BIT, &result);

       try {
        instance->server_goal_handle_succeed(goal_handle.externalAddressValue->contents, result.externalAddressValue->contents, udfc);
       } catch (std::out_of_range &e) {
         RCLCPP_ERROR(*(instance->logger_), "Unknown goal handle or result pointer %s", e.what());
       }
    },
    "server_goal_handle_succeed", this);

  fun_name = "{{name_kebab}}-server-goal-handle-canceled";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";e;e",
    [](clips::Environment * /*env*/, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue goal_handle, result;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &goal_handle);
      clips::UDFNthArgument(udfc, 2, EXTERNAL_ADDRESS_BIT, &result);

       try {
        instance->server_goal_handle_canceled(goal_handle.externalAddressValue->contents, result.externalAddressValue->contents, udfc);
       } catch (std::out_of_range &e) {
         RCLCPP_ERROR(*(instance->logger_), "Unknown goal handle or result pointer %s", e.what());
       }
    },
    "server_goal_handle_canceled", this);

  fun_name = "{{name_kebab}}-server-goal-handle-get-goal";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "e", 1, 1, ";e",
    [](clips::Environment *env, clips::UDFContext *udfc,
       clips::UDFValue *out) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue goal_handle;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &goal_handle);
       try {
        *out = instance->server_goal_handle_get_goal(env, goal_handle.externalAddressValue->contents, udfc);
       } catch (std::out_of_range &e) {
         RCLCPP_ERROR(*(instance->logger_), "Unknown goal handle %s", e.what());
       }
    },
    "server_goal_handle_get_goal", this);

  fun_name = "{{name_kebab}}-server-goal-handle-get-goal-id";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "m", 1, 1, ";e",
    [](clips::Environment *env, clips::UDFContext *udfc,
       clips::UDFValue *out) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue goal_handle;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &goal_handle);
       try {
         *out = instance->server_goal_handle_get_goal_id(env, goal_handle.externalAddressValue->contents, udfc);
       } catch (std::out_of_range &e) {
         RCLCPP_ERROR(*(instance->logger_), "Unknown goal handle %s", e.what());
       }
    },
    "server_goal_handle_get_goal_id", this);

{% set template_type = "ServerGoalHandle" %}
{% set template_call_fun = "IsCanceling" %}
{% set template_ret_type = "Bool" %}
{% include 'ret_fun.jinja.cpp' with context %}
{% set template_call_fun = "IsActive" %}
{% include 'ret_fun.jinja.cpp' with context %}
{% set template_call_fun = "IsExecuting" %}
{% include 'ret_fun.jinja.cpp' with context %}
{% set template_call_fun = "Execute" %}
{% include 'void_fun.jinja.cpp' with context %}

{% set template_type = "ClientGoalHandle" %}
{% set template_call_fun = "IsFeedbackAware" %}
{% include 'ret_fun.jinja.cpp' with context %}
{% set template_call_fun = "IsResultAware" %}
{% include 'ret_fun.jinja.cpp' with context %}
{% set template_call_fun = "GetStatus" %}
{% set template_ret_type = "Integer" %}
{% include 'ret_fun.jinja.cpp' with context %}

  fun_name = "{{name_kebab}}-server-goal-handle-publish-feedback";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";e;e",
    [](clips::Environment * /*env*/, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue goal_handle, feedback;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &goal_handle);
      clips::UDFNthArgument(udfc, 2, EXTERNAL_ADDRESS_BIT, &feedback);
       try {
        instance->server_goal_handle_publish_feedback(goal_handle.externalAddressValue->contents, feedback.externalAddressValue->contents, udfc);
       } catch (std::out_of_range &e) {
         RCLCPP_ERROR(*(instance->logger_), "Unknown goal handle or feedback %s", e.what());
       }
    },
    "server_goal_handle_publish_feedback", this);

  fun_name = "{{name_kebab}}-client-goal-handle-get-goal-id";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "m", 1, 1, ";e",
    [](clips::Environment *env, clips::UDFContext *udfc,
       clips::UDFValue *out) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue goal_handle;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &goal_handle);
       try {
        *out = instance->client_goal_handle_get_goal_id(env, goal_handle.externalAddressValue->contents, udfc);
       } catch (std::out_of_range &e) {
         RCLCPP_ERROR(*(instance->logger_), "Unknown goal handle %s", e.what());
       }
    },
    "client_goal_handle_get_goal_id", this);

  fun_name = "{{name_kebab}}-client-goal-handle-get-goal-stamp";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "d", 1, 1, ";e",
    [](clips::Environment *env, clips::UDFContext *udfc,
       clips::UDFValue *out) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue goal_handle;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &goal_handle);
       try {
        *out = instance->client_goal_handle_get_goal_stamp(env, goal_handle.externalAddressValue->contents, udfc);
       } catch (std::out_of_range &e) {
         RCLCPP_ERROR(*(instance->logger_), "Unknown goal handle %s", e.what());
       }
    },
    "client_goal_handle_get_goal_stamp", this);

  fun_name = "{{name_kebab}}-client-goal-handle-destroy";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "d", 1, 1, ";e",
    [](clips::Environment */*env*/, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue data_ptr;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &data_ptr);

      instance->client_goal_handle_destroy(data_ptr.externalAddressValue->contents);
    },
    "client_goal_handle_destroy", this);

  fun_name = "{{name_kebab}}-server-goal-handle-destroy";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "d", 1, 1, ";e",
    [](clips::Environment */*env*/, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue data_ptr;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &data_ptr);

      instance->server_goal_handle_destroy(data_ptr.externalAddressValue->contents);
    },
    "server_goal_handle_destroy", this);

    fun_name = "{{name_kebab}}-client-cancel-goal";
    function_names_.insert(fun_name);
    clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";sy;e",
    [](clips::Environment *env, clips::UDFContext *udfc,
    clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue goal_handle_val, server_name_val;
      using namespace clips;

      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &server_name_val);
      clips::UDFNthArgument(udfc, 2, EXTERNAL_ADDRESS_BIT, &goal_handle_val);

      std::string server_name = server_name_val.lexemeValue->contents;

      std::scoped_lock lock{instance->map_mtx_};

      auto gh_it = instance->client_goal_handles_.find(
          goal_handle_val.externalAddressValue->contents);
      if (gh_it == instance->client_goal_handles_.end()) {
        RCLCPP_ERROR(*(instance->logger_),
                     "client-cancel-goal: Unknown goal handle");
        clips::UDFThrowError(udfc);
        return;
      }

      auto goal_handle = gh_it->second;
      auto context = CLIPSEnvContext::get_context(env);
      std::string env_name = context->env_name_;

      instance->clients_[env_name][server_name]
          ->async_cancel_goal(goal_handle,
            [env,instance, server_name](auto response) {
              instance->process_cancel_response(env,server_name ,response);
            });
    }, "client_cancel_goal", this);

    fun_name = "{{name_kebab}}-client-cancel-all-goals";
    function_names_.insert(fun_name);
    clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";sy",
    [](clips::Environment *env, clips::UDFContext *udfc,
    clips::UDFValue * /*out*/) {
        auto *instance = static_cast<{{name_camel}} *>(udfc->context);
        clips::UDFValue server_name_val;
        using namespace clips;

        clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &server_name_val);
        std::string server_name = server_name_val.lexemeValue->contents;

        auto context = CLIPSEnvContext::get_context(env);
        std::string env_name = context->env_name_;

        instance->clients_[env_name][server_name]
            ->async_cancel_all_goals(
              [env,instance, server_name](auto response) {
              instance->process_cancel_response(env,server_name ,response);
              });
        },
    "client_cancel_all_goals", this);

    fun_name = "{{name_kebab}}-client-cancel-goals-before";
    function_names_.insert(fun_name);
    clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";sy;f",
    [](clips::Environment *env, clips::UDFContext *udfc,
    clips::UDFValue * /*out*/) {
        auto *instance = static_cast<{{name_camel}} *>(udfc->context);
        clips::UDFValue server_name_val, stamp_val;
        using namespace clips;

        clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &server_name_val);
        clips::UDFNthArgument(udfc, 2, FLOAT_BIT, &stamp_val);

        std::string server_name = server_name_val.lexemeValue->contents;
        double stamp_seconds = stamp_val.floatValue->contents;

        rclcpp::Time stamp(
            static_cast<int64_t>(stamp_seconds * 1e9));

        auto context = CLIPSEnvContext::get_context(env);
        std::string env_name = context->env_name_;

        instance->clients_[env_name][server_name]
            ->async_cancel_goals_before(
              stamp,
              [env,instance, server_name](auto response) {
              instance->process_cancel_response(env,server_name ,response);
              });
    },
    "client_cancel_goals_before", this);

//     fun_name = "{{name_kebab}}-client-goal-handle-stop-callbacks";
//     function_names_.insert(fun_name);
//
//     clips::AddUDF(
//       env.get(), fun_name.c_str(), "v", 2, 2, ";sy;e",
//       [](clips::Environment *env,
//          clips::UDFContext *udfc,
//          clips::UDFValue * /*out*/)
//       {
//         auto *instance = static_cast<{{name_camel}} *>(udfc->context);
//         using namespace clips;
//
//         clips::UDFValue goal_handle_val, server_name_val;
//
//         clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &server_name_val);
//         clips::UDFNthArgument(udfc, 2, EXTERNAL_ADDRESS_BIT, &goal_handle_val);
//
//         std::string server_name = server_name_val.lexemeValue->contents;
//
//         std::scoped_lock lock{instance->map_mtx_};
//
//         // Lookup goal handle
//         auto gh_it = instance->client_goal_handles_.find(
//             goal_handle_val.externalAddressValue->contents);
//
//         if (gh_it == instance->client_goal_handles_.end()) {
//           RCLCPP_ERROR(*(instance->logger_),
//                        "client-goal-handle-stop-callbacks: Unknown goal handle");
//           clips::UDFThrowError(udfc);
//           return;
//         }
//
//         auto goal_handle = gh_it->second;
//
//         // Determine environment
//         auto context = CLIPSEnvContext::get_context(env);
//         std::string env_name = context->env_name_;
//
//         // Lookup client
//         auto client_it =
//             instance->clients_[env_name].find(server_name);
//
//         if (client_it == instance->clients_[env_name].end()) {
//           RCLCPP_ERROR(*(instance->logger_),
//                        "client-goal-handle-stop-callbacks: Unknown server '%s'",
//                        server_name.c_str());
//           clips::UDFThrowError(udfc);
//           return;
//         }
//
//         // Stop callbacks using UUID
//         client_it->second->stop_callbacks(
//             goal_handle->get_goal_id());
//       },
//       "client_goal_handle_stop_callbacks",
//       this);

  // add fact templates
  clips::Build(env.get(),"(deftemplate {{name_kebab}}-client \
            (slot server (type STRING)))");
  clips::Build(env.get(),"(deftemplate {{name_kebab}}-server \
            (slot name (type STRING)))");
  clips::Build(env.get(),"(deftemplate {{name_kebab}}-goal-response \
            (slot server (type STRING) ) \
            (slot client-goal-handle-ptr (type EXTERNAL-ADDRESS)) \
            )");
  clips::Build(env.get(),"(deftemplate {{name_kebab}}-goal-feedback \
            (slot server (type STRING) ) \
            (slot client-goal-handle-ptr (type EXTERNAL-ADDRESS)) \
            (slot feedback-ptr (type EXTERNAL-ADDRESS)) \
            )");
  clips::Build(env.get(),"(deftemplate {{name_kebab}}-wrapped-result \
            (slot server (type STRING) ) \
            (slot goal-id (type STRING)) \
            (slot code (type SYMBOL)) \
            (slot result-ptr (type EXTERNAL-ADDRESS)) \
            )");
  clips::Build(env.get(),"(deftemplate {{name_kebab}}-accepted-goal \
            (slot server (type STRING) ) \
            (slot server-goal-handle-ptr (type EXTERNAL-ADDRESS)) \
            )");
  clips::Build(env.get(),"(deftemplate {{name_kebab}}-cancel-goal-response \
            (slot server (type STRING) ) \
            (slot return-code (type SYMBOL)) \
            (slot num-goals (type INTEGER)) \
            )");
  clips::Build(env.get(),"(deftemplate {{name_kebab}}-cancel-goal-entry \
            (slot server (type STRING) ) \
            (slot goal-id (type STRING)) \
            (slot stamp (type FLOAT)) \
            )");
  return true;

  }

{% set template_part = "definition" %}
{% set template_slots = goal_slots %}
{% set template_type = "Goal" %}
{% include 'set_field.jinja.cpp' with context %}
{% include 'get_field.jinja.cpp' with context %}
{% include 'create.jinja.cpp' with context %}
{% include 'destroy.jinja.cpp' with context %}

{% set template_slots = result_slots %}
{% set template_type = "Result" %}
{% include 'set_field.jinja.cpp' with context %}
{% include 'get_field.jinja.cpp' with context %}
{% include 'create.jinja.cpp' with context %}
{% include 'destroy.jinja.cpp' with context %}

{% set template_slots = feedback_slots %}
{% set template_type = "Feedback" %}
{% include 'set_field.jinja.cpp' with context %}
{% set template_fun_name = "get_field_feedback" %}
{% include 'get_field.jinja.cpp' with context %}
{% include 'create.jinja.cpp' with context %}


void {{name_camel}}::process_cancel_response(
    clips::Environment *env, const std::string &server_name,
    const action_msgs::srv::CancelGoal::Response::SharedPtr &response)
{
  task_queue_.push([this, env, server_name, response]() {

    auto context = CLIPSEnvContext::get_context(env);
    std::lock_guard<std::mutex> guard(context->env_mtx_);

    // ---- Map return code ----
    std::string return_code_str = "ERROR_UNKNOWN";

    switch (response->return_code) {
      case action_msgs::srv::CancelGoal::Response::ERROR_NONE:
        return_code_str = "ERROR_NONE";
        break;
      case action_msgs::srv::CancelGoal::Response::ERROR_REJECTED:
        return_code_str = "ERROR_REJECTED";
        break;
      case action_msgs::srv::CancelGoal::Response::ERROR_UNKNOWN_GOAL_ID:
        return_code_str = "ERROR_UNKNOWN_GOAL_ID";
        break;
      case action_msgs::srv::CancelGoal::Response::ERROR_GOAL_TERMINATED:
        return_code_str = "ERROR_GOAL_TERMINATED";
        break;
    }

    // ---- Assert global response fact ----
    {
      clips::FactBuilder *fb =
          clips::CreateFactBuilder(env,
              "{{name_kebab}}-cancel-goal-response");

      clips::FBPutSlotString(fb, "server", server_name.c_str());
      clips::FBPutSlotSymbol(fb, "return-code",
                             return_code_str.c_str());
      clips::FBPutSlotInteger(fb, "num-goals",
                              response->goals_canceling.size());

      clips::FBAssert(fb);
      clips::FBDispose(fb);
    }

    // ---- Assert one fact per canceled goal ----
    for (const auto &goal_info : response->goals_canceling) {

      clips::FactBuilder *fb =
          clips::CreateFactBuilder(env,
              "{{name_kebab}}-cancel-goal-entry");

      clips::FBPutSlotString(fb, "server", server_name.c_str());

      std::string uuid_str =
          rclcpp_action::to_string(goal_info.goal_id.uuid);

      clips::FBPutSlotString(fb, "goal-id", uuid_str.c_str());
      double stamp =
        static_cast<double>(goal_info.stamp.sec) +
        static_cast<double>(goal_info.stamp.nanosec) * 1e-9;

      clips::FBPutSlotFloat(fb, "stamp",
                              stamp);

      clips::FBAssert(fb);
      clips::FBDispose(fb);
    }

  });

  cv_.notify_one();
}


void {{name_camel}}::feedback_destroy({{message_type}}::Feedback *fb) {
  std::scoped_lock map_lock{map_mtx_};
  auto it = feedbacks_.find(fb);
  if (it != feedbacks_.end()) {
      feedbacks_.erase(it);
  }
  auto const_it = const_feedbacks_.find(fb);
  if (const_it != const_feedbacks_.end()) {
      const_feedbacks_.erase(const_it);
  }
}

void {{name_camel}}::send_goal(clips::Environment *env, {{message_type}}::Goal *goal, const std::string &server_name) {
  using namespace std::chrono_literals;
   // Handle the request asynchronously to not block clips engine potentially endlessly
  std::thread([this, env, goal, server_name]() {
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;
  bool print_warning = true;
  while (!stop_flag_ && !clients_[env_name][server_name]->wait_for_action_server(1s)) {
    if (stop_flag_ || !rclcpp::ok()) {
      RCLCPP_ERROR(*logger_, "Interrupted while waiting for the server. Exiting.");
      return;
    }
    if(print_warning) {
      RCLCPP_WARN(*logger_, "server %s not available, start waiting", server_name.c_str());
      print_warning = false;
    }
    RCLCPP_DEBUG(*logger_, "server %s not available, waiting again...", server_name.c_str());
  }
  if(!print_warning) {
      RCLCPP_INFO(*logger_, "server %s is finally reachable", server_name.c_str());
  }
  std::lock_guard<std::mutex> guard(context->env_mtx_);
  if(stop_flag_) {
    RCLCPP_DEBUG(*logger_, "Shutdown during async call.");
    return;
  }
   auto send_goal_options = rclcpp_action::Client<{{message_type}}>::SendGoalOptions();
  send_goal_options.goal_response_callback = [this, context, env, server_name](const std::shared_ptr<rclcpp_action::ClientGoalHandle<{{message_type}}>> &goal_handle) {
    std::scoped_lock map_lock{map_mtx_};
    task_queue_.push([this, context, env, server_name, goal_handle]() {
      std::lock_guard<std::mutex> guard(context->env_mtx_);
      {
        std::scoped_lock map_lock{map_mtx_};
        client_goal_handles_.try_emplace(goal_handle.get(), goal_handle);
      }
      clips::FactBuilder *fact_builder = clips::CreateFactBuilder(env, "{{name_kebab}}-goal-response");
      clips::FBPutSlotString(fact_builder,"server",server_name.c_str());
      clips::FBPutSlotCLIPSExternalAddress(fact_builder,"client-goal-handle-ptr", clips::CreateCExternalAddress(env, goal_handle.get()));
      clips::FBAssert(fact_builder);
      clips::FBDispose(fact_builder);
    });
    cv_.notify_one();  // Notify the worker thread
  };

  send_goal_options.feedback_callback = [this, context, env, server_name](
      std::shared_ptr<rclcpp_action::ClientGoalHandle<{{message_type}}>> goal_handle,
      const std::shared_ptr<const {{message_type}}::Feedback> feedback) {
    // this indirection is necessary as the feedback callback is called while an internal lock is acquired.
    // The same lock is acquired in calls lige get_status().
    // This can cause a deadlock if get_status() is called from within clips right after a callback is received.
    // clips lock is still held, hence callback can't proceed, but internal lock is already ackquired when this callback is invoked.
    std::lock_guard<std::mutex> lock(queue_mutex_);
    task_queue_.push([this, context, env, server_name, goal_handle, feedback]() {
      // Enqueue the task to avoid directly locking the handle_mutex_ in a callback
      std::lock_guard<std::mutex> guard(context->env_mtx_);
      {
        std::scoped_lock map_lock{map_mtx_};
        const_feedbacks_.try_emplace(const_cast<void *>(static_cast<const void*>(feedback.get())), feedback);
        client_goal_handles_.try_emplace(goal_handle.get(), goal_handle);
      }
      clips::FactBuilder *fact_builder = clips::CreateFactBuilder(env, "{{name_kebab}}-goal-feedback");
      clips::FBPutSlotString(fact_builder,"server",server_name.c_str());
      clips::FBPutSlotCLIPSExternalAddress(fact_builder,"client-goal-handle-ptr", clips::CreateCExternalAddress(env, goal_handle.get()));
      clips::FBPutSlotCLIPSExternalAddress(fact_builder,"feedback-ptr", clips::CreateCExternalAddress(env, const_cast<void *>(static_cast<const void *>(feedback.get()))));
      clips::FBAssert(fact_builder);
      clips::FBDispose(fact_builder);
    });
    cv_.notify_one();  // Notify the worker thread
  };

  send_goal_options.result_callback = [this, context, env, server_name](const rclcpp_action::ClientGoalHandle<{{message_type}}>::WrappedResult &wrapped_result) {
    task_queue_.push([this, context, env, server_name, wrapped_result]() {
      std::lock_guard<std::mutex> guard(context->env_mtx_);
      {
        std::scoped_lock map_lock{map_mtx_};
        results_.try_emplace(wrapped_result.result.get(), wrapped_result.result);
      }
      clips::FactBuilder *fact_builder = clips::CreateFactBuilder(env, "{{name_kebab}}-wrapped-result");
      clips::FBPutSlotString(fact_builder,"server",server_name.c_str());
      clips::FBPutSlotString(fact_builder,"goal-id",rclcpp_action::to_string(wrapped_result.goal_id).c_str());
      std::string code_str = "UNKNOWN";
      switch(wrapped_result.code) {
        case rclcpp_action::ResultCode::UNKNOWN: break;
        case rclcpp_action::ResultCode::SUCCEEDED: code_str = "SUCCEEDED"; break;
        case rclcpp_action::ResultCode::CANCELED: code_str = "CANCELED"; break;
        case rclcpp_action::ResultCode::ABORTED: code_str = "ABORTED"; break;
      }
      clips::FBPutSlotSymbol(fact_builder,"code", code_str.c_str());
      clips::FBPutSlotCLIPSExternalAddress(fact_builder,"result-ptr", clips::CreateCExternalAddress(env, wrapped_result.result.get()));
      clips::FBAssert(fact_builder);
      clips::FBDispose(fact_builder);
    });
    cv_.notify_one();  // Notify the worker thread
  };

  clients_[env_name][server_name]->async_send_goal(*goal, send_goal_options);
  }).detach();
}

void {{name_camel}}::create_new_server(clips::Environment *env, const std::string &server_name) {

  auto handle_goal = [this, env, server_name](const rclcpp_action::GoalUUID & uuid,
      std::shared_ptr<const {{message_type}}::Goal> goal) {
    auto context = CLIPSEnvContext::get_context(env);
    std::string env_name = context->env_name_;
    std::lock_guard<std::mutex> guard(context->env_mtx_);
    clips::Deffunction *dec_fun = clips::FindDeffunction(env,"{{name_kebab}}-handle-goal-callback");
    if(!dec_fun) {
      RCLCPP_DEBUG(*logger_, "Accepting goal per default");
      return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }
    clips::FunctionCallBuilder *fcb = clips::CreateFunctionCallBuilder(env,3);
    clips::FCBAppendString(fcb, server_name.c_str());
    clips::FCBAppendCLIPSExternalAddress(fcb, clips::CreateCExternalAddress(env, const_cast<void*>(static_cast<const void *>(goal.get()))));
    clips::FCBAppendString(fcb, rclcpp_action::to_string(uuid).c_str());
    clips::CLIPSValue ret;
    // no need to delete goal pointer manually, it is not copied
    clips::FCBCall(fcb,"{{name_kebab}}-handle-goal-callback",&ret);
    clips::FCBDispose(fcb);

    int value = ret.integerValue->contents;

    if (value == 3) {
      return rclcpp_action::GoalResponse::ACCEPT_AND_DEFER;
    }
    if (value == 2) {
      return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }
    if (value == 1) {
      return rclcpp_action::GoalResponse::REJECT;
    }

    RCLCPP_ERROR(*logger_, "Invalid goal response from CLIPS: %d", value);
    return rclcpp_action::GoalResponse::REJECT;
  };
  auto handle_cancel = [this, env, server_name](const std::shared_ptr<rclcpp_action::ServerGoalHandle<{{message_type}}>> goal_handle) {
    auto context = CLIPSEnvContext::get_context(env);
    std::string env_name = context->env_name_;
    std::lock_guard<std::mutex> guard(context->env_mtx_);
    clips::Deffunction *dec_fun = clips::FindDeffunction(env,"{{name_kebab}}-cancel-goal-callback");
    if(!dec_fun) {
      RCLCPP_DEBUG(*logger_, "Accepting goal cancellation per default");
      return rclcpp_action::CancelResponse::ACCEPT;
    }
    // store the goal handle to ensure it is not cleaned up implicitly
    // TODO: Maybe not needed?
    {
      std::scoped_lock map_lock{map_mtx_};
      server_goal_handles_.try_emplace(goal_handle.get(),goal_handle);
    }
    clips::FunctionCallBuilder *fcb = clips::CreateFunctionCallBuilder(env,3);
    clips::FCBAppendString(fcb, server_name.c_str());
    clips::FCBAppendCLIPSExternalAddress(fcb, clips::CreateCExternalAddress(env, const_cast<void *>(static_cast<const void *>(goal_handle->get_goal().get()))));
    clips::FCBAppendCLIPSExternalAddress(fcb, clips::CreateCExternalAddress(env, goal_handle.get()));
    clips::CLIPSValue ret;
    clips::FCBCall(fcb,"{{name_kebab}}-cancel-goal-callback",&ret);
    clips::FCBDispose(fcb);
    int value = ret.integerValue->contents;

    if (value == 2) {
      return rclcpp_action::CancelResponse::ACCEPT;
    }
    if (value == 1) {
      return rclcpp_action::CancelResponse::REJECT;
    }

    RCLCPP_ERROR(*logger_, "Invalid cancel response from CLIPS: %d", value);
    return rclcpp_action::CancelResponse::REJECT;
    };

  auto handle_accepted = [this, env, server_name](const std::shared_ptr<rclcpp_action::ServerGoalHandle<{{message_type}}>> goal_handle) {
    auto context = CLIPSEnvContext::get_context(env);
    std::string env_name = context->env_name_;
    std::lock_guard<std::mutex> guard(context->env_mtx_);
    // store the goal handle to ensure it is not cleaned up implicitly
    {
      std::scoped_lock map_lock{map_mtx_};
      server_goal_handles_.try_emplace(goal_handle.get(),goal_handle);
    }
    clips::FactBuilder *fact_builder = clips::CreateFactBuilder(env, "{{name_kebab}}-accepted-goal");
    clips::FBPutSlotString(fact_builder,"server",server_name.c_str());
    clips::FBPutSlotCLIPSExternalAddress(fact_builder,"server-goal-handle-ptr", clips::CreateCExternalAddress(env, goal_handle.get()));
    clips::FBAssert(fact_builder);
    clips::FBDispose(fact_builder);
  };
  auto context = CLIPSEnvContext::get_context(env);
  auto node = parent_.lock();
  std::string env_name = context->env_name_;
  {
    std::scoped_lock map_lock{map_mtx_};
    servers_[env_name][server_name] =
      rclcpp_action::create_server<{{message_type}}>(node, server_name, handle_goal, handle_cancel, handle_accepted);
  }
  clips::AssertString(env, ("({{name_kebab}}-server (name \"" + server_name + "\"))").c_str());
}

void {{name_camel}}::destroy_server(clips::Environment *env, const std::string &server_name) {
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;
  {
    std::scoped_lock map_lock{map_mtx_};
    auto outer_it = servers_.find(env_name);
    if (outer_it != servers_.end()) {
        // Check if server_name exists in the inner map
        auto& inner_map = outer_it->second;
        auto inner_it = inner_map.find(server_name);
        if (inner_it != inner_map.end()) {
            // Remove the server_name entry from the inner map
            inner_map.erase(inner_it);
        } else {
            RCLCPP_WARN(*logger_, "Service %s not found in environment %s", server_name.c_str(), env_name.c_str());
        }
    } else {
        RCLCPP_WARN(*logger_, "Environment %s not found", env_name.c_str());
    }
  }

  clips::Eval(env, ("(do-for-all-facts ((?f {{name_kebab}}-server)) (eq (str-cat ?f:name) (str-cat " + server_name + "))  (retract ?f))").c_str(), NULL);
}

void {{name_camel}}::create_new_client(clips::Environment *env,
    const std::string &server_name) {
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;

  map_mtx_.lock();
  auto it = clients_[env_name].find(server_name);

  if (it != clients_[env_name].end()) {
    RCLCPP_WARN(*logger_,
                "There already exists a client for server %s", server_name.c_str());
    map_mtx_.unlock();
  } else {
    RCLCPP_DEBUG(*logger_,
                "Creating client for server %s", server_name.c_str());
    auto node = parent_.lock();
    clients_[env_name][server_name] =
        rclcpp_action::create_client<{{message_type}}>(node, server_name);
    map_mtx_.unlock();
    clips::AssertString(env, ("({{name_kebab}}-client (server \"" + server_name + "\"))").c_str());
  }
}

void {{name_camel}}::destroy_client(clips::Environment *env,
    const std::string &server_name) {
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;
  {
    std::scoped_lock map_lock{map_mtx_};
    auto it = clients_[env_name].find(server_name);

    if (it != clients_[env_name].end()) {
      RCLCPP_DEBUG(*logger_,
                  "Destroying client for server %s", server_name.c_str());
      clients_[env_name].erase(server_name);
    }
  }

  clips::Eval(env, ("(do-for-all-facts ((?f {{name_kebab}}-client)) (eq (str-cat ?f:server) (str-cat " + server_name + "))  (retract ?f))").c_str(), NULL);
}


void {{name_camel}}::server_goal_handle_abort(void *goal_handle_raw, void *result_raw, clips::UDFContext *udfc) {
  std::scoped_lock map_lock{map_mtx_};
  auto goal_handle = server_goal_handles_.at(goal_handle_raw);
  if(!goal_handle) {
    RCLCPP_ERROR(*logger_, "server-goal-handle-abort: Invalid pointer to ServerGoalHandle");
    clips::UDFThrowError(udfc);
    return;
  }
  auto result = results_.at(result_raw);
  if(!result) {
    RCLCPP_ERROR(*logger_, "server-goal-handle-abort: Invalid pointer to Result");
    clips::UDFThrowError(udfc);
    return;
  }
  goal_handle->abort(result);
}

void {{name_camel}}::server_goal_handle_succeed(void *goal_handle_raw, void *result_raw, clips::UDFContext *udfc) {
  std::scoped_lock map_lock{map_mtx_};
  auto goal_handle = server_goal_handles_.at(goal_handle_raw);
  if(!goal_handle) {
    RCLCPP_ERROR(*logger_, "server-goal-handle-succeed: Invalid pointer to ServerGoalHandle");
    clips::UDFThrowError(udfc);
    return;
  }
  auto result = results_.at(result_raw);
  if(!result) {
    RCLCPP_ERROR(*logger_, "server-goal-handle-succeed: Invalid pointer to Result");
    clips::UDFThrowError(udfc);
    return;
  }
  goal_handle->succeed(result);
}

void {{name_camel}}::server_goal_handle_canceled(void *goal_handle_raw, void *result_raw, clips::UDFContext *udfc) {
  std::scoped_lock map_lock{map_mtx_};
  auto goal_handle = server_goal_handles_.at(goal_handle_raw);
  if(!goal_handle) {
    RCLCPP_ERROR(*logger_, "server-goal-handle-canceled: Invalid pointer to ServerGoalHandle");
    clips::UDFThrowError(udfc);
    return;
  }
  auto result = results_.at(result_raw);
  if(!result) {
    RCLCPP_ERROR(*logger_, "server-goal-handle-canceled: Invalid pointer to Result");
    clips::UDFThrowError(udfc);
    return;
  }
  goal_handle->canceled(result);
}

clips::UDFValue {{name_camel}}::server_goal_handle_get_goal(clips::Environment *env, void *goal_handle_raw, clips::UDFContext *udfc) {
  std::scoped_lock map_lock{map_mtx_};
  clips::UDFValue res;
  auto goal_handle = server_goal_handles_.at(goal_handle_raw);
  if(!goal_handle) {
    RCLCPP_ERROR(*logger_, "server-goal-handle-get-goal: Invalid pointer to ServerGoalHandle");
    clips::UDFThrowError(udfc);
    return res;
  }
  const std::shared_ptr<const {{message_type}}::Goal> goal = goal_handle->get_goal();
  res.externalAddressValue = clips::CreateCExternalAddress(env, (void *) goal.get());
  return res;
}

clips::UDFValue {{name_camel}}::server_goal_handle_get_goal_id(clips::Environment *env, void *goal_handle_raw, clips::UDFContext *udfc) {
  std::scoped_lock map_lock{map_mtx_};
  clips::UDFValue res;
  auto goal_handle = server_goal_handles_.at(goal_handle_raw);
  if(!goal_handle) {
    RCLCPP_ERROR(*logger_, "server-goal-handle-get-goal-id: Invalid pointer to ServerGoalHandle");
    clips::UDFThrowError(udfc);
    return res;
  }
  rclcpp_action::GoalUUID goal_id = goal_handle->get_goal_id();
  res.lexemeValue = clips::CreateString(env, rclcpp_action::to_string(goal_id).c_str());
  res.begin = 0;
  res.range = -1;
  return res;
}

{% set template_ret_type = "Bool" %}
{% set template_type = "ServerGoalHandle" %}
{% set template_call_fun = "is_canceling" %}
{% include 'ret_fun.jinja.cpp' with context %}
{% set template_call_fun = "IsActive" %}
{% include 'ret_fun.jinja.cpp' with context %}
{% set template_call_fun = "IsExecuting" %}
{% include 'ret_fun.jinja.cpp' with context %}
{% set template_call_fun = "Execute" %}
{% include 'void_fun.jinja.cpp' with context %}

{% set template_type = "ClientGoalHandle" %}
{% set template_call_fun = "IsFeedbackAware" %}
{% include 'ret_fun.jinja.cpp' with context %}
{% set template_call_fun = "IsResultAware" %}
{% include 'ret_fun.jinja.cpp' with context %}
{% set template_call_fun = "GetStatus" %}
{% set template_ret_type = "Integer" %}
{% include 'ret_fun.jinja.cpp' with context %}

void {{name_camel}}::server_goal_handle_publish_feedback(void *goal_handle_raw, void *feedback_raw, clips::UDFContext *udfc) {
  std::scoped_lock map_lock{map_mtx_};
  auto goal_handle = server_goal_handles_.at(goal_handle_raw);
  if(!goal_handle) {
    RCLCPP_ERROR(*logger_, "server-goal-handle-publish-feedback: Invalid pointer to ServerGoalHandle");
    clips::UDFThrowError(udfc);
    return;
  }
  auto it = const_feedbacks_.find(feedback_raw);
  if(it != const_feedbacks_.end()) {
    RCLCPP_ERROR(*logger_, "server-goal-handle-publish-feedback: Cannot publish const feedback");
    clips::UDFThrowError(udfc);
    return;
  }
  auto feedback = feedbacks_.at(feedback_raw);
  if(!feedback) {
    RCLCPP_ERROR(*logger_, "server-goal-handle-publish-feedback: Invalid pointer to Feedback");
    clips::UDFThrowError(udfc);
    return;
  }
  goal_handle->publish_feedback(feedbacks_.at(feedback_raw));
}

clips::UDFValue {{name_camel}}::client_goal_handle_get_goal_id(clips::Environment *env, void *goal_handle_raw, clips::UDFContext *udfc) {
  std::scoped_lock map_lock{map_mtx_};
  auto goal_handle = client_goal_handles_.at(goal_handle_raw);
  clips::UDFValue res;
  std::string uuid_str;
  if(goal_handle) {
    rclcpp_action::GoalUUID goal_id = goal_handle->get_goal_id();
	uuid_str = rclcpp_action::to_string(goal_id);
  } else {
    RCLCPP_ERROR(*logger_, "client-goal-handle-get-goal-id: Invalid pointer to ClientGoalHandle");
    clips::UDFThrowError(udfc);
    uuid_str = "";
  }
  res.lexemeValue = clips::CreateString(env, uuid_str.c_str());
  return res;
}

clips::UDFValue {{name_camel}}::client_goal_handle_get_goal_stamp(clips::Environment *env, void *goal_handle_raw, clips::UDFContext *udfc) {
  std::scoped_lock map_lock{map_mtx_};
  auto goal_handle = client_goal_handles_.at(goal_handle_raw);
  clips::UDFValue res;
  if(goal_handle) {
  res.floatValue = clips::CreateFloat(env, goal_handle->get_goal_stamp().seconds());
  } else {
     RCLCPP_ERROR(*logger_, "client-goal-handle-get-goal-id: Invalid pointer to ClientGoalHandle");
     res.floatValue = clips::CreateFloat(env, 0.0);
     clips::UDFThrowError(udfc);
  }
  return res;
}

void {{name_camel}}::client_goal_handle_destroy(void *g) {
  std::scoped_lock map_lock{map_mtx_};
  auto it = client_goal_handles_.find(g);
  if (it != client_goal_handles_.end()) {
      client_goal_handles_.erase(it);
  }
}

void {{name_camel}}::server_goal_handle_destroy(void *g) {
  std::scoped_lock map_lock{map_mtx_};
  auto it = server_goal_handles_.find(g);
  if (it != server_goal_handles_.end()) {
      server_goal_handles_.erase(it);
  }
}
} // namespace cx
PLUGINLIB_EXPORT_CLASS(cx::{{name_camel}}, cx::ClipsPlugin)
