/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include "gz/fuel_tools/Result.hh"

namespace gz::fuel_tools
{

class ResultPrivate
{
  /// \brief a type of result
  public: ResultType type = ResultType::UNKNOWN;
};

//////////////////////////////////////////////////
Result::Result()
  : dataPtr(std::make_unique<ResultPrivate>())
{
}

//////////////////////////////////////////////////
Result::~Result() = default;

//////////////////////////////////////////////////
ResultType Result::Type() const
{
  return this->dataPtr->type;
}

//////////////////////////////////////////////////
Result::Result(const ResultType _type)
  : dataPtr(std::make_unique<ResultPrivate>())
{
  this->dataPtr->type = _type;
}

//////////////////////////////////////////////////
Result::Result(const Result &_result)
  : dataPtr(std::make_unique<ResultPrivate>(*_result.dataPtr))
{
}

/////////////////////////////////////////////////
Result::Result(Result &&_result) noexcept = default;  // NOLINT

/////////////////////////////////////////////////
Result &Result::operator=(const Result &_result)
{
  *this->dataPtr = (*_result.dataPtr);
  return *this;
}

/////////////////////////////////////////////////
Result &Result::operator=(Result &&_result) noexcept = default;  // NOLINT

//////////////////////////////////////////////////
Result::operator bool() const
{
  switch (this->dataPtr->type)
  {
    case ResultType::DELETE:
    case ResultType::FETCH:
    case ResultType::FETCH_ALREADY_EXISTS:
    case ResultType::PATCH:
    case ResultType::UPLOAD:
      return true;
    default:
      return false;
  }
}

//////////////////////////////////////////////////
std::string Result::ReadableResult() const
{
  switch (this->dataPtr->type)
  {
    case ResultType::DELETE:
      return "Successfully deleted";
    case ResultType::FETCH:
      return "Successfully fetched from server";
    case ResultType::FETCH_ALREADY_EXISTS:
      return "Already in cache, did not fetch from server";
    case ResultType::UPLOAD:
      return "Successfully uploaded to server";
    case ResultType::DELETE_NOT_FOUND:
      return "Could not delete, model not found";
    case ResultType::DELETE_ERROR:
      return "Delete failed. Other errors";
    case ResultType::FETCH_NOT_FOUND:
        return "Model not found";
    case ResultType::FETCH_ERROR:
        return "Fetch failed. Other errors";
    case ResultType::UPLOAD_ALREADY_EXISTS:
        return "Model already exists";
    case ResultType::UPLOAD_ERROR:
        return "Upload failed. Other errors";
    case ResultType::PATCH_ERROR:
        return "Patch failed.";
    case ResultType::PATCH:
      return "Successfully sent patch request to the server";
    case ResultType::UNKNOWN:
    default:
      return "Unknown result";
  }
}
}  // namespace gz::fuel_tools
