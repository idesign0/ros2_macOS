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

#ifndef AUTOWARE_LANELET2_EXTENSION__PROJECTION__TRANSVERSE_MERCATOR_PROJECTOR_HPP_
#define AUTOWARE_LANELET2_EXTENSION__PROJECTION__TRANSVERSE_MERCATOR_PROJECTOR_HPP_

// NOLINTBEGIN(readability-identifier-naming, modernize-use-nodiscard)

#include <GeographicLib/TransverseMercatorExact.hpp>

#include <lanelet2_io/Exceptions.h>
#include <lanelet2_io/Projection.h>

#include <string>
#include <utility>

namespace lanelet::projection
{

inline namespace format_v2
{
class TransverseMercatorProjector : public Projector
{
public:
  explicit TransverseMercatorProjector(
    Origin origin = Origin({0.0, 0.0}), const double scale_factor = 0.9996);

  /**
   * [TransverseMercatorProjector::forward projects gps lat/lon to Transverse Mercator coordinate]
   * @param  gps [point with latitude longitude information]
   * @return     [projected point in Transverse Mercator coordinate]
   */
  BasicPoint3d forward(const GPSPoint & gps) const override;

  /**
   * @param  local_point [3d point in Transverse Mercator coordinate]
   * @return            [projected point in WGS84]
   */
  GPSPoint reverse(const BasicPoint3d & local_point) const override;

private:
  /**
   * origin geoid coordinate for reverse function
   */
  double origin_x_;
  double origin_y_;
  double central_meridian_;

  /**
   * scale factor of the map
   */
  double scale_factor_;
};
}  // namespace format_v2

}  // namespace lanelet::projection

// NOLINTEND(readability-identifier-naming, modernize-use-nodiscard)

#endif  // AUTOWARE_LANELET2_EXTENSION__PROJECTION__TRANSVERSE_MERCATOR_PROJECTOR_HPP_
