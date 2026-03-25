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

#include "cx_ros_param_plugin/ros_param_plugin.hpp"

#include <string>

#include "cx_utils/clips_env_context.hpp"
#include "cx_utils/param_utils.hpp"

// To export as plugin
#include "pluginlib/class_list_macros.hpp"

using std::placeholders::_1;
namespace cx
{
RosParamPlugin::RosParamPlugin() {}

RosParamPlugin::~RosParamPlugin() {}

void RosParamPlugin::initialize()
{
  logger_ = std::make_unique<rclcpp::Logger>(rclcpp::get_logger(plugin_name_));
}

bool RosParamPlugin::clips_env_init(std::shared_ptr<clips::Environment> & env)
{
  auto context = CLIPSEnvContext::get_context(env.get());
  RCLCPP_INFO(*logger_, "Initializing plugin for environment %s", context->env_name_.c_str());
  clips::AddUDF(
    env.get(), "ros-param-get-value", "*", 2, 2, ";sy;*",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto instance = static_cast<RosParamPlugin *>(udfc->context);
      clips::UDFValue param_name;
      clips::UDFValue default_value;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &param_name);
      clips::UDFNthArgument(
        udfc, 2, LEXEME_BITS | NUMBER_BITS | MULTIFIELD_BIT | BOOLEAN_BIT, &default_value);
      *out = instance->get_ros_param(env, param_name.lexemeValue->contents, default_value);
    },
    "ros_param_get_value", this);

  return true;
}

bool RosParamPlugin::clips_env_destroyed(std::shared_ptr<clips::Environment> & env)
{
  clips::RemoveUDF(env.get(), "ros-param-get-value");
  return true;
}

clips::UDFValue RosParamPlugin::get_ros_param(
  clips::Environment * env, std::string param_name, clips::UDFValue & default_value)
{
  clips::UDFValue res;
  res.begin = 0;
  res.range = -1;
  auto node = parent_.lock();

  switch (default_value.header->type) {
    case INTEGER_TYPE: {
      int64_t default_int = default_value.integerValue->contents;
      int64_t result;
      cx_utils::declare_parameter_if_not_declared(
        node, param_name, rclcpp::ParameterValue(default_int));
      node->get_parameter(param_name, result);
      res.value = clips::CreateInteger(env, result);
      return res;
    }
    case FLOAT_TYPE: {
      double default_double = default_value.floatValue->contents;
      double result;
      cx_utils::declare_parameter_if_not_declared(
        node, param_name, rclcpp::ParameterValue(default_double));
      node->get_parameter(param_name, result);
      res.value = clips::CreateFloat(env, result);
      return res;
    }

    case STRING_TYPE:
    case SYMBOL_TYPE: {
      std::string def = default_value.lexemeValue->contents;
      // Handle boolean as TRUE/FALSE symbol
      if (def == "TRUE" || def == "FALSE") {
        bool default_bool = (def == "TRUE");
        bool result;
        cx_utils::declare_parameter_if_not_declared(
          node, param_name, rclcpp::ParameterValue(default_bool));
        node->get_parameter(param_name, result);
        res.value = clips::CreateBoolean(env, result);
        return res;
      }

      // Otherwise treat as string
      std::string result;
      cx_utils::declare_parameter_if_not_declared(node, param_name, rclcpp::ParameterValue(def));
      node->get_parameter(param_name, result);
      res.value = clips::CreateString(env, result.c_str());
      return res;
    }
    case MULTIFIELD_TYPE: {
      clips::MultifieldBuilder * mb = clips::CreateMultifieldBuilder(env, 0);
      const auto & mf = *default_value.multifieldValue;
      size_t n = mf.length;

      if (n == 0) {
        // Ambiguous: treat as string array
        std::vector<std::string> def, result;
        cx_utils::declare_parameter_if_not_declared(node, param_name, rclcpp::ParameterValue(def));
        node->get_parameter(param_name, result);
        for (const auto & s : result) MBAppendString(mb, s.c_str());
        res.multifieldValue = clips::MBCreate(mb);
        break;
      }

      // Inspect type of first element to disambiguate
      auto & first = mf.contents[0];

      if (first.header->type == INTEGER_TYPE) {
        std::vector<int64_t> def, result;
        for (size_t i = 0; i < n; ++i) def.push_back(mf.contents[i].integerValue->contents);

        cx_utils::declare_parameter_if_not_declared(node, param_name, rclcpp::ParameterValue(def));
        node->get_parameter(param_name, result);

        for (auto v : result) {
          clips::MBAppendInteger(mb, v);
        }
        res.multifieldValue = clips::MBCreate(mb);
      }

      if (first.header->type == FLOAT_TYPE) {
        std::vector<double> def, result;
        for (size_t i = 0; i < n; ++i) def.push_back(mf.contents[i].floatValue->contents);

        cx_utils::declare_parameter_if_not_declared(node, param_name, rclcpp::ParameterValue(def));
        node->get_parameter(param_name, result);

        for (auto v : result) {
          clips::MBAppendFloat(mb, v);
        }
        res.multifieldValue = clips::MBCreate(mb);
      }

      if (first.header->type == SYMBOL_TYPE || first.header->type == STRING_TYPE) {
        bool all_bool = true;
        std::vector<bool> bool_def;

        for (size_t i = 0; i < n; ++i) {
          std::string s = mf.contents[i].lexemeValue->contents;
          if (s == "TRUE") {
            bool_def.push_back(true);
          } else if (s == "FALSE") {
            bool_def.push_back(false);
          } else {
            all_bool = false;
            break;
          }
        }

        if (all_bool) {
          std::vector<bool> result;
          cx_utils::declare_parameter_if_not_declared(
            node, param_name, rclcpp::ParameterValue(bool_def));
          node->get_parameter(param_name, result);

          for (bool b : result) clips::MBAppendSymbol(mb, b ? "TRUE" : "FALSE");
          res.multifieldValue = clips::MBCreate(mb);
          break;
        }

        // Treat as string array
        std::vector<std::string> def, result;
        for (size_t i = 0; i < n; ++i) def.push_back(mf.contents[i].lexemeValue->contents);

        cx_utils::declare_parameter_if_not_declared(node, param_name, rclcpp::ParameterValue(def));
        node->get_parameter(param_name, result);

        for (const auto & s : result) clips::MBAppendString(mb, s.c_str());
        res.multifieldValue = clips::MBCreate(mb);
      }

      if (first.header->type == FLOAT_TYPE) {
        std::vector<double> def, result;
        for (size_t i = 0; i < n; ++i) def.push_back(mf.contents[i].floatValue->contents);
        cx_utils::declare_parameter_if_not_declared(node, param_name, rclcpp::ParameterValue(def));
        node->get_parameter(param_name, result);
        for (const auto & v : result) clips::MBAppendFloat(mb, v);
        res.multifieldValue = clips::MBCreate(mb);
      }
      clips::MBDispose(mb);
    }

    default:
      return res;
  }
  return res;
}

}  // namespace cx

PLUGINLIB_EXPORT_CLASS(cx::RosParamPlugin, cx::ClipsPlugin)
