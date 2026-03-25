// Copyright 2023 TIER IV, Inc.
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

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-pro-type-member-init)

#include "autoware_lanelet2_extension/projection/transverse_mercator_projector.hpp"

#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace lanelet::projection
{
TransverseMercatorProjector::TransverseMercatorProjector(Origin origin, const double scale_factor)
: Projector(origin)
{
  central_meridian_ = origin.position.lon;
  scale_factor_ = scale_factor;
  // Calculate origin in Transverse Mercator coordinate
  const GeographicLib::TransverseMercatorExact & proj = GeographicLib::TransverseMercatorExact(
    GeographicLib::Constants::WGS84_a(), GeographicLib::Constants::WGS84_f(), scale_factor_);
  proj.Forward(central_meridian_, origin.position.lat, origin.position.lon, origin_x_, origin_y_);
}

BasicPoint3d TransverseMercatorProjector::forward(const GPSPoint & gps) const
{
  BasicPoint3d tm_point{0., 0., gps.ele};
  const GeographicLib::TransverseMercatorExact & proj = GeographicLib::TransverseMercatorExact(
    GeographicLib::Constants::WGS84_a(), GeographicLib::Constants::WGS84_f(), scale_factor_);
  proj.Forward(central_meridian_, gps.lat, gps.lon, tm_point.x(), tm_point.y());

  // x is already aligned with origin as it is projected in transverse mercator
  tm_point.y() = tm_point.y() - origin_y_;

  return tm_point;
}

GPSPoint TransverseMercatorProjector::reverse(const BasicPoint3d & local_point) const
{
  GPSPoint gps{0.0, 0.0, local_point.z()};
  const GeographicLib::TransverseMercatorExact & proj = GeographicLib::TransverseMercatorExact(
    GeographicLib::Constants::WGS84_a(), GeographicLib::Constants::WGS84_f(), scale_factor_);

  BasicPoint3d local_point_copy = local_point;
  local_point_copy.y() = local_point.y() + origin_y_;
  proj.Reverse(central_meridian_, local_point_copy.x(), local_point_copy.y(), gps.lat, gps.lon);
  return gps;
}

}  // namespace lanelet::projection

// NOLINTEND(readability-identifier-naming, cppcoreguidelines-pro-type-member-init)
