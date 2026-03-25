// Copyright 2015-2024 Autoware Foundation. All rights reserved.
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
//
// Authors: Mamoru Sobue

#ifndef AUTOWARE_LANELET2_EXTENSION__VERSION_HPP_
#define AUTOWARE_LANELET2_EXTENSION__VERSION_HPP_

// NOLINTBEGIN(readability-identifier-naming)

namespace lanelet::autoware
{
/*
 * @brief denotes the major format_version
 */
enum class Version : int {
  none = 0,
  format_v1,
  format_v2,
};

// current format_version
static constexpr Version version = Version::format_v2;
}  // namespace lanelet::autoware

#endif  // AUTOWARE_LANELET2_EXTENSION__VERSION_HPP_
