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

{{name_camel}}::{{name_camel}}()
    : Node("{{name_snake}}_msg_plugin_node") {}
{{name_camel}}::~{{name_camel}}() {}

void {{name_camel}}::finalize() {
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
  if(services_.size() > 0) {
    for (const auto &service_map : services_) {
      for (const auto &service : service_map.second) {
        RCLCPP_WARN(*logger_,
                    "Environment %s has open %s service, cleaning up ...",
                    service_map.first.c_str(), service.first.c_str());
      }
    }
    services_.clear();
  }
  if(requests_.size() > 0) {
    RCLCPP_WARN(*logger_, "Found %li request(s), cleaning up ...", requests_.size());
    requests_.clear();
  }
  if(responses_.size() > 0) {
    RCLCPP_WARN(*logger_, "Found %li response(s), cleaning up ...", responses_.size());
    responses_.clear();
  }
  stop_flag_ = true;
}

void {{name_camel}}::initialize() {
   logger_ = std::make_unique<rclcpp::Logger>(rclcpp::get_logger(plugin_name_));
   auto node = parent_.lock();
   cb_group_ = node->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
   stop_flag_ = false;
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
  curr_tmpl = clips::FindDeftemplate(env.get(), "{{name_kebab}}-service");
  if(curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_,
              "{{name_kebab}}-service can not be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "{{name_kebab}}-response");
  if(curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_,
              "{{name_kebab}}-response can not be undefined");
  }

  return true;
}

bool {{name_camel}}::clips_env_init(std::shared_ptr<clips::Environment> &env) {
  RCLCPP_DEBUG(*logger_,
              "Initialising context for plugin %s",
              plugin_name_.c_str());

{% set template_part = "registration" %}
{% set template_type = "Request" %}
{% include 'get_field.jinja.cpp' with context %}
{% include 'set_field.jinja.cpp' with context %}
{% include 'create.jinja.cpp' with context %}
{% include 'destroy.jinja.cpp' with context %}
{% set template_type = "Response" %}
{% include 'get_field.jinja.cpp' with context %}
{% include 'set_field.jinja.cpp' with context %}
{% include 'create.jinja.cpp' with context %}
{% include 'destroy.jinja.cpp' with context %}

  std::string fun_name;

  fun_name = "{{name_kebab}}-create-service";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";sy",
    [](clips::Environment *env, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue service_name;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &service_name);

      instance->create_new_service(env, service_name.lexemeValue->contents);
    },
    "create_new_service", this);

  fun_name = "{{name_kebab}}-destroy-service";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";sy",
    [](clips::Environment *env, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue service_name;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &service_name);

      instance->destroy_service(env, service_name.lexemeValue->contents);
    },
    "destroy_service", this);

  fun_name = "{{name_kebab}}-create-client";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";s",
    [](clips::Environment *env, clips::UDFContext *udfc, clips::UDFValue * /*out*/) {
        auto *instance = static_cast<{{name_camel}} *>(udfc->context);
        clips::UDFValue service_name;

        using namespace clips;
        clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &service_name);
        instance->create_new_client(env,service_name.lexemeValue->contents);
    },
    "create_new_client", this);

  fun_name = "{{name_kebab}}-destroy-client";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";s",
    [](clips::Environment *env, clips::UDFContext *udfc, clips::UDFValue * /*out*/) {
        auto *instance = static_cast<{{name_camel}} *>(udfc->context);
        clips::UDFValue service_name;

        using namespace clips;
        clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &service_name);
        instance->destroy_client(env, service_name.lexemeValue->contents);
    },
    "destroy_client", this);

  fun_name = "{{name_kebab}}-send-request";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";e;sy",
    [](clips::Environment *env, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue request, service_name;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &request);
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &service_name);

      instance->send_request(env, static_cast<{{message_type}}::Request*>(request.externalAddressValue->contents), service_name.lexemeValue->contents);
    },
    "send_request", this);


  // add fact templates
  clips::Build(env.get(),"(deftemplate {{name_kebab}}-client \
            (slot service (type STRING)))");
  clips::Build(env.get(),"(deftemplate {{name_kebab}}-service \
            (slot name (type STRING)))");
  clips::Build(env.get(),"(deftemplate {{name_kebab}}-response \
            (slot service (type STRING) ) \
            (slot msg-ptr (type EXTERNAL-ADDRESS)) \
            )");


  return true;
}

{% set template_part = "definition" %}
{% set template_slots = request_slots %}
{% set template_type = "Request" %}
{% include 'set_field.jinja.cpp' with context %}
{% include 'get_field.jinja.cpp' with context %}
{% include 'create.jinja.cpp' with context %}
{% include 'destroy.jinja.cpp' with context %}

{% set template_slots = response_slots %}
{% set template_type = "Response" %}
{% include 'set_field.jinja.cpp' with context %}
{% include 'get_field.jinja.cpp' with context %}
{% include 'create.jinja.cpp' with context %}
{% include 'destroy.jinja.cpp' with context %}

void {{name_camel}}::send_request(clips::Environment *env, {{message_type}}::Request *req, const std::string &service_name) {
  using namespace std::chrono_literals;
  {{message_type}}::Request::SharedPtr req_shared = std::make_shared<{{message_type}}::Request>(*req);

   // Handle the result asynchronously to not block clips engine potentially endlessly
  std::thread([this, req_shared, service_name, env]() {
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;
  bool print_warning = true;
  while (!clients_[env_name][service_name]->wait_for_service(1s)) {
    if (stop_flag_ || !rclcpp::ok()) {
      RCLCPP_ERROR(*logger_, "Interrupted while waiting for the service. Exiting.");
      return;
    }
    if(print_warning) {
      RCLCPP_WARN(*logger_, "service %s not available, start waiting", service_name.c_str());
      print_warning = false;
    }
    RCLCPP_DEBUG(*logger_, "service %s not available, waiting again...", service_name.c_str());
  }
  if(!print_warning) {
      RCLCPP_INFO(*logger_, "service %s is finally reachable", service_name.c_str());
  }
  std::shared_ptr<{{message_type}}::Response> resp;
  {
    std::scoped_lock map_lock{map_mtx_};
    auto future = clients_[env_name][service_name]->async_send_request(req_shared);
    resp = future.get();
    responses_.try_emplace(resp.get(), resp);
  }
   std::lock_guard<std::mutex> guard(context->env_mtx_);
   clips::FactBuilder *fact_builder = clips::CreateFactBuilder(env, "{{name_kebab}}-response");
   clips::FBPutSlotString(fact_builder,"service",service_name.c_str());
   clips::FBPutSlotCLIPSExternalAddress(fact_builder,"msg-ptr", clips::CreateCExternalAddress(env, resp.get()));
   clips::FBAssert(fact_builder);
   clips::FBDispose(fact_builder);

  }).detach();
}

void {{name_camel}}::create_new_service(clips::Environment *env, const std::string &service_name) {
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;
  auto node = parent_.lock();

  std::scoped_lock map_lock{map_mtx_};
  services_[env_name][service_name] = node->create_service<{{message_type}}>(service_name, [this, env_name, service_name, env](const std::shared_ptr<{{message_type}}::Request> request,
    std::shared_ptr<{{message_type}}::Response> response) {
    this->service_callback(request, response, service_name, env);
  });
    clips::AssertString(env, ("({{name_kebab}}-service (name \"" +
                                 service_name + "\"))").c_str());
}

void {{name_camel}}::destroy_service(clips::Environment *env, const std::string &service_name) {
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;

  std::scoped_lock map_lock{map_mtx_};
  auto outer_it = services_.find(env_name);
  if (outer_it != services_.end()) {
      // Check if service_name exists in the inner map
      auto& inner_map = outer_it->second;
      auto inner_it = inner_map.find(service_name);
      if (inner_it != inner_map.end()) {
          // Remove the service_name entry from the inner map
          inner_map.erase(inner_it);
      } else {
          RCLCPP_WARN(*logger_, "Service %s not found in environment %s", service_name.c_str(), env_name.c_str());
      }
  } else {
      RCLCPP_WARN(*logger_, "Environment %s not found", env_name.c_str());
  }
}

void {{name_camel}}::create_new_client(clips::Environment *env,
    const std::string &service_name) {
  RCLCPP_DEBUG(rclcpp::get_logger(plugin_name_), "Creating client for serive %s",
              service_name.c_str());
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;

  map_mtx_.lock();
  auto it = clients_[env_name].find(service_name);

  if (it != clients_[env_name].end()) {
    RCLCPP_DEBUG(rclcpp::get_logger(plugin_name_),
                "There already exists a client for service %s", service_name.c_str());
    map_mtx_.unlock();
  } else {
    RCLCPP_DEBUG(rclcpp::get_logger(plugin_name_),
                "Creating client for service %s", service_name.c_str());
    auto node = parent_.lock();
    clients_[env_name][service_name] =
        node->create_client<{{message_type}}>(service_name);
    map_mtx_.unlock();
    clips::AssertString(env, ("({{name_kebab}}-client (service \"" + service_name + "\"))").c_str());
  }
}

void {{name_camel}}::destroy_client(clips::Environment *env,
    const std::string &service_name) {
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;
  {
    std::scoped_lock map_lock{map_mtx_};
    auto it = clients_[env_name].find(service_name);

    if (it != clients_[env_name].end()) {
      RCLCPP_DEBUG(rclcpp::get_logger(plugin_name_),
                  "Destroying client for service %s", service_name.c_str());
      clients_[env_name].erase(service_name);
    }
  }

  clips::Eval(env, ("(do-for-all-facts ((?f {{name_kebab}}-client)) (eq (str-cat ?f:service) (str-cat " + service_name + "))  (retract ?f))").c_str(), NULL);
}

void {{name_camel}}::service_callback(const std::shared_ptr<{{message_type}}::Request> request,
                        std::shared_ptr<{{message_type}}::Response> response,
    std::string service_name,
    clips::Environment *env) {
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;
  std::lock_guard<std::mutex> guard(context->env_mtx_);

  // call a user-defined function
  clips::Deffunction *dec_fun = clips::FindDeffunction(env,"{{name_kebab}}-service-callback");
  if(!dec_fun) {
    RCLCPP_WARN(*logger_, "{{name_kebab}}-service-callback not defined, skip callback");
    return;
  }
  clips::FunctionCallBuilder *fcb = clips::CreateFunctionCallBuilder(env,3);
  clips::FCBAppendString(fcb, service_name.c_str());
  clips::FCBAppendCLIPSExternalAddress(fcb, clips::CreateCExternalAddress(env, request.get()));
  clips::FCBAppendCLIPSExternalAddress(fcb, clips::CreateCExternalAddress(env, response.get()));
  clips::FCBCall(fcb,"{{name_kebab}}-service-callback",NULL);
  clips::FCBDispose(fcb);
}

} // namespace cx
PLUGINLIB_EXPORT_CLASS(cx::{{name_camel}}, cx::ClipsPlugin)
