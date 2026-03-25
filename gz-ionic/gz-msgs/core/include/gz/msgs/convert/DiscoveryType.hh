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
#ifndef GZ_MSGS_CONVERT_DISCOVERYTYPE_HH_
#define GZ_MSGS_CONVERT_DISCOVERYTYPE_HH_

#include <gz/msgs/config.hh>

// Message Headers
#include "gz/msgs/discovery.pb.h"

// Data Headers
#include <string>

namespace gz::msgs {
// Inline bracket to help doxygen filtering.
inline namespace GZ_MSGS_VERSION_NAMESPACE {

/////////////////////////////////////////////
inline msgs::Discovery::Type ConvertDiscoveryType(const std::string &_str)
{
  msgs::Discovery::Type result = msgs::Discovery::UNINITIALIZED;

  if (_str == "UNINITIALIZED")
  {
    result = msgs::Discovery::UNINITIALIZED;
  }
  else if (_str == "ADVERTISE")
  {
    result = msgs::Discovery::ADVERTISE;
  }
  else if (_str == "SUBSCRIBE")
  {
    result = msgs::Discovery::SUBSCRIBE;
  }
  else if (_str == "UNADVERTISE")
  {
    result = msgs::Discovery::UNADVERTISE;
  }
  else if (_str == "HEARTBEAT")
  {
    result = msgs::Discovery::HEARTBEAT;
  }
  else if (_str == "BYE")
  {
    result = msgs::Discovery::BYE;
  }
  else if (_str == "NEW_CONNECTION")
  {
    result = msgs::Discovery::NEW_CONNECTION;
  }
  else if (_str == "END_CONNECTION")
  {
    result = msgs::Discovery::END_CONNECTION;
  }
  else if (_str == "SUBSCRIBERS_REQ")
  {
    result = msgs::Discovery::SUBSCRIBERS_REQ;
  }
  else if (_str == "SUBSCRIBERS_REP")
  {
    result = msgs::Discovery::SUBSCRIBERS_REP;
  }
  else
  {
    std::cerr << "Unrecognized DiscoveryType["
              << _str
              << "], returning msgs::Discovery::UNINITIALIZED"
              << std::endl;
  }
  return result;
}

/////////////////////////////////////////////
inline std::string ConvertDiscoveryType(const msgs::Discovery::Type &_type)
{
  switch (_type)
  {
    default:
    case msgs::Discovery::UNINITIALIZED:
      return "UNINITIALIZED";
    case msgs::Discovery::ADVERTISE:
      return "ADVERTISE";
    case msgs::Discovery::SUBSCRIBE:
      return "SUBSCRIBE";
    case msgs::Discovery::UNADVERTISE:
      return "UNADVERTISE";
    case msgs::Discovery::HEARTBEAT:
      return "HEARTBEAT";
    case msgs::Discovery::BYE:
      return "BYE";
    case msgs::Discovery::NEW_CONNECTION:
      return "NEW_CONNECTION";
    case msgs::Discovery::END_CONNECTION:
      return "END_CONNECTION";
    case msgs::Discovery::SUBSCRIBERS_REQ:
      return "SUBSCRIBERS_REQ";
    case msgs::Discovery::SUBSCRIBERS_REP:
      return "SUBSCRIBERS_REP";
  };
}

// This is for API compatibility
inline std::string ToString(const msgs::Discovery::Type &_t)
{
  return ConvertDiscoveryType(_t);
}
}  // namespace
}  // namespace gz::msgs

#endif  // GZ_MSGS_CONVERT_DISCOVERYTYPE_HH_
