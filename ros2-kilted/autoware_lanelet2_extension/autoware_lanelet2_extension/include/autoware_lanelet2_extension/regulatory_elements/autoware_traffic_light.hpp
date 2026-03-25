// Copyright 2015-2019 Autoware Foundation. All rights reserved.
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
// Authors: Ryohsuke Mitsudome

#ifndef AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__AUTOWARE_TRAFFIC_LIGHT_HPP_
#define AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__AUTOWARE_TRAFFIC_LIGHT_HPP_

// NOLINTBEGIN(readability-identifier-naming)

#include <autoware_lanelet2_extension/regulatory_elements/Forward.hpp>

#include <lanelet2_core/Forward.h>
#include <lanelet2_core/primitives/Lanelet.h>

#include <memory>

namespace lanelet::autoware
{

inline namespace format_v2
{
class AutowareTrafficLight : public lanelet::TrafficLight
{
public:
  using Ptr = std::shared_ptr<AutowareTrafficLight>;
  static constexpr char RuleName[] = "traffic_light";

  struct AutowareRoleNameString
  {
    static constexpr const char LightBulbs[] = "light_bulbs";
  };

  //! Directly construct a stop line from its required rule parameters.
  //! Might modify the input data in oder to get correct tags.
  static Ptr make(
    Id id, const AttributeMap & attributes, const LineStringsOrPolygons3d & trafficLights,
    const Optional<LineString3d> & stopLine = {}, const LineStrings3d & lightBulbs = {})
  {
    return Ptr{new AutowareTrafficLight(id, attributes, trafficLights, stopLine, lightBulbs)};
  }

  /**
   * @brief get the relevant traffic light bulbs
   * @return the traffic light bulbs
   *
   * There might be multiple traffic light bulbs but they are required to show
   * the same signal.
   */
  [[nodiscard]] ConstLineStrings3d lightBulbs() const;

  /**
   * @brief add a new traffic light bulb
   * @param primitive the traffic light bulb to add
   *
   * Traffic light bulbs are represented as linestrings with each point
   * expressing position of each light bulb (lamp).
   */
  void addLightBulbs(const LineStringOrPolygon3d & primitive);

  /**
   * @brief remove a traffic light bulb
   * @param primitive the primitive
   * @return true if the traffic light bulb existed and was removed
   */
  bool removeLightBulbs(const LineStringOrPolygon3d & primitive);

private:
  AutowareTrafficLight(
    Id id, const AttributeMap & attributes, const LineStringsOrPolygons3d & trafficLights,
    const Optional<LineString3d> & stopLine, const LineStrings3d & lightBulbs);

  // the following lines are required so that lanelet2 can create this object
  // when loading a map with this regulatory element
  friend class RegisterRegulatoryElement<AutowareTrafficLight>;
  explicit AutowareTrafficLight(const lanelet::RegulatoryElementDataPtr & data);
};
}  // namespace format_v2

}  // namespace lanelet::autoware

// NOLINTEND(readability-identifier-naming)

#endif  // AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__AUTOWARE_TRAFFIC_LIGHT_HPP_
