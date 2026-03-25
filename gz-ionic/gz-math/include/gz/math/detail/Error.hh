/*
 * Copyright (C) 2024 Open Source Robotics Foundation
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
#ifndef GZ_MATH_DETAIL_ERROR_HH_
#define GZ_MATH_DETAIL_ERROR_HH_

#include <string>

#include <gz/math/Export.hh>
#include <gz/math/config.hh>

namespace gz::math
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_MATH_VERSION_NAMESPACE {
  namespace detail {
    /// Prints the given error message to std::cerr, followed by a newline.
    /// (In the future, we might provide a function to change the destination.)
    void GZ_MATH_VISIBLE LogErrorMessage(const std::string& message);
  }  // namespace detail
  }  // namespace GZ_MATH_VERSION_NAMESPACE
}  // namespace gz::math
#endif  // GZ_MATH_DETAIL_ERROR_HH_
