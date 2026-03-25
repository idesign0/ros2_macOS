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

#include <ostream>
#include <sstream>

#include "gz/transport/parameters/result.hh"

namespace gz::transport::parameters
{
// Inline bracket to help doxygen filtering.
inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
//////////////////////////////////////////////////
ParameterResult::ParameterResult(ParameterResultType _resultType)
: resultType{_resultType}
{}

//////////////////////////////////////////////////
ParameterResult::ParameterResult(
  ParameterResultType _resultType, const std::string & _paramName)
: resultType{_resultType}, paramName{_paramName}
{}

//////////////////////////////////////////////////
ParameterResult::ParameterResult(
  ParameterResultType _resultType,
  const std::string & _paramName,
  const std::string & _paramType)
: resultType{_resultType}
, paramName{_paramName}
, paramType{_paramType}
{}

//////////////////////////////////////////////////
ParameterResultType ParameterResult::ResultType() const
{
  return resultType;
}

//////////////////////////////////////////////////
const std::string & ParameterResult::ParamName() const
{
  return paramName;
}

//////////////////////////////////////////////////
const std::string & ParameterResult::ParamType() const
{
  return paramType;
}

//////////////////////////////////////////////////
ParameterResult::operator bool() const
{
  return resultType == ParameterResultType::Success;
}

//////////////////////////////////////////////////
std::ostream & operator<<(
  std::ostream & os, const ParameterResult & ret)
{
  std::ostringstream ss;
  switch (ret.ResultType()) {
    case ParameterResultType::Success:
      ss << "parameter operation succeeded";
      break;
    case ParameterResultType::AlreadyDeclared:
      ss << "parameter already declared";
      break;
    case ParameterResultType::NotDeclared:
      ss << "parameter not declared";
      break;
    case ParameterResultType::InvalidType:
      ss << "parameter type is not valid";
      break;
    case ParameterResultType::ClientTimeout:
      ss << "parameter client timed out";
      break;
    case ParameterResultType::Unexpected:
    default:
      ss << "parameter operation unexpected error";
      break;
  }
  if (!ret.ParamName().empty()) {
    ss << ", parameter name [" << ret.ParamName() << "]";
  }
  if (!ret.ParamType().empty()) {
    ss << ", parameter type [" << ret.ParamType() << "]";
  }
  os << ss.str();
  return os;
}
}  // namespace GZ_TRANSPORT_VERSION_NAMESPACE
}  // namespace gz::transport::parameters
