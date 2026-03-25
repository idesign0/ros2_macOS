// Copyright 2023 Tier IV, Inc.
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

#ifndef AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__NO_PARKING_AREA_HPP_
#define AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__NO_PARKING_AREA_HPP_

// NOLINTBEGIN(readability-identifier-naming)

#include <autoware_lanelet2_extension/regulatory_elements/Forward.hpp>

#include <lanelet2_core/primitives/Lanelet.h>

#include <memory>

namespace lanelet::autoware
{

inline namespace format_v2
{
class NoParkingArea : public lanelet::RegulatoryElement
{
public:
  using Ptr = std::shared_ptr<NoParkingArea>;
  static constexpr char RuleName[] = "no_parking_area";

  static Ptr make(Id id, const AttributeMap & attributes, const Polygons3d & no_parking_areas)
  {
    return Ptr{new NoParkingArea(id, attributes, no_parking_areas)};
  }

  /**
   * @brief get the relevant no parking area
   * @return no parking area
   */
  [[nodiscard]] ConstPolygons3d noParkingAreas() const;
  [[nodiscard]] Polygons3d noParkingAreas();

  /**
   * @brief add a new no parking area
   * @param primitive no parking area to add
   */
  void addNoParkingArea(const Polygon3d & primitive);

  /**
   * @brief remove a no parking area
   * @param primitive the primitive
   * @return true if the no parking area existed and was removed
   */
  bool removeNoParkingArea(const Polygon3d & primitive);

private:
  NoParkingArea(Id id, const AttributeMap & attributes, const Polygons3d & no_parking_area);

  // the following lines are required so that lanelet2 can create this object
  // when loading a map with this regulatory element
  friend class RegisterRegulatoryElement<NoParkingArea>;
  explicit NoParkingArea(const lanelet::RegulatoryElementDataPtr & data);
};
}  // namespace format_v2

}  // namespace lanelet::autoware

// NOLINTEND(readability-identifier-naming)

#endif  // AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__NO_PARKING_AREA_HPP_
