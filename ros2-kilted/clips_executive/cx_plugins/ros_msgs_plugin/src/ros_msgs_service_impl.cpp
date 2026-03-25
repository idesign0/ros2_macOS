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

#ifdef HAVE_GENERIC_SERVICE
#include "cx_ros_msgs_plugin/ros_msgs_plugin.hpp"
#include "cx_utils/clips_env_context.hpp"
#include "rclcpp/create_generic_service.hpp"

namespace cx
{
void RosMsgsPlugin::finalize_generic_service_bindings()
{
  if (services_.size() > 0) {
    for (const auto & service_map : services_) {
      for (const auto & service : service_map.second) {
        RCLCPP_WARN(
          *logger_, "Environment %s has open %s services, cleaning up ...",
          service_map.first.c_str(), service.first.c_str());
      }
    }
    services_.clear();
  }
}

void RosMsgsPlugin::init_generic_service_bindings(std::shared_ptr<clips::Environment> env)
{
  std::string fun_name = "ros-msgs-create-service";
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

      instance->create_new_service(env, service.lexemeValue->contents, type.lexemeValue->contents);
    },
    "create_new_service", this);

  fun_name = "ros-msgs-destroy-service";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";sy;sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue service;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &service);

      instance->destroy_service(env, service.lexemeValue->contents);
    },
    "create_new_service", this);

  clips::Build(
    env.get(),
    R"((deftemplate ros-msgs-service
            (slot service (type STRING))
            (slot type (type STRING))))");
}

void RosMsgsPlugin::destroy_generic_service_bindings(std::shared_ptr<clips::Environment> env)
{
  clips::Deftemplate * curr_tmpl = clips::FindDeftemplate(env.get(), "ros-msgs-service");
  if (curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_, "ros-msgs-service cant be undefined");
  }
}

void RosMsgsPlugin::create_new_service(
  clips::Environment * env, const std::string & service_name, const std::string & service_type)
{
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;

  auto node = parent_.lock();
  if (!node) {
    RCLCPP_ERROR(*logger_, "Invalid reference to parent node");
  }

  std::map<std::__cxx11::basic_string<char>, std::shared_ptr<rclcpp::GenericService>>::iterator it;
  {
    map_mtx_.lock();
    it = services_[env_name].find(service_name);
  }

  if (it != services_[env_name].end()) {
    map_mtx_.unlock();
    RCLCPP_WARN(*logger_, "Already registered to service %s", service_name.c_str());
  } else {
    RCLCPP_DEBUG(*logger_, "Creating service %s", service_name.c_str());

    if (!libs_.contains(service_type)) {
      libs_[service_type] = rclcpp::get_typesupport_library(service_type, "rosidl_typesupport_cpp");
    }
    if (!service_type_support_cache_.contains(service_type)) {
      service_type_support_cache_[service_type] = rclcpp::get_service_typesupport_handle(
        service_type, "rosidl_typesupport_cpp", *libs_[service_type]);
    }
    service_types_[env_name][service_name] = service_type;
    auto callback = [this, env, service_name](
                      std::shared_ptr<rmw_request_id_t> header,
                      rclcpp::GenericService::SharedRequest request,
                      rclcpp::GenericService::SharedResponse response) {
      service_callback(env, service_name, request, response);
      auto context = CLIPSEnvContext::get_context(env);
      services_[context->env_name_][service_name]->send_response(*header, response);
    };

    services_[context->env_name_][service_name] = rclcpp::create_generic_service(
      node->get_node_base_interface(), node->get_node_services_interface(), service_name,
      service_type, callback, rclcpp::QoS(10), cb_group_);

    map_mtx_.unlock();
    clips::AssertString(
      env, ("(ros-msgs-service (service \"" + service_name + "\") (type \"" + service_type + "\"))")
             .c_str());
  }
}

void RosMsgsPlugin::destroy_service(clips::Environment * env, const std::string & service_name)
{
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;
  map_mtx_.lock();
  auto outer_it = services_.find(env_name);
  if (outer_it != services_.end()) {
    // Check if service_name exists in the inner map
    auto & inner_map = outer_it->second;
    auto inner_it = inner_map.find(service_name);
    if (inner_it != inner_map.end()) {
      // Remove the service_name entry from the inner map
      RCLCPP_DEBUG(*logger_, "Destroying service %s", service_name.c_str());
      inner_map.erase(inner_it);
      map_mtx_.unlock();
      clips::Eval(
        env,
        ("(do-for-all-facts ((?f ros-msgs-service)) (eq (str-cat "
         "?f:service) (str-cat \"" +
         service_name + "\"))  (retract ?f))")
          .c_str(),
        NULL);
    } else {
      map_mtx_.unlock();
      RCLCPP_WARN(
        *logger_, "Service %s not found in environment %s", service_name.c_str(), env_name.c_str());
    }
  } else {
    map_mtx_.unlock();
    RCLCPP_WARN(*logger_, "Environment %s not found", env_name.c_str());
  }
}

void RosMsgsPlugin::service_callback(
  clips::Environment * env, const std::string service_name,
  rclcpp::GenericService::SharedRequest request, rclcpp::GenericService::SharedResponse response)
{
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;
  std::string response_msg_type;
  std::string service_type;

  std::shared_ptr<MessageInfo> request_info;
  std::shared_ptr<MessageInfo> response_info;
  {
    std::scoped_lock map_lock{map_mtx_};
    service_type = service_types_[env_name][service_name];
    auto * introspection_type_support = get_service_typesupport_handle(
      service_type_support_cache_[service_type],
      rosidl_typesupport_introspection_cpp::typesupport_identifier);
    auto * request_introspection_info =
      static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
        introspection_type_support->request_typesupport->data);
    auto * response_introspection_info =
      static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
        introspection_type_support->response_typesupport->data);
    request_info = std::make_shared<MessageInfo>(request_introspection_info, request);
    std::string request_msg_type = get_msg_type(request_introspection_info);
    if (!request_info) {
      RCLCPP_ERROR(
        *logger_, "failed to process request (service: %s request type: %s)", service_name.c_str(),
        request_msg_type.c_str());
      return;
    }
    response_info = std::make_shared<MessageInfo>(response_introspection_info, response);
    response_msg_type = get_msg_type(response_introspection_info);
    if (!response_info) {
      RCLCPP_ERROR(
        *logger_, "failed to process response (service: %s response type: %s)",
        service_name.c_str(), response_msg_type.c_str());
      return;
    }

    messages_[request_info.get()] = request_info;
    messages_[response_info.get()] = response_info;
  }
  {
    std::lock_guard<std::mutex> guard(context->env_mtx_);
    std::string fun_name = service_name + "-service-callback";

    // call a user-defined function
    clips::Deffunction * dec_fun = clips::FindDeffunction(env, fun_name.c_str());
    if (!dec_fun) {
      RCLCPP_WARN(*logger_, "%s not defined, skip callback", fun_name.c_str());
      return;
    }
    clips::FunctionCallBuilder * fcb = clips::CreateFunctionCallBuilder(env, 3);
    clips::FCBAppendString(fcb, service_name.c_str());
    clips::FCBAppendCLIPSExternalAddress(
      fcb, clips::CreateCExternalAddress(env, request_info.get()));
    clips::FCBAppendCLIPSExternalAddress(
      fcb, clips::CreateCExternalAddress(env, response_info.get()));
    clips::FCBCall(fcb, fun_name.c_str(), NULL);
    clips::FCBDispose(fcb);
  }
  destroy_msg(request_info.get());
  destroy_msg(response_info.get());
}

void RosMsgsPlugin::destroy_client(clips::Environment * env, const std::string & service_name)
{
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;
  map_mtx_.lock();
  auto outer_it = clients_.find(env_name);
  if (outer_it != clients_.end()) {
    // Check if service_name exists in the inner map
    auto & inner_map = outer_it->second;
    auto inner_it = inner_map.find(service_name);
    if (inner_it != inner_map.end()) {
      // Remove the service_name entry from the inner map
      RCLCPP_DEBUG(*logger_, "Destroying client for service %s", service_name.c_str());
      inner_map.erase(inner_it);
      map_mtx_.unlock();
      clips::Eval(
        env,
        ("(do-for-all-facts ((?f ros-msgs-client)) (eq (str-cat "
         "?f:service) (str-cat \"" +
         service_name + "\"))  (retract ?f))")
          .c_str(),
        NULL);
    } else {
      map_mtx_.unlock();
      RCLCPP_WARN(
        *logger_, "Client %s not found in environment %s", service_name.c_str(), env_name.c_str());
    }
  } else {
    map_mtx_.unlock();
    RCLCPP_WARN(*logger_, "Environment %s not found", env_name.c_str());
  }
}
}  // namespace cx
#endif
