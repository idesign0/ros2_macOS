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

#include "cx_config_plugin/config_plugin.hpp"

#include <memory>
#include <string>

#include "ament_index_cpp/get_package_prefix.hpp"
#include "ament_index_cpp/get_package_share_directory.hpp"
#include "cx_utils/format.hpp"
#include "yaml-cpp/yaml.h"

namespace cx
{
ConfigPlugin::ConfigPlugin() {}

ConfigPlugin::~ConfigPlugin() {}

void ConfigPlugin::initialize()
{
  logger_ = std::make_unique<rclcpp::Logger>(rclcpp::get_logger(plugin_name_));
}

bool ConfigPlugin::clips_env_init(std::shared_ptr<clips::Environment> & env)
{
  RCLCPP_DEBUG(*logger_, "Initialising context");

  std::string clips_path = ament_index_cpp::get_package_share_directory("cx_config_plugin") +
                           "/clips/cx_config_plugin/ff-config.clp";

  if (!clips::BatchStar(env.get(), clips_path.c_str())) {
    RCLCPP_ERROR(
      *logger_,
      "Failed to initialize CLIPS environment, "
      "batch file '%s' failed!, aborting...",
      clips_path.c_str());
    return false;
  }
  clips::AddUDF(
    env.get(), "config-load", "v", 2, 2, ";sy;sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * /*out*/) {
      ConfigPlugin * instance = static_cast<ConfigPlugin *>(udfc->context);
      using namespace clips;  // NOLINT
      clips::UDFValue file;
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &file);
      clips::UDFValue cfg_prefix;
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &cfg_prefix);
      instance->clips_config_load(
        env, file.lexemeValue->contents, cfg_prefix.lexemeValue->contents);
    },
    "clips_config_load", this);
  return true;
}

bool ConfigPlugin::clips_env_destroyed(std::shared_ptr<clips::Environment> & env)
{
  RCLCPP_DEBUG(*logger_, "Destroying clips context");

  clips::RemoveUDF(env.get(), "config-load");
  clips::Deftemplate * curr_tmpl = clips::FindDeftemplate(env.get(), "conval");
  if (curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  }
  return true;
}

void ConfigPlugin::clips_config_load(
  clips::Environment * env, const std::string & file, const std::string & cfg_prefix)
{
  try {
    std::string sanitized_cfg_prefix = cfg_prefix;
    if (sanitized_cfg_prefix.size() > 0) {
      if (!sanitized_cfg_prefix.starts_with('/')) {
        sanitized_cfg_prefix = '/' + sanitized_cfg_prefix;
      }
      if (sanitized_cfg_prefix.ends_with('/')) {
        sanitized_cfg_prefix.pop_back();
      }
    }
    YAML::Node config = YAML::LoadFile(file);
    YAML::Node config_main = config;
    std::istringstream path_stream(sanitized_cfg_prefix);
    std::string segment;
    while (std::getline(path_stream, segment, '/')) {
      if (!segment.empty()) {
        config_main = config_main[segment];
        if (!config_main) {
          RCLCPP_ERROR(*logger_, "Segment '%s' not found in YAML file.", segment.c_str());
          break;
        }
      }
    }
    iterateThroughYamlRecuresively(config_main, sanitized_cfg_prefix, env);
  } catch (const std::exception & e) {
    RCLCPP_ERROR_STREAM(*logger_, e.what());
    RCLCPP_WARN(*logger_, "Aborting config loading...");
  }
}

void ConfigPlugin::iterateThroughYamlRecuresively(
  const YAML::Node & current_level_node, std::string cfg_prefix, clips::Environment * env)
{
  std::string config_path = cfg_prefix;
  // RCLCPP_INFO(*logger_, "CFG PATH: %s",
  //             config_path.c_str());

  for (const auto & item : current_level_node) {
    std::string type = "";
    std::string path = "";

    switch (item.second.Type()) {
      // If it is a NullNode
      case YAML::NodeType::Undefined: {
        RCLCPP_ERROR(*logger_, "Undefined YAML KEY");
        break;
      }
      case YAML::NodeType::Null: {
        RCLCPP_ERROR(*logger_, "NULL YAML KEY");
        break;
      }

      // If it is a ScalarNode -> Single key/value pair
      case YAML::NodeType::Scalar: {
        type = std::move(getScalarType(item.second));
        path = config_path + "/" + item.first.as<std::string>();

        if (type == "STRING") {
          std::stringstream escaped_quotes;
          escaped_quotes << std::quoted(item.second.as<std::string>());
          // RCLCPP_INFO(*logger_,
          //             "(confval (path \"%s\") (type %s) (value %s))",
          //             path.c_str(), type.c_str(),
          //             escaped_quotes.str().c_str());

          clips::AssertString(
            env, cx::format(
                   "(confval (path \"{}\") (type {}) (value {}))", path.c_str(), type.c_str(),
                   escaped_quotes.str().c_str())
                   .c_str());
        } else {
          // RCLCPP_INFO(*logger_,
          //             "(confval (path \"%s\") (type %s) (value %s))",
          //             path.c_str(), type.c_str(),
          //             item.second.as<std::string>().c_str());
          if (item.second.as<std::string>() == "true" || item.second.as<std::string>() == "false") {
            std::string val = item.second.as<std::string>();
            std::transform(val.begin(), val.end(), val.begin(), ::toupper);
            clips::AssertString(
              env, cx::format(
                     "(confval (path \"{}\") (type {}) (value {}))", path.c_str(), type.c_str(),
                     val.c_str())
                     .c_str());
          } else {
            clips::AssertString(
              env, cx::format(
                     "(confval (path \"{}\") (type {}) (value {}))", path.c_str(), type.c_str(),
                     item.second.as<std::string>().c_str())
                     .c_str());
          }
        }
        break;
      }

      // If it is a SequenceNode
      case YAML::NodeType::Sequence: {
        const YAML::Node & nested_node = item.second;
        path = config_path + "/" + item.first.as<std::string>();

        sequenceIterator(nested_node, path, env);

        break;
      }
      // If it is a MapNode
      case YAML::NodeType::Map: {
        path = config_path + "/" + item.first.as<std::string>();
        iterateThroughYamlRecuresively(current_level_node[item.first], path, env);
        break;
      }
    }
  }
}

void ConfigPlugin::sequenceIterator(
  const YAML::Node & input_node, std::string & cfg_prefix, clips::Environment * env)
{
  size_t i = 0;
  std::string path;
  std::string list_values{};
  int sequenceIndex = 0;

  for (const auto & item2 : input_node) {
    if (item2) {
      if (item2.IsScalar()) {
        // Direct sequence of the form ["1", "2", ...]
        list_values = list_values + " " + "\"" + item2.as<std::string>() + "\"";
        i++;

        if (i == input_node.size()) {
          clips::AssertString(
            env, cx::format(
                   "(confval (path \"{}\") (type STRING) "
                   "(is-list TRUE) (list-value {}))",
                   cfg_prefix.c_str(), list_values.c_str())
                   .c_str());
          // RCLCPP_INFO(*logger_,
          //             "(confval (path \"%s\") (type STRING) "
          //             "(is-list TRUE) (list-value%s))",
          //             cfg_prefix.c_str(), list_values.c_str());
        }

        continue;
      }
      // item2 is a Map or Sequence

      for (const auto & item3 : item2) {
        // OF the FORM
        //   - name: domain,
        //     file: test-scenario/test-domain.clp
        if (item3.second.IsScalar()) {
          path = cfg_prefix + "/" + std::move(std::to_string(sequenceIndex)) + "/" +
                 item3.first.as<std::string>();

          // path = path + std::move(std::to_string(sequenceIndex));

          clips::AssertString(
            env, cx::format(
                   "(confval (path \"{}\") (is-list FALSE) (type {}) (value "
                   "\"{}\"))",
                   path.c_str(), std::move(getScalarType(item3.second)).c_str(),
                   item3.second.as<std::string>().c_str())
                   .c_str());

          // RCLCPP_INFO(*logger_,
          //             "(confval (path \"%s\") (is-list FALSE) (type %s)
          //             (value "
          //             "\"%s\"))",
          //             path.c_str(),
          //             std::move(getScalarType(item3.second)).c_str(),
          //             item3.second.as<std::string>().c_str());

        } else if (item3.second.IsSequence()) {
          // OF the FORM
          //  action-execution:
          //    - skills-actions.clp
          //    - test-scenario-pddl/print-action.clp

          path = cfg_prefix + "/" + std::move(std::to_string(sequenceIndex)) + "/" +
                 item3.first.as<std::string>();
          // path = path + std::move(std::to_string(sequenceIndex));

          list_values = "";

          std::stringstream escaped_quotes;
          for (const auto & el : item3.second) {
            // if (list_values != "") {
            //   list_values = " " + el.as<std::string>();

            // } else {
            //   list_values = el.as<std::string>();
            // }
            escaped_quotes << std::quoted(el.as<std::string>());
          }

          clips::AssertString(
            env, cx::format(
                   "(confval (path \"{}\") (type STRING) "
                   "(is-list TRUE) (list-value {}))",
                   path.c_str(), escaped_quotes.str().c_str())
                   .c_str());
          // RCLCPP_INFO(*logger_,
          //             "(confval (path \"%s\") (type STRING) "
          //             "(is-list TRUE) (list-value%s))",
          //             path.c_str(), escaped_quotes.str().c_str());

        } else {
          sequenceIterator(item3.second, cfg_prefix, env);
        }
      }
      sequenceIndex++;
    } else {
      RCLCPP_ERROR(
        *logger_,
        "Something went wrong while trying to iterate through the "
        "yaml file!");
    }
  }
}

std::string ConfigPlugin::getScalarType(const YAML::Node & input_node)
{
  std::optional<bool> as_bool = YAML::as_if<bool, std::optional<bool>>(input_node)();
  if (as_bool) {
    return "BOOL";
  }

  std::optional<int> as_int = YAML::as_if<int, std::optional<int>>(input_node)();
  if (as_int) {
    return "INT";
  }

  std::optional<double> as_double = YAML::as_if<double, std::optional<double>>(input_node)();
  if (as_double) {
    return "FLOAT";
  }

  std::optional<std::string> as_string =
    YAML::as_if<std::string, std::optional<std::string>>(input_node)();
  if (as_string) {
    return "STRING";
  }
  return "UNKNOWN";
}

}  // namespace cx

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(cx::ConfigPlugin, cx::ClipsPlugin)
