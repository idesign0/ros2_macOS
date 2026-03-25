// Copyright 2020 Tier IV, Inc.
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

#ifndef AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__ROAD_MARKING_HPP_
#define AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__ROAD_MARKING_HPP_

// NOLINTBEGIN(readability-identifier-naming)

#include <autoware_lanelet2_extension/regulatory_elements/Forward.hpp>

#include <lanelet2_core/primitives/LineString.h>

#include <memory>

namespace lanelet::autoware
{

inline namespace format_v2
{
class RoadMarking : public lanelet::RegulatoryElement
{
public:
  using Ptr = std::shared_ptr<RoadMarking>;
  static constexpr char RuleName[] = "road_marking";

  //! Directly construct a stop line from its required rule parameters.
  //! Might modify the input data in oder to get correct tags.
  static Ptr make(Id id, const AttributeMap & attributes, const LineString3d & road_marking)
  {
    return Ptr{new RoadMarking(id, attributes, road_marking)};
  }

  /**
   * @brief get the relevant road marking
   * @return road marking
   */
  [[nodiscard]] ConstLineString3d roadMarking() const;
  [[nodiscard]] LineString3d roadMarking();

  /**
   * @brief add a new road marking
   * @param primitive road marking to add
   */
  void setRoadMarking(const LineString3d & road_marking);

  /**
   * @brief remove a road marking
   */
  void removeRoadMarking();

private:
  RoadMarking(Id id, const AttributeMap & attributes, const LineString3d & road_marking);

  // the following lines are required so that lanelet2 can create this object
  // when loading a map with this regulatory element
  friend class RegisterRegulatoryElement<RoadMarking>;
  explicit RoadMarking(const lanelet::RegulatoryElementDataPtr & data);
};
}  // namespace format_v2

}  // namespace lanelet::autoware

// NOLINTEND(readability-identifier-naming)

#endif  // AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__ROAD_MARKING_HPP_
