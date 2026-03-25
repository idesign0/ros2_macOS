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
#ifndef GZ_MSGS_CONVERT_JOINTTYPE_HH_
#define GZ_MSGS_CONVERT_JOINTTYPE_HH_

#include <gz/msgs/config.hh>

// Message Headers
#include "gz/msgs/joint.pb.h"

// Data Headers
#include <string>

namespace gz::msgs {
// Inline bracket to help doxygen filtering.
inline namespace GZ_MSGS_VERSION_NAMESPACE {

/////////////////////////////////////////////
inline msgs::Joint::Type ConvertJointType(const std::string &_str)
{
  msgs::Joint::Type result = msgs::Joint::REVOLUTE;
  if (_str == "revolute")
  {
    result = msgs::Joint::REVOLUTE;
  }
  else if (_str == "revolute2")
  {
    result = msgs::Joint::REVOLUTE2;
  }
  else if (_str == "prismatic")
  {
    result = msgs::Joint::PRISMATIC;
  }
  else if (_str == "universal")
  {
    result = msgs::Joint::UNIVERSAL;
  }
  else if (_str == "ball")
  {
    result = msgs::Joint::BALL;
  }
  else if (_str == "screw")
  {
    result = msgs::Joint::SCREW;
  }
  else if (_str == "gearbox")
  {
    result = msgs::Joint::GEARBOX;
  }
  else if (_str == "fixed")
  {
    result = msgs::Joint::FIXED;
  }
  else if (_str == "continuous")
  {
    result = msgs::Joint::CONTINUOUS;
  }
  else
  {
    std::cerr << "Unrecognized JointType ["
              << _str
              << "], returning msgs::Joint::REVOLUTE"
              << std::endl;
  }
  return result;
}

/////////////////////////////////////////////
inline std::string ConvertJointType(const msgs::Joint::Type &_type)
{
  std::string result;
  switch (_type)
  {
    case msgs::Joint::REVOLUTE:
    {
      result = "revolute";
      break;
    }
    case msgs::Joint::REVOLUTE2:
    {
      result = "revolute2";
      break;
    }
    case msgs::Joint::PRISMATIC:
    {
      result = "prismatic";
      break;
    }
    case msgs::Joint::UNIVERSAL:
    {
      result = "universal";
      break;
    }
    case msgs::Joint::BALL:
    {
      result = "ball";
      break;
    }
    case msgs::Joint::SCREW:
    {
      result = "screw";
      break;
    }
    case msgs::Joint::GEARBOX:
    {
      result = "gearbox";
      break;
    }
    case msgs::Joint::FIXED:
    {
      result = "fixed";
      break;
    }
    case msgs::Joint::CONTINUOUS:
    {
      result = "continuous";
      break;
    }
    default:
    {
      result = "unknown";
      std::cerr << "Unrecognized JointType ["
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

#endif  // GZ_MSGS_CONVERT_JOINTTYPE_HH_
