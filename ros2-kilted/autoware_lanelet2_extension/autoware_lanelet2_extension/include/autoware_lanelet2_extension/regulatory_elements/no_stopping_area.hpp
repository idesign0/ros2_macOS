// Copyright 2021 Tier IV, Inc.
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

#ifndef AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__NO_STOPPING_AREA_HPP_
#define AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__NO_STOPPING_AREA_HPP_

// NOLINTBEGIN(readability-identifier-naming)

#include <autoware_lanelet2_extension/regulatory_elements/Forward.hpp>

#include <lanelet2_core/primitives/Lanelet.h>

#include <memory>

namespace lanelet::autoware
{

inline namespace format_v2
{
class NoStoppingArea : public lanelet::RegulatoryElement
{
public:
  using Ptr = std::shared_ptr<NoStoppingArea>;
  static constexpr char RuleName[] = "no_stopping_area";

  //! Directly construct a stop line from its required rule parameters.
  //! Might modify the input data in oder to get correct tags.
  static Ptr make(
    Id id, const AttributeMap & attributes, const Polygons3d & no_stopping_areas,
    const Optional<LineString3d> & stopLine = {})
  {
    return Ptr{new NoStoppingArea(id, attributes, no_stopping_areas, stopLine)};
  }

  /**
   * @brief get the relevant no stopping area
   * @return no stopping area
   */
  [[nodiscard]] ConstPolygons3d noStoppingAreas() const;
  [[nodiscard]] Polygons3d noStoppingAreas();

  /**
   * @brief add a new no stopping area
   * @param primitive no stopping area to add
   */
  void addNoStoppingArea(const Polygon3d & primitive);

  /**
   * @brief remove a no stopping area
   * @param primitive the primitive
   * @return true if the no stopping area existed and was removed
   */
  bool removeNoStoppingArea(const Polygon3d & primitive);

  /**
   * @brief get the stop line for the no stopping area
   * @return the stop line as LineString
   */
  [[nodiscard]] Optional<ConstLineString3d> stopLine() const;
  [[nodiscard]] Optional<LineString3d> stopLine();

  /**
   * @brief set a new stop line, overwrite the old one
   * @param stopLine new stop line
   */
  void setStopLine(const LineString3d & stopLine);

  //! Deletes the stop line
  void removeStopLine();

private:
  NoStoppingArea(
    Id id, const AttributeMap & attributes, const Polygons3d & no_stopping_area,
    const Optional<LineString3d> & stopLine);

  // the following lines are required so that lanelet2 can create this object
  // when loading a map with this regulatory element
  friend class RegisterRegulatoryElement<NoStoppingArea>;
  explicit NoStoppingArea(const lanelet::RegulatoryElementDataPtr & data);
};
}  // namespace format_v2

}  // namespace lanelet::autoware

// NOLINTEND(readability-identifier-naming)

#endif  // AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__NO_STOPPING_AREA_HPP_
