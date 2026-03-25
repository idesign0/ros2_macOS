// Copyright 2024 Autoware Foundation. All rights reserved.
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

#ifndef AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__BUS_STOP_AREA_HPP_
#define AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__BUS_STOP_AREA_HPP_

// NOLINTBEGIN(readability-identifier-naming)

#include <autoware_lanelet2_extension/regulatory_elements/Forward.hpp>

#include <lanelet2_core/primitives/Lanelet.h>

#include <memory>

namespace lanelet::autoware
{

inline namespace format_v2
{
class BusStopArea : public lanelet::RegulatoryElement
{
public:
  using SharedPtr = std::shared_ptr<BusStopArea>;
  static constexpr char RuleName[] = "bus_stop_area";

  static SharedPtr make(Id id, const AttributeMap & attributes, const Polygons3d & bus_stop_areas)
  {
    return SharedPtr{new BusStopArea(id, attributes, bus_stop_areas)};
  }

  /**
   * @brief get the relevant bus stop area
   * @return bus stop area
   */
  [[nodiscard]] ConstPolygons3d busStopAreas() const;
  [[nodiscard]] Polygons3d busStopAreas();

  /**
   * @brief add a new bus stop are
   * @param primitive bus stop area to add
   */
  void addBusStopArea(const Polygon3d & primitive);

  /**
   * @brief remove a bus stop area
   * @param primitive the primitive
   * @return true if the bus stop area existed and was removed
   */
  bool removeBusStopArea(const Polygon3d & primitive);

private:
  BusStopArea(Id id, const AttributeMap & attributes, const Polygons3d & bus_stop_area);

  // the following lines are required so that lanelet2 can create this object
  // when loading a map with this regulatory element
  friend class RegisterRegulatoryElement<BusStopArea>;
  explicit BusStopArea(const lanelet::RegulatoryElementDataPtr & data);
};
}  // namespace format_v2

}  // namespace lanelet::autoware

// NOLINTEND(readability-identifier-naming)

#endif  // AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__BUS_STOP_AREA_HPP_
