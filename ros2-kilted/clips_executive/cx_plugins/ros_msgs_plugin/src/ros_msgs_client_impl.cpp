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

#ifdef HAVE_GENERIC_CLIENT
#include "cx_ros_msgs_plugin/ros_msgs_plugin.hpp"
#include "cx_utils/clips_env_context.hpp"
#include "rclcpp/create_generic_client.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/service_introspection.hpp"

namespace cx
{
void RosMsgsPlugin::finalize_generic_client_bindings()
{
  if (clients_.size() > 0) {
    for (const auto & client_map : clients_) {
      for (const auto & client : client_map.second) {
        RCLCPP_WARN(
          *logger_, "Environment %s has open %s clients, cleaning up ...", client_map.first.c_str(),
          client.first.c_str());
      }
    }
    clients_.clear();
  }
}

void RosMsgsPlugin::init_generic_client_bindings(std::shared_ptr<clips::Environment> env)
{
  std::string fun_name = "ros-msgs-create-client";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";sy;sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue service;
      clips::UDFValue type;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &service);
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &type);

      instance->create_new_client(env, service.lexemeValue->contents, type.lexemeValue->contents);
    },
    "create_new_client", this);

  fun_name = "ros-msgs-destroy-client";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue service;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &service);

      instance->destroy_client(env, service.lexemeValue->contents);
    },
    "destroy_client", this);

  fun_name = "ros-msgs-create-request";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "e", 1, 1, ";sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue type;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &type);
      *out = instance->create_request(env, type.lexemeValue->contents);
    },
    "create_request", this);

  fun_name = "ros-msgs-async-send-request";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "bl", 2, 2, ";e;sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue request, service_name;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &request);
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &service_name);

      *out = instance->send_request(
        env, request.externalAddressValue->contents, service_name.lexemeValue->contents);
    },
    "async-send-request", this);

  clips::Build(
    env.get(),
    R"((deftemplate ros-msgs-client
            (slot service (type STRING))
            (slot type (type STRING))))");
  clips::Build(
    env.get(),
    R"((deftemplate ros-msgs-response
            (slot service (type STRING))
            (slot request-id (type INTEGER))
            (slot msg-ptr (type EXTERNAL-ADDRESS))
            ))");
}

void RosMsgsPlugin::destroy_generic_client_bindings(std::shared_ptr<clips::Environment> env)
{
  clips::Deftemplate * curr_tmpl = clips::FindDeftemplate(env.get(), "ros-msgs-client");
  if (curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_, "ros-msgs-client cant be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "ros-msgs-response");
  if (curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_, "ros-msgs-response cant be undefined");
  }
}

void RosMsgsPlugin::create_new_client(
  clips::Environment * env, const std::string & service_name, const std::string & service_type)
{
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;

  auto node = parent_.lock();
  if (!node) {
    RCLCPP_ERROR(*logger_, "Invalid reference to parent node");
  }

  std::map<std::__cxx11::basic_string<char>, std::shared_ptr<rclcpp::GenericClient>>::iterator it;
  {
    map_mtx_.lock();
    it = clients_[env_name].find(service_name);
  }

  if (it != clients_[env_name].end()) {
    map_mtx_.unlock();
    RCLCPP_WARN(*logger_, "Already registered to client %s", service_name.c_str());
  } else {
    RCLCPP_DEBUG(*logger_, "Creating publisher to client %s", service_name.c_str());

    if (!libs_.contains(service_type)) {
      libs_[service_type] = rclcpp::get_typesupport_library(service_type, "rosidl_typesupport_cpp");
    }
    if (!service_type_support_cache_.contains(service_type)) {
      service_type_support_cache_[service_type] = rclcpp::get_service_typesupport_handle(
        service_type, "rosidl_typesupport_cpp", *libs_[service_type]);
    }
    service_types_[env_name][service_name] = service_type;

    clients_[context->env_name_][service_name] =
      rclcpp::create_generic_client(node, service_name, service_type, rclcpp::QoS(10), cb_group_);
    map_mtx_.unlock();
    clips::AssertString(
      env, ("(ros-msgs-client (service \"" + service_name + "\") (type \"" + service_type + "\"))")
             .c_str());
  }
}

clips::UDFValue RosMsgsPlugin::create_request(
  clips::Environment * env, const std::string & service_type)
{
  auto scoped_lock = std::scoped_lock{map_mtx_};
  std::shared_ptr<MessageInfo> ptr;

  if (!libs_.contains(service_type)) {
    RCLCPP_DEBUG(*logger_, "Create new message information on message creation");
    libs_[service_type] = rclcpp::get_typesupport_library(service_type, "rosidl_typesupport_cpp");
    service_type_support_cache_[service_type] = rclcpp::get_service_typesupport_handle(
      service_type, "rosidl_typesupport_cpp", *libs_[service_type]);
  }
  auto * introspection_type_support = get_service_typesupport_handle(
    service_type_support_cache_[service_type],
    rosidl_typesupport_introspection_cpp::typesupport_identifier);
  auto * introspection_info =
    static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
      introspection_type_support->request_typesupport->data);
  ptr = std::make_shared<RosMsgsPlugin::MessageInfo>(introspection_info);
  messages_[ptr.get()] = ptr;
  clips::UDFValue res;
  res.externalAddressValue =
    clips::CreateCExternalAddress(env, reinterpret_cast<void *>(ptr.get()));
  return res;
}

clips::UDFValue RosMsgsPlugin::send_request(
  clips::Environment * env, void * deserialized_msg, const std::string & service_name)
{
  using namespace std::chrono_literals;
  auto scoped_lock = std::scoped_lock{map_mtx_};
  clips::UDFValue result;
  if (!messages_.contains(deserialized_msg)) {
    RCLCPP_ERROR(*logger_, "Failed to publish invalid msg pointer");
    result.value = clips::CreateBoolean(env, false);
    return result;
  }
  std::shared_ptr<MessageInfo> msg_info = messages_[deserialized_msg];
  std::string msg_type = get_msg_type(msg_info->members);
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;
  if (!clients_[env_name][service_name]->wait_for_service(1s)) {
    RCLCPP_WARN(*logger_, "service %s not available, abort request.", service_name.c_str());
    result.value = clips::CreateBoolean(env, false);
    return result;
  }
  rclcpp::GenericClient::FutureAndRequestId future_and_id =
    clients_[env_name][service_name]->async_send_request(msg_info->msg_ptr);
  int id = future_and_id.request_id;
  rclcpp::GenericClient::SharedFuture fut = future_and_id.future.share();
  std::thread([this, env, service_name, env_name, id, fut]() {
    auto context = CLIPSEnvContext::get_context(env);
    std::shared_ptr<void> resp = fut.get();
    std::shared_ptr<MessageInfo> response_info;
    {
      std::scoped_lock map_lock{map_mtx_};
      std::string service_type = service_types_[env_name][service_name];
      auto * introspection_type_support = get_service_typesupport_handle(
        service_type_support_cache_[service_type],
        rosidl_typesupport_introspection_cpp::typesupport_identifier);
      auto * introspection_info =
        static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
          introspection_type_support->response_typesupport->data);
      response_info = std::make_shared<MessageInfo>(introspection_info, resp);
      std::string msg_type = get_msg_type(introspection_info);
      if (!response_info) {
        RCLCPP_ERROR(
          *logger_, "failed to process msg (service: %s response type: %s)", service_name.c_str(),
          msg_type.c_str());
        return;
      }

      messages_[response_info.get()] = response_info;
    }
    {
      std::lock_guard<std::mutex> guard(context->env_mtx_);
      clips::FactBuilder * fact_builder = clips::CreateFactBuilder(env, "ros-msgs-response");
      clips::FBPutSlotString(fact_builder, "service", service_name.c_str());
      clips::FBPutSlotInteger(fact_builder, "request-id", id);
      clips::FBPutSlotCLIPSExternalAddress(
        fact_builder, "msg-ptr", clips::CreateCExternalAddress(env, response_info.get()));
      clips::FBAssert(fact_builder);
      clips::FBDispose(fact_builder);
    }
  }).detach();
  result.value = clips::CreateInteger(env, id);
  return result;
}
}  // namespace cx
#endif
