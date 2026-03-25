/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#include "Utils.hh"

#include <ios>
#include <sstream>
#include <string>

namespace gz::transport::parameters
{
// Inline bracket to help doxygen filtering.
inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
//////////////////////////////////////////////////
std::string addGzMsgsPrefix(const std::string &_gzType)
{
  std::ostringstream oss{"gz_msgs.", std::ios_base::ate};
  oss << _gzType;
  return oss.str();
}

//////////////////////////////////////////////////
std::optional<std::string> getGzTypeFromAnyProto(
  const google::protobuf::Any &_any)
{
  auto typeUrl = _any.type_url();
  auto pos = typeUrl.rfind('/');
  if (pos == std::string::npos) {
    return std::nullopt;
  }
  const char prefix[] = "gz.msgs.";
  auto ret = typeUrl.substr(pos + 1);
  if (!ret.compare(0, sizeof(prefix), prefix)) {
    return std::nullopt;
  }
  return ret.substr(sizeof(prefix) - 1);
}
}  // namespace GZ_TRANSPORT_VERSION_NAMESPACE
}  // namespace gz::transport::parameters
