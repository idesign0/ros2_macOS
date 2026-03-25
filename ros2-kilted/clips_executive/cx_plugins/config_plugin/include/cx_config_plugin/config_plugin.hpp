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

#ifndef CX_CONFIG_PLUGIN__CONFIG_PLUGIN_HPP_
#define CX_CONFIG_PLUGIN__CONFIG_PLUGIN_HPP_

#include <yaml-cpp/yaml.h>

#include <map>
#include <memory>
#include <string>
#include <utility>

#include "cx_plugin/clips_plugin.hpp"

// HACKY WAY TO DETERMINE SCALAR TYPE AS YAML-CPP DOESN'T OFFER THAT..
namespace YAML
{
template <typename T>
struct as_if<T, std::optional<T>>
{
  explicit as_if(const Node & input_node) : node(input_node) {}
  const Node & node;

  const std::optional<T> operator()() const
  {
    std::optional<T> val;
    T t;
    if (node.m_pNode && convert<T>::decode(node, t)) val = std::move(t);

    return val;
  }
};

// There is already a std::string partial specialisation, so we need a full
// specialisation here
template <>
struct as_if<std::string, std::optional<std::string>>
{
  explicit as_if(const Node & input_node) : node(input_node) {}
  const Node & node;

  const std::optional<std::string> operator()() const
  {
    std::optional<std::string> val;
    std::string t;
    if (node.m_pNode && convert<std::string>::decode(node, t)) val = std::move(t);

    return val;
  }
};
}  // namespace YAML

namespace cx
{

class ConfigPlugin : public ClipsPlugin
{
public:
  ConfigPlugin();
  ~ConfigPlugin();

  void initialize() override;

  bool clips_env_init(std::shared_ptr<clips::Environment> & env) override;
  bool clips_env_destroyed(std::shared_ptr<clips::Environment> & env) override;

private:
  void clips_config_load(
    clips::Environment * env, const std::string & file, const std::string & cfg_prefix);

  std::string getScalarType(const YAML::Node & input_node);

  void iterateThroughYamlRecuresively(
    const YAML::Node & current_level_node, std::string cfg_prefix, clips::Environment * env);

  void sequenceIterator(
    const YAML::Node & input_node, std::string & cfg_prefix, clips::Environment * env);

  std::unique_ptr<rclcpp::Logger> logger_;
};

}  // namespace cx

#endif  // CX_CONFIG_PLUGIN__CONFIG_PLUGIN_HPP_
