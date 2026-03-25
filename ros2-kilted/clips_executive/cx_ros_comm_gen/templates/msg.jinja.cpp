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
#include <map>
#include <memory>
#include <string>

#include "cx_plugin/clips_plugin.hpp"
#include "{{name_snake}}.hpp"
#include "cx_utils/clips_env_context.hpp"
#include "cx_utils/format.hpp"

// To export as plugin
#include "pluginlib/class_list_macros.hpp"

using std::placeholders::_1;

namespace cx {

{{name_camel}}::{{name_camel}}()
    : Node("{{name_snake}}_msg_plugin_node") {}
{{name_camel}}::~{{name_camel}}() {
}

void {{name_camel}}::finalize() {
  std::scoped_lock map_lock{map_mtx_};
  if (subscriptions_.size() > 0) {
    for (const auto &sub_map : subscriptions_) {
      for (const auto &sub : sub_map.second) {
        RCLCPP_WARN(*logger_,
                    "Environment %s has open %s subscription, cleaning up ...",
                    sub_map.first.c_str(), sub.first.c_str());
      }
    }
    subscriptions_.clear();
  }
  if (publishers_.size() > 0) {
    for (const auto &pub_map : publishers_) {
      for (const auto &pub : pub_map.second) {
        RCLCPP_WARN(*logger_,
                    "Environment %s has open %s publishers, cleaning up ...",
                    pub_map.first.c_str(), pub.first.c_str());
      }
    }
    publishers_.clear();
  }
  cb_group_.reset();
  if (messages_.size() > 0) {
    RCLCPP_WARN(*logger_, "Found %li message(s), cleaning up ...",
                messages_.size());
    messages_.clear();
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
  clips::Deftemplate *curr_tmpl = clips::FindDeftemplate(env.get(), "{{name_kebab}}-subscription");
  if(curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_,
              "{{name_kebab}}-subscription cant be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "{{name_kebab}}-publisher");
  if(curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_,
              "{{name_kebab}}-publisher cant be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "{{name_kebab}}-message");
  if(curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_,
              "{{name_kebab}}-msg cant be undefined");
  }
  return true;
}

bool {{name_camel}}::clips_env_init(std::shared_ptr<clips::Environment> &env) {
  RCLCPP_DEBUG(*logger_,
              "Initializing context for plugin %s",
              plugin_name_.c_str());

{% set template_part = "registration" %}
{% set template_type = "" %}
{% include 'get_field.jinja.cpp' with context %}
{% include 'set_field.jinja.cpp' with context %}

  std::string fun_name;

  fun_name = "{{name_kebab}}-create-message";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "e", 0, 0, "",
    [](clips::Environment *env, clips::UDFContext *udfc,
       clips::UDFValue *out) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      using namespace clips;
      *out = instance->create_message(env);
    },
    "create_message", this);

  fun_name = "{{name_kebab}}-create-publisher";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";sy",
    [](clips::Environment *env, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue topic;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &topic);

      instance->create_new_publisher(env, topic.lexemeValue->contents);
    },
    "create_new_publisher", this);

  fun_name = "{{name_kebab}}-destroy-publisher";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";sy",
    [](clips::Environment *env, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue topic;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &topic);

      instance->destroy_publisher(env, topic.lexemeValue->contents);
    },
    "destroy_publisher", this);

  fun_name = "{{name_kebab}}-publish";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";e;sy",
    [](clips::Environment *env, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue message, topic;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &message);
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &topic);

      instance->publish_to_topic(env, static_cast<{{message_type}}*>(message.externalAddressValue->contents), topic.lexemeValue->contents);
    },
    "publish_to_topic", this);

  fun_name = "{{name_kebab}}-destroy-message";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";e",
    [](clips::Environment */*env*/, clips::UDFContext *udfc,
       clips::UDFValue * /*out*/) {
      auto *instance = static_cast<{{name_camel}} *>(udfc->context);
      clips::UDFValue message;
      using namespace clips;
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &message);

      instance->destroy_msg(static_cast<{{message_type}}*>(message.externalAddressValue->contents));
    },
    "destroy_msg", this);

  fun_name = "{{name_kebab}}-create-subscription";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";s",
    [](clips::Environment *env, clips::UDFContext *udfc, clips::UDFValue * /*out*/) {
        auto *instance = static_cast<{{name_camel}} *>(udfc->context);
        clips::UDFValue topic;

        using namespace clips;
        clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &topic);
        instance->subscribe_to_topic(env,topic.lexemeValue->contents);
    },
    "subscribe_to_topic", this);

  fun_name = "{{name_kebab}}-destroy-subscription";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";s",
    [](clips::Environment *env, clips::UDFContext *udfc, clips::UDFValue * /*out*/) {
        auto *instance = static_cast<{{name_camel}} *>(udfc->context);
        clips::UDFValue topic;

        using namespace clips;
        clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &topic);
        instance->unsubscribe_from_topic(env, topic.lexemeValue->contents);
    },
    "unsubscribe_from_topic", this);


  // add fact templates
  clips::Build(env.get(),"(deftemplate {{name_kebab}}-subscription \
            (slot topic (type STRING)))");
  clips::Build(env.get(),"(deftemplate {{name_kebab}}-publisher \
            (slot topic (type STRING)))");
  clips::Build(env.get(),"(deftemplate {{name_kebab}}-message \
            (slot topic (type STRING) ) \
            (slot msg-ptr (type EXTERNAL-ADDRESS)) \
            )");


  return true;
}

{% set template_part = "definition" %}
{% set template_slots = slots %}
{% set template_type = "" %}
{% include 'get_field.jinja.cpp' with context %}
{% include 'set_field.jinja.cpp' with context %}

void {{name_camel}}::publish_to_topic(clips::Environment *env, {{message_type}} *msg, const std::string &topic_name) {
  auto context = CLIPSEnvContext::get_context(env);
  publishers_[context->env_name_][topic_name]->publish(*msg);
}

void {{name_camel}}::create_new_publisher(clips::Environment *env, const std::string &topic_name) {
  std::scoped_lock map_lock{map_mtx_};
  auto context = CLIPSEnvContext::get_context(env);
  auto node = parent_.lock();
  publishers_[context->env_name_][topic_name] =
      node->create_publisher<{{message_type}}>(topic_name, 10);
    clips::AssertString(env, ("({{name_kebab}}-publisher (topic \"" +
                                 topic_name + "\"))").c_str());
}

void {{name_camel}}::destroy_publisher(clips::Environment *env, const std::string &topic_name) {
  std::scoped_lock map_lock{map_mtx_};
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;
  auto outer_it = publishers_.find(env_name);
  if (outer_it != publishers_.end()) {
      // Check if topic_name exists in the inner map
      auto& inner_map = outer_it->second;
      auto inner_it = inner_map.find(topic_name);
      if (inner_it != inner_map.end()) {
          // Remove the topic_name entry from the inner map
          inner_map.erase(inner_it);
      } else {
          RCLCPP_WARN(*logger_, "Topic %s not found in environment %s", topic_name.c_str(), env_name.c_str());
      }
  } else {
      RCLCPP_WARN(*logger_, "Environment %s not found", env_name.c_str());
  }
}

void {{name_camel}}::subscribe_to_topic(clips::Environment *env,
    const std::string &topic_name) {
  RCLCPP_DEBUG(rclcpp::get_logger(plugin_name_), "Subscribing to topic %s",
              topic_name.c_str());
  std::scoped_lock map_lock{map_mtx_};
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;

  auto it = subscriptions_[env_name].find(topic_name);

  if (it != subscriptions_[env_name].end()) {
    RCLCPP_WARN(rclcpp::get_logger(plugin_name_),
                "Already subscribed to topic %s", topic_name.c_str());
  } else {
    RCLCPP_DEBUG(rclcpp::get_logger(plugin_name_),
                "Creating subscription to topic %s", topic_name.c_str());
	auto options = rclcpp::SubscriptionOptions();
	options.callback_group = cb_group_;
    auto node = parent_.lock();
    subscriptions_[env_name][topic_name] =
        node->create_subscription<{{message_type}}>(
            topic_name, 10, [this,topic_name,env](const {{message_type}}::SharedPtr msg) {
              topic_callback(msg, topic_name, env);
            }, options);
    clips::AssertString(env, ("({{name_kebab}}-subscription (topic \"" +
                                 topic_name + "\"))").c_str());
  }
}

void {{name_camel}}::unsubscribe_from_topic(clips::Environment *env,
    const std::string &topic_name) {
  std::scoped_lock map_lock{map_mtx_};
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;

  auto it = subscriptions_[env_name].find(topic_name);

  if (it != subscriptions_[env_name].end()) {
    RCLCPP_DEBUG(rclcpp::get_logger(plugin_name_),
                "Unsubscribing from topic %s", topic_name.c_str());
    subscriptions_[env_name].erase(topic_name);
  }

  clips::Eval(env, ("(do-for-all-facts ((?f {{name_kebab}}-subscription)) (eq (str-cat ?f:topic) (str-cat " + topic_name + "))  (retract ?f))").c_str(), NULL);
}

void {{name_camel}}::topic_callback(
    const {{message_type}}::SharedPtr msg, std::string topic_name, clips::Environment *env) {
  auto context = CLIPSEnvContext::get_context(env);
  std::scoped_lock clips_lock{context->env_mtx_};
  {
    std::scoped_lock map_lock{map_mtx_};
    messages_[msg.get()] = msg;
  }

  // assert the newest message
  clips::FactBuilder *fact_builder = clips::CreateFactBuilder(env, "{{name_kebab}}-message");
  clips::FBPutSlotString(fact_builder,"topic",topic_name.c_str());
  clips::FBPutSlotCLIPSExternalAddress(fact_builder,"msg-ptr", clips::CreateCExternalAddress(env, msg.get()));
  clips::FBAssert(fact_builder);
  clips::FBDispose(fact_builder);
}

clips::UDFValue {{name_camel}}::create_message(clips::Environment *env) {
  std::shared_ptr<{{message_type}}> ptr = std::make_shared<{{message_type}}>();
  std::scoped_lock map_lock{map_mtx_};
  messages_[ptr.get()] = ptr;
  clips::UDFValue res;
  res.externalAddressValue = clips::CreateCExternalAddress(env, ptr.get());
  return res;
}

void {{name_camel}}::destroy_msg({{message_type}} *msg) {
  std::scoped_lock map_lock{map_mtx_};
  auto it = messages_.find(msg);
  if (it != messages_.end()) {
      messages_.erase(it);
  }
}

} // namespace cx
PLUGINLIB_EXPORT_CLASS(cx::{{name_camel}}, cx::ClipsPlugin)
