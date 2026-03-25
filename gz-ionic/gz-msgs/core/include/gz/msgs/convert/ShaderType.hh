/*
 * Copyright (C) 2023 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef GZ_MSGS_CONVERT_SHADERTYPE_HH_
#define GZ_MSGS_CONVERT_SHADERTYPE_HH_

#include <gz/msgs/config.hh>

// Message Headers
#include "gz/msgs/material.pb.h"

// Data Headers
#include <string>

namespace gz::msgs {
// Inline bracket to help doxygen filtering.
inline namespace GZ_MSGS_VERSION_NAMESPACE {

/////////////////////////////////////////////////
inline msgs::Material::ShaderType ConvertShaderType(const std::string &_str)
{
  auto result = msgs::Material::VERTEX;
  if (_str == "vertex")
  {
    result = msgs::Material::VERTEX;
  }
  else if (_str == "pixel")
  {
    result = msgs::Material::PIXEL;
  }
  else if (_str == "normal_map_object_space")
  {
    result = msgs::Material::NORMAL_MAP_OBJECT_SPACE;
  }
  else if (_str == "normal_map_tangent_space")
  {
    result = msgs::Material::NORMAL_MAP_TANGENT_SPACE;
  }
  else
  {
    std::cerr << "Unrecognized Material::ShaderType ["
              << _str
              << "], returning msgs::Material::VERTEX"
              << std::endl;
  }
  return result;
}

/////////////////////////////////////////////////
inline std::string ConvertShaderType(const msgs::Material::ShaderType &_type)
{
  std::string result;
  switch (_type)
  {
    case msgs::Material::VERTEX:
    {
      result = "vertex";
      break;
    }
    case msgs::Material::PIXEL:
    {
      result = "pixel";
      break;
    }
    case msgs::Material::NORMAL_MAP_OBJECT_SPACE:
    {
      result = "normal_map_object_space";
      break;
    }
    case msgs::Material::NORMAL_MAP_TANGENT_SPACE:
    {
      result = "normal_map_tangent_space";
      break;
    }
    default:
    {
      result = "unknown";
      std::cerr << "Unrecognized Material::ShaderType ["
                << _type
                << "], returning 'unknown'"
                << std::endl;
      break;
    }
  }
  return result;
}

}  // namespace
}  // namespace gz::msgs

#endif  // GZ_MSGS_CONVERT_SHADERTYPE_HH_
