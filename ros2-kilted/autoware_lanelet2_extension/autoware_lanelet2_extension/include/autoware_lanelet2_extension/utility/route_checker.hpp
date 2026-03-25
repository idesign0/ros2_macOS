// Copyright 2022 TIER IV, Inc.
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

#ifndef AUTOWARE_LANELET2_EXTENSION__UTILITY__ROUTE_CHECKER_HPP_
#define AUTOWARE_LANELET2_EXTENSION__UTILITY__ROUTE_CHECKER_HPP_

// NOLINTBEGIN(readability-identifier-naming)

#include <autoware_planning_msgs/msg/lanelet_route.hpp>

#include <lanelet2_core/Forward.h>

namespace lanelet::utils::route
{
using autoware_planning_msgs::msg::LaneletRoute;

bool isRouteValid(const LaneletRoute & route, const lanelet::LaneletMapPtr lanelet_map_ptr_);
}  // namespace lanelet::utils::route

// NOLINTEND(readability-identifier-naming)

#endif  // AUTOWARE_LANELET2_EXTENSION__UTILITY__ROUTE_CHECKER_HPP_
