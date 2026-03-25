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
#ifndef GZ_MSGS_CONVERT_GEOMETRYTYPE_HH_
#define GZ_MSGS_CONVERT_GEOMETRYTYPE_HH_

#include <gz/msgs/config.hh>

// Message Headers
#include "gz/msgs/geometry.pb.h"

// Data Headers
#include <string>

namespace gz::msgs {
// Inline bracket to help doxygen filtering.
inline namespace GZ_MSGS_VERSION_NAMESPACE {

/////////////////////////////////////////////////
inline msgs::Geometry::Type ConvertGeometryType(const std::string &_str)
{
  msgs::Geometry::Type result = msgs::Geometry::BOX;
  if (_str == "box")
  {
    result = msgs::Geometry::BOX;
  }
  else if (_str == "capsule")
  {
    result = msgs::Geometry::CAPSULE;
  }
  else if (_str == "cone")
  {
    result = msgs::Geometry::CONE;
  }
  else if (_str == "cylinder")
  {
    result = msgs::Geometry::CYLINDER;
  }
  else if (_str == "ellipsoid")
  {
    result = msgs::Geometry::ELLIPSOID;
  }
  else if (_str == "sphere")
  {
    result = msgs::Geometry::SPHERE;
  }
  else if (_str == "plane")
  {
    result = msgs::Geometry::PLANE;
  }
  else if (_str == "image")
  {
    result = msgs::Geometry::IMAGE;
  }
  else if (_str == "heightmap")
  {
    result = msgs::Geometry::HEIGHTMAP;
  }
  else if (_str == "mesh")
  {
    result = msgs::Geometry::MESH;
  }
  else if (_str == "polyline")
  {
    result = msgs::Geometry::POLYLINE;
  }
  else
  {
    std::cerr << "Unrecognized Geometry::Type ["
              << _str
              << "], returning msgs::Geometry::BOX"
              << std::endl;
  }

  return result;
}

/////////////////////////////////////////////////
inline std::string ConvertGeometryType(const msgs::Geometry::Type _type)
{
  std::string result;
  switch (_type)
  {
    case msgs::Geometry::BOX:
    {
      result = "box";
      break;
    }
    case msgs::Geometry::CAPSULE:
    {
      result = "capsule";
      break;
    }
  case msgs::Geometry::CONE:
    {
      result = "cone";
      break;
    }
    case msgs::Geometry::CYLINDER:
    {
      result = "cylinder";
      break;
    }
    case msgs::Geometry::ELLIPSOID:
    {
      result = "ellipsoid";
      break;
    }
    case msgs::Geometry::SPHERE:
    {
      result = "sphere";
      break;
    }
    case msgs::Geometry::PLANE:
    {
      result = "plane";
      break;
    }
    case msgs::Geometry::IMAGE:
    {
      result = "image";
      break;
    }
    case msgs::Geometry::HEIGHTMAP:
    {
      result = "heightmap";
      break;
    }
    case msgs::Geometry::MESH:
    {
      result = "mesh";
      break;
    }
    case msgs::Geometry::POLYLINE:
    {
      result = "polyline";
      break;
    }
    default:
    {
      result = "unknown";
      std::cerr << "Unrecognized Geometry::Type ["
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

#endif  // GZ_MSGS_CONVERT_GEOMETRYTYPE_HH_
