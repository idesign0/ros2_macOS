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

#include "cx_ament_index_plugin/ament_index_plugin.hpp"

#include <string>

#include "ament_index_cpp/get_package_prefix.hpp"
#include "ament_index_cpp/get_package_share_directory.hpp"
#include "ament_index_cpp/get_packages_with_prefixes.hpp"
#include "ament_index_cpp/get_resource.hpp"
#include "ament_index_cpp/get_resources.hpp"
#include "ament_index_cpp/get_search_paths.hpp"
#include "ament_index_cpp/has_resource.hpp"
#include "cx_utils/clips_env_context.hpp"
#include "cx_utils/format.hpp"

// To export as plugin
#include "pluginlib/class_list_macros.hpp"

using std::placeholders::_1;
namespace cx
{
AmentIndexPlugin::AmentIndexPlugin() {}

AmentIndexPlugin::~AmentIndexPlugin() {}

void AmentIndexPlugin::initialize()
{
  logger_ = std::make_unique<rclcpp::Logger>(rclcpp::get_logger(plugin_name_));
}

bool AmentIndexPlugin::clips_env_init(std::shared_ptr<clips::Environment> & env)
{
  auto context = CLIPSEnvContext::get_context(env.get());
  RCLCPP_DEBUG(*logger_, "Initializing plugin for environment %s", context->env_name_.c_str());
  std::string fun_name;
  fun_name = "ament-index-get-package-prefix";
  function_names_.emplace(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "sb", 1, 1, ";sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto instance = static_cast<AmentIndexPlugin *>(udfc->context);
      clips::UDFValue package_name;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &package_name);
      try {
        std::string prefix =
          ament_index_cpp::get_package_prefix(package_name.lexemeValue->contents);
        out->lexemeValue = clips::CreateString(env, prefix.c_str());
      } catch (const std::exception & e) {
        RCLCPP_ERROR(*instance->logger_, "ament-index-get-package-prefix: %s", e.what());
        out->lexemeValue = clips::CreateBoolean(env, false);
      }
    },
    "ament_index_get_package_prefix", this);

  fun_name = "ament-index-get-package-share-directory";
  function_names_.emplace(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "sb", 1, 1, ";sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto instance = static_cast<AmentIndexPlugin *>(udfc->context);
      clips::UDFValue package_name;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &package_name);
      try {
        std::string share_dir =
          ament_index_cpp::get_package_share_directory(package_name.lexemeValue->contents);
        out->lexemeValue = clips::CreateString(env, share_dir.c_str());
      } catch (const std::exception & e) {
        RCLCPP_ERROR(*instance->logger_, "ament-index-get-package-share-directory: %s", e.what());
        out->lexemeValue = clips::CreateBoolean(env, false);
      }
    },
    "ament_index_get_package_share_directory", this);

  fun_name = "ament-index-get-packages-with-prefixes";
  function_names_.emplace(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "mb", 0, 0, NULL,
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto instance = static_cast<AmentIndexPlugin *>(udfc->context);
      try {
        auto packages_with_prefixes = ament_index_cpp::get_packages_with_prefixes();
        clips::MultifieldBuilder * mb =
          clips::CreateMultifieldBuilder(env, packages_with_prefixes.size() * 2);
        for (const auto & [package, prefix] : packages_with_prefixes) {
          clips::MBAppendString(mb, package.c_str());
          clips::MBAppendString(mb, prefix.c_str());
        }
        out->multifieldValue = clips::MBCreate(mb);
        clips::MBDispose(mb);
      } catch (const std::exception & e) {
        RCLCPP_ERROR(*instance->logger_, "ament-index-get-packages-with-prefixes: %s", e.what());
        out->lexemeValue = clips::CreateBoolean(env, false);
      }
    },
    "ament_index_get_packages_with_prefixes", this);

  fun_name = "ament-index-get-resource";
  function_names_.emplace(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "bm", 2, 2, ";sy;sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto instance = static_cast<AmentIndexPlugin *>(udfc->context);
      clips::UDFValue resource_type, resource_name;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &resource_type);
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &resource_name);
      std::string content;
      std::string path;
      try {
        if (ament_index_cpp::get_resource(
              resource_type.lexemeValue->contents, resource_name.lexemeValue->contents, content,
              &path)) {
          out->multifieldValue =
            clips::StringToMultifield(env, cx::format("\"{}\" \"{}\"", content, path).c_str());
          return;
        }
      } catch (std::exception & e) {
        RCLCPP_ERROR(*instance->logger_, "ament-index-get-resource: %s", e.what());
        out->lexemeValue = clips::CreateBoolean(env, false);
      }
      out->lexemeValue = clips::CreateBoolean(env, false);
    },
    "ament_index_get_resource", this);

  fun_name = "ament-index-get-resources";
  function_names_.emplace(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "mb", 1, 1, "sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto instance = static_cast<AmentIndexPlugin *>(udfc->context);
      clips::UDFValue resource_type;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &resource_type);
      try {
        auto resources = ament_index_cpp::get_resources(resource_type.lexemeValue->contents);
        clips::MultifieldBuilder * mb = clips::CreateMultifieldBuilder(env, resources.size() * 2);
        for (const auto & [package, prefix] : resources) {
          clips::MBAppendString(mb, package.c_str());
          clips::MBAppendString(mb, prefix.c_str());
        }
        out->multifieldValue = clips::MBCreate(mb);
        clips::MBDispose(mb);
      } catch (std::exception & e) {
        RCLCPP_ERROR(*instance->logger_, "ament-index-get-resources: %s", e.what());
        out->lexemeValue = clips::CreateBoolean(env, false);
      }
    },
    "ament_index_get_resources", this);

  fun_name = "ament-index-get-search-paths";
  function_names_.emplace(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "m", 0, 0, NULL,
    [](clips::Environment * env, clips::UDFContext * /*udfc*/, clips::UDFValue * out) {
      auto paths = ament_index_cpp::get_search_paths();
      clips::MultifieldBuilder * mb = clips::CreateMultifieldBuilder(env, paths.size());
      for (const auto & path : paths) {
        clips::MBAppendString(mb, path.c_str());
      }
      out->multifieldValue = clips::MBCreate(mb);
      clips::MBDispose(mb);
    },
    "ament_index_get_search_paths", NULL);

  fun_name = "ament-index-has-resource";
  function_names_.emplace(fun_name);
  clips::AddUDF(
    env.get(), fun_name.c_str(), "b", 2, 2, ";sy;sy",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto instance = static_cast<AmentIndexPlugin *>(udfc->context);
      clips::UDFValue resource_type, resource_name;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &resource_type);
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &resource_name);
      try {
        bool exists = ament_index_cpp::has_resource(
          resource_type.lexemeValue->contents, resource_name.lexemeValue->contents);
        out->lexemeValue = clips::CreateBoolean(env, exists);
      } catch (std::exception & e) {
        RCLCPP_ERROR(*instance->logger_, "ament-index-has-resource: %s", e.what());
        out->lexemeValue = clips::CreateBoolean(env, false);
      }
    },
    "ament_index_has_resource", this);

  return true;
}

bool AmentIndexPlugin::clips_env_destroyed(std::shared_ptr<clips::Environment> & env)
{
  auto context = CLIPSEnvContext::get_context(env.get());
  RCLCPP_INFO(*logger_, "Destroying plugin for environment %s", context->env_name_.c_str());
  for (const auto & fun : function_names_) {
    clips::RemoveUDF(env.get(), fun.c_str());
  }
  return true;
}
}  // namespace cx

PLUGINLIB_EXPORT_CLASS(cx::AmentIndexPlugin, cx::ClipsPlugin)
