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

#include "cx_ros_msgs_plugin/ros_msgs_plugin.hpp"

#include <string>

#include "cx_utils/clips_env_context.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rcutils/types/uint8_array.h"
#include "rmw/rmw.h"
#include "rmw/serialized_message.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "unicode/ucnv.h"
#include "unicode/unistr.h"

// To export as plugin
#include "pluginlib/class_list_macros.hpp"

using std::placeholders::_1;
namespace cx
{
RosMsgsPlugin::RosMsgsPlugin() {}

RosMsgsPlugin::~RosMsgsPlugin() {}

void RosMsgsPlugin::initialize()
{
  logger_ = std::make_unique<rclcpp::Logger>(rclcpp::get_logger(plugin_name_));
  auto node = parent_.lock();
  cb_group_ = node->create_callback_group(rclcpp::CallbackGroupType::Reentrant);

  clips_worker_thread_ = std::thread([this]() {
    while (!stop_flag_) {
      std::function<void()> task;
      {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        cv_.wait(lock, [&] { return !task_queue_.empty() || stop_flag_; });
        if (stop_flag_) {
          continue;
        }
        task = std::move(task_queue_.front());
        task_queue_.pop();
      }
      task();
    }
  });
}

void RosMsgsPlugin::finalize()
{
  if (subscriptions_.size() > 0) {
    for (const auto & sub_map : subscriptions_) {
      for (const auto & sub : sub_map.second) {
        RCLCPP_WARN(
          *logger_, "Environment %s has open %s subscription, cleaning up ...",
          sub_map.first.c_str(), sub.first.c_str());
      }
    }
    subscriptions_.clear();
  }
  if (publishers_.size() > 0) {
    for (const auto & pub_map : publishers_) {
      for (const auto & pub : pub_map.second) {
        RCLCPP_WARN(
          *logger_, "Environment %s has open %s publishers, cleaning up ...", pub_map.first.c_str(),
          pub.first.c_str());
      }
    }
    publishers_.clear();
  }
#ifdef HAVE_GENERIC_CLIENT
  finalize_generic_client_bindings();
#endif
#ifdef HAVE_GENERIC_SERVICE
  finalize_generic_service_bindings();
#endif
#ifdef HAVE_GENERIC_ACTION_CLIENT
  finalize_generic_action_client_bindings();
#endif
  cb_group_.reset();
  if (messages_.size() > 0) {
    RCLCPP_WARN(*logger_, "Found %li message(s), cleaning up ...", messages_.size());
    messages_.clear();
  }
  sub_messages_.clear();
  stop_flag_ = true;
  cv_.notify_all();

  if (clips_worker_thread_.joinable()) {
    clips_worker_thread_.join();
  }
}

bool RosMsgsPlugin::clips_env_init(std::shared_ptr<clips::Environment> & env)
{
  auto context = CLIPSEnvContext::get_context(env.get());
  RCLCPP_DEBUG(*logger_, "Initializing plugin for environment %s", context->env_name_.c_str());

  std::string fun_name = "ros-msgs-get-field";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "*", 2, 2, ";e;sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue data_ptr, field;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &data_ptr);
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &field);
      *out = instance->get_field(
        env, data_ptr.externalAddressValue->contents, field.lexemeValue->contents, udfc);
    },
    "get_field", this);

  fun_name = "ros-msgs-set-field";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 3, 3, ";e;sy;*",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue msg_ptr, field, value;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &msg_ptr);
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &field);
      clips::UDFNthArgument(udfc, 3, ANY_TYPE_BITS, &value);

      instance->set_field(
        env, msg_ptr.externalAddressValue->contents, field.lexemeValue->contents, value, udfc);
    },
    "set_field", this);

  fun_name = "ros-msgs-create-message";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "e", 1, 1, ";sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue type;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &type);
      *out = instance->create_message(env, type.lexemeValue->contents);
    },
    "create_message", this);

  fun_name = "ros-msgs-create-publisher";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";sy;sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue topic;
      clips::UDFValue type;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &topic);
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &type);

      instance->create_new_publisher(env, topic.lexemeValue->contents, type.lexemeValue->contents);
    },
    "create_new_publisher", this);

  fun_name = "ros-msgs-destroy-publisher";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue topic;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &topic);

      instance->destroy_publisher(env, topic.lexemeValue->contents);
    },
    "destroy_publisher", this);

  fun_name = "ros-msgs-publish";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";e;sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue message, topic;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &message);
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &topic);

      instance->publish_to_topic(
        env, message.externalAddressValue->contents, topic.lexemeValue->contents);
    },
    "publish_to_topic", this);

  fun_name = "ros-msgs-destroy-message";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";e",
    [](clips::Environment * /*env*/, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue message;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &message);

      instance->destroy_msg(message.externalAddressValue->contents);
    },
    "destroy_msg", this);

  fun_name = "ros-msgs-create-subscription";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 2, 2, ";sy;sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue topic;
      clips::UDFValue type;

      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &topic);
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &type);
      instance->subscribe_to_topic(env, topic.lexemeValue->contents, type.lexemeValue->contents);
    },
    "subscribe_to_topic", this);

  fun_name = "ros-msgs-destroy-subscription";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "v", 1, 1, ";s",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue topic;

      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &topic);
      instance->unsubscribe_from_topic(env, topic.lexemeValue->contents);
    },
    "unsubscribe_from_topic", this);

  fun_name = "ros-msgs-goal-uuid-to-string";
  function_names_.insert(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "y", 1, 1, ";m",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto * instance = static_cast<RosMsgsPlugin *>(udfc->context);
      clips::UDFValue res;
      clips::UDFValue uuid_field;
      clips::UDFNthArgument(udfc, 1, clips::MULTIFIELD_BIT, &uuid_field);

      rclcpp_action::GoalUUID uuid;

      auto multifield = uuid_field.multifieldValue;

      if (multifield->length != 16) {
        RCLCPP_ERROR(*instance->logger_, "Invalid UUID passed, expected multifield of 16 integers");
        res.lexemeValue = clips::CreateSymbol(env, "FALSE");
        *out = res;
        return;
      }

      for (size_t i = 0; i < 16; ++i) {
        auto val = multifield->contents[i];
        uuid[i] = val.integerValue->contents;
      }

      std::string uuid_str = rclcpp_action::to_string(uuid);

      res.lexemeValue = clips::CreateSymbol(env, uuid_str.c_str());
      *out = res;
    },
    "goal_uuid_to_string", this);

#ifdef HAVE_GENERIC_CLIENT
  init_generic_client_bindings(env);
#endif

#ifdef HAVE_GENERIC_SERVICE
  init_generic_service_bindings(env);
#endif

#ifdef HAVE_GENERIC_ACTION_CLIENT
  init_generic_action_client_bindings(env);
#endif

  // add fact templates
  clips::Build(
    env.get(),
    R"((deftemplate ros-msgs-subscription
            (slot topic (type STRING))
            (slot type (type STRING))))");
  clips::Build(
    env.get(),
    R"((deftemplate ros-msgs-publisher
            (slot topic (type STRING))
            (slot type (type STRING))))");
  clips::Build(
    env.get(),
    R"((deftemplate ros-msgs-message
            (slot topic (type STRING))
            (slot msg-ptr (type EXTERNAL-ADDRESS))
      ))");

  return true;
}

void RosMsgsPlugin::clips_to_ros_value(
  clips::Environment * env, const clips::CLIPSValue & val,
  const rosidl_typesupport_introspection_cpp::MessageMember & member, uint8_t * field_ptr)
{
  switch (member.type_id_) {
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL: {
      bool * bool_field = reinterpret_cast<bool *>(field_ptr);
      *bool_field = (strcmp(val.lexemeValue->contents, "TRUE") == 0);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8: {
      int8_t * int8_field = reinterpret_cast<int8_t *>(field_ptr);
      *int8_field = static_cast<int8_t>(val.integerValue->contents);
      break;
    }
    // case rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_OCTET:
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8: {
      uint8_t * uint8_field = reinterpret_cast<uint8_t *>(field_ptr);
      *uint8_field = static_cast<uint8_t>(val.integerValue->contents);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16: {
      int16_t * int16_field = reinterpret_cast<int16_t *>(field_ptr);
      *int16_field = static_cast<int16_t>(val.integerValue->contents);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16: {
      uint16_t * uint16_field = reinterpret_cast<uint16_t *>(field_ptr);
      *uint16_field = static_cast<uint16_t>(val.integerValue->contents);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32: {
      int32_t * int32_field = reinterpret_cast<int32_t *>(field_ptr);
      *int32_field = static_cast<int32_t>(val.integerValue->contents);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32: {
      uint32_t * uint32_field = reinterpret_cast<uint32_t *>(field_ptr);
      *uint32_field = static_cast<uint32_t>(val.integerValue->contents);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64: {
      int64_t * int64_field = reinterpret_cast<int64_t *>(field_ptr);
      *int64_field = static_cast<int64_t>(val.integerValue->contents);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64: {
      uint64_t * uint64_field = reinterpret_cast<uint64_t *>(field_ptr);
      *uint64_field = static_cast<uint64_t>(val.integerValue->contents);
      break;
    }

    // case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT: {
      float * float_field = reinterpret_cast<float *>(field_ptr);
      *float_field = static_cast<float>(val.floatValue->contents);
      break;
    }
    // case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE: {
      double * double_field = reinterpret_cast<double *>(field_ptr);
      *double_field = static_cast<double>(val.floatValue->contents);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_LONG_DOUBLE: {
      long double * long_double_field = reinterpret_cast<long double *>(field_ptr);
      *long_double_field = static_cast<double>(val.floatValue->contents);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR: {
      unsigned char * char_field = reinterpret_cast<unsigned char *>(field_ptr);
      *char_field = static_cast<unsigned char>(val.lexemeValue->contents[0]);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING: {
      std::string * string_field = reinterpret_cast<std::string *>(field_ptr);
      const char * clips_string = val.lexemeValue->contents;
      *string_field = std::string(clips_string);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_WCHAR: {
      char16_t * char16_t_field = reinterpret_cast<char16_t *>(field_ptr);
      std::string utf8_str = std::string(val.lexemeValue->contents);
      icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(utf8_str);
      std::u16string u16_str(
        reinterpret_cast<const char16_t *>(unicode_str.getBuffer()), unicode_str.length());
      *char16_t_field = u16_str[0];
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_WSTRING: {
      std::u16string * u16string_field = reinterpret_cast<std::u16string *>(field_ptr);
      std::string utf8_str = std::string(val.lexemeValue->contents);
      icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(utf8_str);
      std::u16string u16_str(
        reinterpret_cast<const char16_t *>(unicode_str.getBuffer()), unicode_str.length());
      *u16string_field = u16_str;
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE: {
      void * target = reinterpret_cast<void *>(field_ptr);
      if (messages_.contains(val.externalAddressValue->contents)) {
        std::shared_ptr<MessageInfo> msg_info = messages_[val.externalAddressValue->contents];
        move_field_to_parent(target, &member, msg_info->msg_ptr);
      } else {
        RCLCPP_ERROR(*logger_, "Failed to set unknown message");
      }
      break;
    }
    default:
      clips::Writeln(env, "Unsupported field type for setting value");
      // clips::UDFThrowError(udfc);
      throw std::runtime_error("Unsupported field type for setting value.");
  }
}

void RosMsgsPlugin::move_field_to_parent(
  void * parent_msg, const rosidl_typesupport_introspection_cpp::MessageMember * parent_member,
  void * source_msg)
{
  // Access the submessage in the parent at the specified offset
  void * target_submsg = parent_msg;

  // Obtain introspection information for the type of the submessage
  const rosidl_typesupport_introspection_cpp::MessageMembers * sub_members =
    static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
      parent_member->members_->data);

  // Move fields from the source sub-message to the target sub-message in parent
  for (uint32_t i = 0; i < sub_members->member_count_; ++i) {
    const rosidl_typesupport_introspection_cpp::MessageMember * sub_member =
      &sub_members->members_[i];

    // Calculate the offset for the source and target members
    void * source_field_ptr = reinterpret_cast<uint8_t *>(source_msg) + sub_member->offset_;
    void * target_field_ptr = reinterpret_cast<uint8_t *>(target_submsg) + sub_member->offset_;

    switch (sub_member->type_id_) {
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL: {
        if (sub_member->is_array_) {
          if (!sub_member->is_upper_bound_) {
            // Dynamically-sized array of bool (std::vector<bool>)
            auto * source_vector = reinterpret_cast<std::vector<bool> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<bool> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
          } else {
            // Fixed-size array
            std::memcpy(target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(bool));
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
            auto * source_vector = reinterpret_cast<std::vector<int8_t> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<int8_t> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
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
            auto * source_vector = reinterpret_cast<std::vector<uint8_t> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<uint8_t> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
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
            auto * source_vector = reinterpret_cast<std::vector<int16_t> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<int16_t> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
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
            auto * source_vector = reinterpret_cast<std::vector<uint16_t> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<uint16_t> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
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
            auto * source_vector = reinterpret_cast<std::vector<int32_t> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<int32_t> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
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
            auto * source_vector = reinterpret_cast<std::vector<uint32_t> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<uint32_t> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
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
            auto * source_vector = reinterpret_cast<std::vector<int64_t> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<int64_t> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
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
            auto * source_vector = reinterpret_cast<std::vector<uint64_t> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<uint64_t> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
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
            auto * source_vector = reinterpret_cast<std::vector<float> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<float> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
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
            auto * source_vector = reinterpret_cast<std::vector<double> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<double> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
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
            auto * source_vector = reinterpret_cast<std::vector<long double> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<long double> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
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
            auto * source_vector = reinterpret_cast<std::vector<std::string> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<std::string> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
          } else {
            // Fixed-size array of strings
            auto source_array = reinterpret_cast<std::string *>(source_field_ptr);
            auto target_array = reinterpret_cast<std::string *>(target_field_ptr);
            for (size_t i = 0; i < sub_member->array_size_; ++i) {
              target_array[i] = std::move(source_array[i]);
              source_array[i] = {};
            }
          }
        } else {
          // Single string (std::string)
          auto * source_str = reinterpret_cast<std::string *>(source_field_ptr);
          auto * target_str = reinterpret_cast<std::string *>(target_field_ptr);
          *target_str = std::move(*source_str);
        }
        break;
      }
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR: {
        if (sub_member->is_array_) {
          if (!sub_member->is_upper_bound_) {
            // Dynamically-sized array (std::vector<char>)
            auto * source_vector = reinterpret_cast<std::vector<char> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<char> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
          } else {
            // Fixed-size array
            std::memcpy(target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(char));
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
            auto * source_vector = reinterpret_cast<std::vector<wchar_t> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<wchar_t> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
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
            auto * source_vector = reinterpret_cast<std::vector<wchar_t> *>(source_field_ptr);
            auto * target_vector = reinterpret_cast<std::vector<wchar_t> *>(target_field_ptr);
            *target_vector = std::move(*source_vector);
            *source_vector = {};
          } else {
            // Fixed-size array
            std::memcpy(
              target_field_ptr, source_field_ptr, sub_member->array_size_ * sizeof(wchar_t));
          }
        } else {
          // Single wstring (using std::wstring)
          auto * source_string = reinterpret_cast<std::wstring *>(source_field_ptr);
          auto * target_string = reinterpret_cast<std::wstring *>(target_field_ptr);
          *target_string = std::move(*source_string);
          *source_string = {};
        }
        break;
      }

      case rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE: {
        const auto * sub_members =
          static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
            sub_member->members_->data);

        if (sub_member->is_array_) {
          if (!sub_member->is_upper_bound_) {
            // Dynamically-sized array of sub-messages
            auto * source_vector = reinterpret_cast<std::vector<void *> *>(source_msg);
            auto * target_vector = reinterpret_cast<std::vector<void *> *>(target_field_ptr);
            target_vector->resize(source_vector->size());

            for (size_t i = 0; i < source_vector->size(); ++i) {
              // Iterate over each member in the nested message
              for (size_t j = 0; j < sub_members->member_count_; ++j) {
                const auto & sub_member = sub_members->members_[j];
                move_field_to_parent(target_vector->at(i), &sub_member, source_vector->at(i));
              }
            }
          } else {
            // Fixed-size array of sub-messages
            auto source_array = reinterpret_cast<void **>(source_msg);
            auto target_array = reinterpret_cast<void **>(target_field_ptr);

            for (size_t i = 0; i < parent_member->array_size_; ++i) {
              for (size_t j = 0; j < sub_members->member_count_; ++j) {
                const auto & sub_member = sub_members->members_[j];
                move_field_to_parent(target_array[i], &sub_member, source_array[i]);
              }
            }
          }
        } else {
          // Single nested sub-message
          move_field_to_parent(target_field_ptr, sub_member, source_field_ptr);
        }
        break;
      }

      default:
        throw std::runtime_error("Unsupported field type.");
    }
  }
}

bool RosMsgsPlugin::clips_env_destroyed(std::shared_ptr<clips::Environment> & env)
{
  for (const auto & fun : function_names_) {
    clips::RemoveUDF(env.get(), fun.c_str());
  }
  clips::Deftemplate * curr_tmpl = clips::FindDeftemplate(env.get(), "ros-msgs-subscription");
  if (curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_, "ros-msgs-subscription cant be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "ros-msgs-publisher");
  if (curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_, "ros-msgs-publisher cant be undefined");
  }
  curr_tmpl = clips::FindDeftemplate(env.get(), "ros-msgs-message");
  if (curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  } else {
    RCLCPP_WARN(*logger_, "ros-msgs-message cant be undefined");
  }
#ifdef HAVE_GENERIC_CLIENT
  destroy_generic_client_bindings(env);
#endif
#ifdef HAVE_GENERIC_SERVICE
  destroy_generic_service_bindings(env);
#endif
#ifdef HAVE_GENERIC_ACTION_CLIENT
  destroy_generic_action_client_bindings(env);
#endif
  return true;
}

void RosMsgsPlugin::subscribe_to_topic(
  clips::Environment * env, const std::string & topic_name, const std::string & topic_type)
{
  RCLCPP_DEBUG(*logger_, "Subscribing to topic %s %s", topic_name.c_str(), topic_type.c_str());
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;
  auto node = parent_.lock();
  if (!node) {
    RCLCPP_ERROR(*logger_, "Invalid reference to parent node");
    return;
  }

  auto it = subscriptions_[env_name].find(topic_name);

  if (it != subscriptions_[env_name].end()) {
    RCLCPP_WARN(*logger_, "Already subscribed to topic %s", topic_name.c_str());
  } else {
    RCLCPP_DEBUG(*logger_, "Creating subscription to topic %s", topic_name.c_str());
    auto options = rclcpp::SubscriptionOptions();
    options.callback_group = cb_group_;
    {
      std::scoped_lock map_lock{map_mtx_};
      if (!libs_.contains(topic_type)) {
        libs_[topic_type] = rclcpp::get_typesupport_library(topic_type, "rosidl_typesupport_cpp");
      }
      if (!type_support_cache_.contains(topic_type)) {
        type_support_cache_[topic_type] = rclcpp::get_message_typesupport_handle(
          topic_type, "rosidl_typesupport_cpp", *libs_[topic_type]);
      }

      subscriptions_[env_name][topic_name] = node->create_generic_subscription(
        topic_name, topic_type, rclcpp::QoS(10),
        [this, env, topic_name, topic_type](std::shared_ptr<const rclcpp::SerializedMessage> msg) {
          topic_callback(msg, topic_name, topic_type, env);
        },
        options);
    }
    clips::AssertString(
      env, ("(ros-msgs-subscription (topic \"" + topic_name + "\") (type \"" + topic_type + "\"))")
             .c_str());
  }
}

void RosMsgsPlugin::unsubscribe_from_topic(clips::Environment * env, const std::string & topic_name)
{
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;

  map_mtx_.lock();
  auto it = subscriptions_[env_name].find(topic_name);

  if (it != subscriptions_[env_name].end()) {
    map_mtx_.unlock();
    RCLCPP_DEBUG(
      rclcpp::get_logger(plugin_name_), "Unsubscribing from topic %s", topic_name.c_str());
    subscriptions_[env_name].erase(topic_name);
  } else {
    RCLCPP_WARN(
      rclcpp::get_logger(plugin_name_), "No subscription of %s found to destroy, ignoring",
      topic_name.c_str());
  }

  map_mtx_.unlock();
  clips::Eval(
    env,
    ("(do-for-all-facts ((?f ros-msgs-subscription)) (eq (str-cat "
     "?f:topic) (str-cat \"" +
     topic_name + "\"))  (retract ?f))")
      .c_str(),
    NULL);
}

std::shared_ptr<RosMsgsPlugin::MessageInfo> RosMsgsPlugin::create_deserialized_msg(
  const std::string & msg_type)
{
  if (!libs_.contains(msg_type)) {
    RCLCPP_DEBUG(*logger_, "Create new message information on message creation");
    libs_[msg_type] = rclcpp::get_typesupport_library(msg_type, "rosidl_typesupport_cpp");
    type_support_cache_[msg_type] =
      rclcpp::get_message_typesupport_handle(msg_type, "rosidl_typesupport_cpp", *libs_[msg_type]);
  }
  auto * introspection_info = get_msg_members(msg_type);
  return std::make_shared<RosMsgsPlugin::MessageInfo>(introspection_info);
}

std::shared_ptr<RosMsgsPlugin::MessageInfo> RosMsgsPlugin::deserialize_msg(
  std::shared_ptr<const rclcpp::SerializedMessage> msg, const std::string & msg_type)
{
  // Prepare meta data for deserialization
  rcutils_uint8_array_t serialized_data;
  serialized_data.buffer = const_cast<uint8_t *>(msg->get_rcl_serialized_message().buffer);
  serialized_data.buffer_length = msg->get_rcl_serialized_message().buffer_length;
  serialized_data.buffer_capacity = msg->get_rcl_serialized_message().buffer_capacity;

  // Perform the deserialization
  std::shared_ptr<MessageInfo> deserialized_msg = create_deserialized_msg(msg_type);
  rmw_ret_t result =
    rmw_deserialize(&serialized_data, type_support_cache_[msg_type], deserialized_msg->msg_ptr);

  if (result != RMW_RET_OK) {
    RCLCPP_ERROR(*logger_, "Error while deserializing message");
    return std::shared_ptr<MessageInfo>();
  }
  return deserialized_msg;
}

void RosMsgsPlugin::topic_callback(
  std::shared_ptr<const rclcpp::SerializedMessage> msg, const std::string & topic_name,
  const std::string & msg_type, clips::Environment * env)
{
  auto context = CLIPSEnvContext::get_context(env);
  std::scoped_lock clips_lock{context->env_mtx_};
  std::shared_ptr<MessageInfo> deserialized_msg;
  {
    std::scoped_lock map_lock{map_mtx_};
    deserialized_msg = deserialize_msg(msg, msg_type);
    if (!deserialized_msg) {
      RCLCPP_ERROR(
        *logger_, "failed to process msg (topic: %s type: %s)", topic_name.c_str(),
        msg_type.c_str());
      return;
    }

    messages_[deserialized_msg.get()] = deserialized_msg;
  }

  // assert the newest message
  clips::FactBuilder * fact_builder = clips::CreateFactBuilder(env, "ros-msgs-message");
  clips::FBPutSlotString(fact_builder, "topic", topic_name.c_str());
  clips::FBPutSlotString(fact_builder, "type", msg_type.c_str());
  clips::FBPutSlotCLIPSExternalAddress(
    fact_builder, "msg-ptr", clips::CreateCExternalAddress(env, deserialized_msg.get()));
  clips::FBAssert(fact_builder);
  clips::FBDispose(fact_builder);
}

void RosMsgsPlugin::create_new_publisher(
  clips::Environment * env, const std::string & topic_name, const std::string & topic_type)
{
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;

  auto node = parent_.lock();
  if (!node) {
    RCLCPP_ERROR(*logger_, "Invalid reference to parent node");
  }

  std::map<std::__cxx11::basic_string<char>, std::shared_ptr<rclcpp::GenericPublisher>>::iterator
    it;
  {
    map_mtx_.lock();
    it = publishers_[env_name].find(topic_name);
  }

  if (it != publishers_[env_name].end()) {
    map_mtx_.unlock();
    RCLCPP_WARN(*logger_, "Already publishing to topic %s", topic_name.c_str());
  } else {
    RCLCPP_DEBUG(*logger_, "Creating publisher to topic %s", topic_name.c_str());
    auto options = rclcpp::PublisherOptions();
    options.callback_group = cb_group_;

    publishers_[context->env_name_][topic_name] =
      node->create_generic_publisher(topic_name, topic_type, rclcpp::QoS(10), options);
    map_mtx_.unlock();
    clips::AssertString(
      env, ("(ros-msgs-publisher (topic \"" + topic_name + "\") (type \"" + topic_type + "\"))")
             .c_str());
  }
}

void RosMsgsPlugin::destroy_publisher(clips::Environment * env, const std::string & topic_name)
{
  auto context = CLIPSEnvContext::get_context(env);
  std::string env_name = context->env_name_;
  map_mtx_.lock();
  auto outer_it = publishers_.find(env_name);
  if (outer_it != publishers_.end()) {
    // Check if topic_name exists in the inner map
    auto & inner_map = outer_it->second;
    auto inner_it = inner_map.find(topic_name);
    if (inner_it != inner_map.end()) {
      // Remove the topic_name entry from the inner map
      RCLCPP_DEBUG(*logger_, "Destroying publisher for topic %s", topic_name.c_str());
      inner_map.erase(inner_it);
      map_mtx_.unlock();
      clips::Eval(
        env,
        ("(do-for-all-facts ((?f ros-msgs-publisher)) (eq (str-cat "
         "?f:topic) (str-cat \"" +
         topic_name + "\"))  (retract ?f))")
          .c_str(),
        NULL);
    } else {
      map_mtx_.unlock();
      RCLCPP_WARN(
        *logger_, "Publisher %s not found in environment %s", topic_name.c_str(), env_name.c_str());
    }
  } else {
    map_mtx_.unlock();
    RCLCPP_WARN(*logger_, "Environment %s not found", env_name.c_str());
  }
}

rclcpp::SerializedMessage RosMsgsPlugin::serialize_msg(
  std::shared_ptr<MessageInfo> msg_info, const std::string & msg_type)
{
  rmw_serialized_message_t serialized_msg = rmw_get_zero_initialized_serialized_message();

  const size_t initial_capacity = msg_info->members->size_of_;
  ;
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rmw_serialized_message_init(&serialized_msg, initial_capacity, &allocator);

  // Use the RMW API to serialize the message
  rmw_ret_t ret = rmw_serialize(msg_info->msg_ptr, type_support_cache_[msg_type], &serialized_msg);

  rclcpp::SerializedMessage typed_serialized_msg;
  if (ret != RMW_RET_OK) {
    // Handle serialization failure
    RCLCPP_ERROR(*logger_, "Failed to serialize message: %s\n", rmw_get_error_string().str);
    return typed_serialized_msg;
  }
  typed_serialized_msg.reserve(serialized_msg.buffer_capacity);
  typed_serialized_msg.get_rcl_serialized_message().buffer = serialized_msg.buffer;
  typed_serialized_msg.get_rcl_serialized_message().buffer_capacity =
    serialized_msg.buffer_capacity;
  typed_serialized_msg.get_rcl_serialized_message().buffer_length = serialized_msg.buffer_length;
  return typed_serialized_msg;
}

void RosMsgsPlugin::publish_to_topic(
  clips::Environment * env, void * deserialized_msg, const std::string & topic_name)
{
  auto scoped_lock = std::scoped_lock{map_mtx_};
  auto context = CLIPSEnvContext::get_context(env);

  if (!messages_.contains(deserialized_msg)) {
    RCLCPP_ERROR(*logger_, "Failed to publish invalid msg pointer");
    return;
  }
  std::shared_ptr<MessageInfo> msg_info = messages_[deserialized_msg];
  std::string msg_type = get_msg_type(msg_info->members);
  rclcpp::SerializedMessage serialized_msg = serialize_msg(msg_info, msg_type);
  if (serialized_msg.capacity() == 0) {
    RCLCPP_ERROR(*logger_, "Failed to publish due to serialization error");
  }
  publishers_[context->env_name_][topic_name]->publish(serialized_msg);
}

clips::UDFValue RosMsgsPlugin::create_message(clips::Environment * env, const std::string & type)
{
  auto scoped_lock = std::scoped_lock{map_mtx_};
  std::shared_ptr<MessageInfo> ptr = create_deserialized_msg(type);
  messages_[ptr.get()] = ptr;
  clips::UDFValue res;
  res.externalAddressValue =
    clips::CreateCExternalAddress(env, reinterpret_cast<void *>(ptr.get()));
  return res;
}

void RosMsgsPlugin::destroy_msg(void * msg)
{
  auto scoped_lock = std::scoped_lock{map_mtx_};
  auto sub_it = sub_messages_.find(msg);
  if (sub_it != sub_messages_.end()) {
    for (const auto & sub_msg : sub_it->second) {
      messages_.erase(sub_msg.get());
    }
    sub_messages_.erase(sub_it);
  }
  auto it = messages_.find(msg);
  if (it != messages_.end()) {
    messages_.erase(it);
  }
}

const rosidl_typesupport_introspection_cpp::MessageMembers * RosMsgsPlugin::get_msg_members(
  const std::string & msg_type)
{
  if (type_support_cache_.contains(msg_type)) {
    auto * introspection_type_support = get_message_typesupport_handle(
      type_support_cache_[msg_type], rosidl_typesupport_introspection_cpp::typesupport_identifier);
    return static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
      introspection_type_support->data);
  }
  RCLCPP_ERROR(*logger_, "type support information for type %s missing", msg_type.c_str());
  return nullptr;
}

std::string RosMsgsPlugin::get_msg_type(
  const rosidl_typesupport_introspection_cpp::MessageMembers * members)
{
  // Get the members of the nested message
  std::string message_type =
    std::string(members->message_namespace_) + "::" + members->message_name_;
  size_t start_pos = 0;
  while ((start_pos = message_type.find("::", start_pos)) != std::string::npos) {
    message_type.replace(start_pos, 2, "/");
    start_pos += 1;
  }
  return message_type;
}

std::shared_ptr<RosMsgsPlugin::MessageInfo> RosMsgsPlugin::process_nested_msg(
  void * nested_msg, const rosidl_typesupport_introspection_cpp::MessageMember & member)
{
  // Access the type support of the nested message
  const rosidl_message_type_support_t * type_support = member.members_;
  if (type_support) {
    // Get the members of the nested message
    const rosidl_typesupport_introspection_cpp::MessageMembers * nested_members =
      static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(type_support->data);
    std::string message_type = get_msg_type(nested_members);
    if (!libs_.contains(message_type)) {
      libs_[message_type] = rclcpp::get_typesupport_library(message_type, "rosidl_typesupport_cpp");
    }
    if (!type_support_cache_.contains(message_type)) {
      type_support_cache_[message_type] = rclcpp::get_message_typesupport_handle(
        message_type, "rosidl_typesupport_cpp", *libs_[message_type]);
    }
    return std::make_shared<MessageInfo>(nested_members, nested_msg);
  }
  return std::shared_ptr<MessageInfo>();
}

clips::UDFValue RosMsgsPlugin::ros_msg_member_to_udf_value(
  clips::Environment * env, std::shared_ptr<MessageInfo> & msg_info,
  const rosidl_typesupport_introspection_cpp::MessageMember & member)
{
  void * deserialized_msg = msg_info->msg_ptr;
  clips::UDFValue res;
  res.begin = 0;
  res.range = -1;
  // If member is an array, create a multifield
  if (member.is_array_) {
    size_t array_size = member.array_size_;
    void * member_loc = reinterpret_cast<uint8_t *>(deserialized_msg) + member.offset_;

    if (array_size == 0) {
      array_size = member.size_function(member_loc);
    }
    clips::MultifieldBuilder * mb = clips::CreateMultifieldBuilder(env, array_size);
    clips::Multifield * mf;

    for (size_t i = 0; i < array_size; ++i) {
      void * element = member.get_function(member_loc, i);
      if (!element) {
        RCLCPP_ERROR(*logger_, "Failed to retrieve element");
        continue;  // Skip if the element is null
      }
      clips::CLIPSValue elem = ros_to_clips_value(env, element, member.type_id_);
      // properly store nested message and ensure the CLIPS value reflects that
      if (member.type_id_ == rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE) {
        std::shared_ptr<MessageInfo> sub_msg_info =
          process_nested_msg(elem.externalAddressValue->contents, member);
        if (sub_msg_info) {
          sub_messages_[msg_info.get()].push_back(sub_msg_info);
          messages_[sub_msg_info.get()] = sub_msg_info;
          elem.externalAddressValue->contents = sub_msg_info.get();
        } else {
          RCLCPP_ERROR(*logger_, "Error while processing nested message");
        }
      }
      clips::MBAppend(mb, &elem);
    }
    mf = clips::MBCreate(mb);
    clips::MBDispose(mb);
    res.multifieldValue = mf;
  } else {
    void * value =
      reinterpret_cast<void *>(reinterpret_cast<uint8_t *>(deserialized_msg) + member.offset_);
    clips::CLIPSValue elem = ros_to_clips_value(env, value, member.type_id_);
    // properly store nested message and ensure the CLIPS value reflects that
    if (member.type_id_ == rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE) {
      std::shared_ptr<MessageInfo> sub_msg_info =
        process_nested_msg(elem.externalAddressValue->contents, member);
      if (sub_msg_info) {
        sub_messages_[msg_info.get()].push_back(sub_msg_info);
        messages_[sub_msg_info.get()] = sub_msg_info;
        elem.externalAddressValue->contents = sub_msg_info.get();
      } else {
        RCLCPP_ERROR(*logger_, "Error while processing nested message");
      }
    }
    res.value = elem.value;
  }
  return res;
}

clips::CLIPSValue RosMsgsPlugin::ros_to_clips_value(
  clips::Environment * env, void * val, uint8_t ros_type)
{
  clips::CLIPSValue res;
  switch (ros_type) {
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL: {
      bool * bool_value = reinterpret_cast<bool *>(val);
      res.value = clips::CreateBoolean(env, *bool_value);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8: {
      int8_t * int8_value = reinterpret_cast<int8_t *>(val);
      res.value = clips::CreateInteger(env, *int8_value);
      break;
    }
    // case rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_OCTET:
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8: {
      uint8_t * uint8_value = reinterpret_cast<uint8_t *>(val);
      res.value = clips::CreateInteger(env, *uint8_value);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16: {
      int16_t * int16_value = reinterpret_cast<int16_t *>(val);
      res.value = clips::CreateInteger(env, *int16_value);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16: {
      uint16_t * uint16_value = reinterpret_cast<uint16_t *>(val);
      res.value = clips::CreateInteger(env, *uint16_value);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32: {
      int32_t * int32_value = reinterpret_cast<int32_t *>(val);
      res.value = clips::CreateInteger(env, *int32_value);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32: {
      uint32_t * uint32_value = reinterpret_cast<uint32_t *>(val);
      res.value = clips::CreateInteger(env, *uint32_value);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64: {
      int64_t * int64_value = reinterpret_cast<int64_t *>(val);
      res.value = clips::CreateInteger(env, *int64_value);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64: {
      uint64_t * uint64_value = reinterpret_cast<uint64_t *>(val);
      res.value = clips::CreateInteger(env, *uint64_value);
      break;
    }
    // case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT: {
      float * float_value = reinterpret_cast<float *>(val);
      res.value = clips::CreateFloat(env, *float_value);
      break;
    }
    // case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE: {
      double * double_value = static_cast<double *>(val);
      res.value = clips::CreateFloat(env, *double_value);
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_LONG_DOUBLE: {
      long double * long_double_value = reinterpret_cast<long double *>(val);
      res.value = clips::CreateFloat(env, static_cast<double>(*long_double_value));
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR: {
      unsigned char * char_value = reinterpret_cast<unsigned char *>(val);
      std::string str_value = std::string(1, *char_value);
      res.value = clips::CreateString(env, str_value.c_str());
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING: {
      std::string * str_value = reinterpret_cast<std::string *>(val);
      res.value = clips::CreateString(env, str_value->c_str());
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_WCHAR: {
      char16_t * char16_t_value = reinterpret_cast<char16_t *>(val);
      std::u16string u16_str = std::u16string(1, *char16_t_value);
      icu::UnicodeString unicode_str(
        reinterpret_cast<const UChar *>(u16_str.data()), u16_str.length());
      std::string utf8_str;
      unicode_str.toUTF8String(utf8_str);
      res.value = clips::CreateString(env, utf8_str.c_str());
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_WSTRING: {
      std::u16string * u16string_value = reinterpret_cast<std::u16string *>(val);
      icu::UnicodeString unicode_str(
        reinterpret_cast<const UChar *>(u16string_value->data()), u16string_value->length());
      std::string utf8_str;
      unicode_str.toUTF8String(utf8_str);
      res.value = clips::CreateString(env, utf8_str.c_str());
      break;
    }
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE: {
      res.value = clips::CreateCExternalAddress(env, val);
      break;
    }
    default:
      throw std::runtime_error("Unsupported field type.");
  }
  return res;
}

clips::UDFValue RosMsgsPlugin::get_field(
  clips::Environment * env, void * deserialized_msg, const std::string & field,
  clips::UDFContext * udfc)
{
  auto scoped_lock = std::scoped_lock{map_mtx_};
  clips::UDFValue res;
  res.begin = 0;
  res.range = -1;

  if (!deserialized_msg || !messages_.contains(deserialized_msg)) {
    RCLCPP_ERROR(*logger_, "ros-msgs-get-field: Invalid pointer ");
    clips::UDFThrowError(udfc);
    return res;
  }
  std::shared_ptr<MessageInfo> info = messages_[deserialized_msg];
  const rosidl_typesupport_introspection_cpp::MessageMembers * members = info->members;
  std::string message_type = get_msg_type(members);

  // Search for the field in the message
  for (size_t i = 0; i < members->member_count_; ++i) {
    const auto & member = members->members_[i];
    if (field == member.name_) {
      return ros_msg_member_to_udf_value(env, info, member);
    }
  }

  RCLCPP_ERROR(*logger_, "Failed to retrieve field %s", field.c_str());
  return res;
}

void RosMsgsPlugin::set_field(
  clips::Environment * env, void * deserialized_msg, const std::string & field,
  clips::UDFValue & val, clips::UDFContext * udfc)
{
  auto scoped_lock = std::scoped_lock(map_mtx_);
  if (!deserialized_msg || !messages_.contains(deserialized_msg)) {
    RCLCPP_ERROR(*logger_, "ros-msgs-get-field: Invalid pointer");
    clips::UDFThrowError(udfc);
    return;
  }
  std::shared_ptr<MessageInfo> info = messages_[deserialized_msg];
  const rosidl_typesupport_introspection_cpp::MessageMembers * members = info->members;
  std::string message_type = get_msg_type(members);

  // Search for the field in the message
  for (size_t i = 0; i < members->member_count_; ++i) {
    const auto & member = members->members_[i];
    if (field == member.name_) {
      udf_value_to_ros_message_member(env, info->msg_ptr, member, val);
      return;
    }
  }
  RCLCPP_ERROR(*logger_, "Failed to set field %s", field.c_str());
}

void RosMsgsPlugin::udf_value_to_ros_message_member(
  clips::Environment * env, void * deserialized_msg,
  const rosidl_typesupport_introspection_cpp::MessageMember & member, clips::UDFValue & val)
{
  // Check if the member is an array
  uint8_t * field_ptr = static_cast<uint8_t *>(deserialized_msg) + member.offset_;
  if (member.is_array_) {
    if (val.header->type != MULTIFIELD_TYPE) {
      RCLCPP_ERROR(*logger_, "expected multifield value!");
      return;
    }
    member.resize_function(field_ptr, val.multifieldValue->length);
    uint8_t * field_index_ptr;
    for (size_t i = 0; i < val.multifieldValue->length; i++) {
      field_index_ptr = static_cast<uint8_t *>(member.get_function(field_ptr, i));
      clips_to_ros_value(env, val.multifieldValue->contents[i], member, field_index_ptr);
    }
  } else {
    clips::CLIPSValue clips_val;
    clips_val.value = val.value;
    clips_to_ros_value(env, clips_val, member, field_ptr);
  }
}

}  // namespace cx

PLUGINLIB_EXPORT_CLASS(cx::RosMsgsPlugin, cx::ClipsPlugin)
