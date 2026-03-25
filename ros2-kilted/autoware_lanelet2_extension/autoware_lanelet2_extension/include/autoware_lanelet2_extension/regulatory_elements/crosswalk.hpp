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

#ifndef AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__CROSSWALK_HPP_
#define AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__CROSSWALK_HPP_

// NOLINTBEGIN(readability-identifier-naming)

#include <autoware_lanelet2_extension/regulatory_elements/Forward.hpp>

#include <lanelet2_core/primitives/Lanelet.h>

#include <memory>

namespace lanelet::autoware
{

inline namespace format_v2
{
class Crosswalk : public lanelet::RegulatoryElement
{
public:
  using Ptr = std::shared_ptr<Crosswalk>;
  static constexpr char RuleName[] = "crosswalk";

  struct AutowareRoleNameString
  {
    static constexpr const char CrosswalkPolygon[] = "crosswalk_polygon";
  };

  static Ptr make(
    Id id, const AttributeMap & attributes, const Lanelet & crosswalk_lanelet,
    const Polygon3d & crosswalk_area, const LineStrings3d & stop_line)
  {
    return Ptr{new Crosswalk(id, attributes, crosswalk_lanelet, crosswalk_area, stop_line)};
  }

  /**
   * @brief get the relevant crosswalk area
   * @return crosswalk area
   */
  [[nodiscard]] ConstPolygons3d crosswalkAreas() const;

  /**
   * @brief get the relevant crosswalk line
   * @return stop line
   */
  [[nodiscard]] ConstLineStrings3d stopLines() const;

  /**
   * @brief get the relevant crosswalk lanelet
   * @return lanelet
   */
  [[nodiscard]] ConstLanelet crosswalkLanelet() const;

  /**
   * @brief add a new crosswalk area
   * @param primitive crosswalk area to add
   */
  void addCrosswalkArea(const Polygon3d & primitive);

  /**
   * @brief remove a crosswalk area
   * @param primitive the primitive
   * @return true if the crosswalk area existed and was removed
   */
  bool removeCrosswalkArea(const Polygon3d & primitive);

private:
  Crosswalk(
    Id id, const AttributeMap & attributes, const Lanelet & crosswalk_lanelet,
    const Polygon3d & crosswalk_area, const LineStrings3d & stop_line);

  // the following lines are required so that lanelet2 can create this object
  // when loading a map with this regulatory element
  friend class RegisterRegulatoryElement<Crosswalk>;
  explicit Crosswalk(const lanelet::RegulatoryElementDataPtr & data);
};
}  // namespace format_v2

}  // namespace lanelet::autoware

// NOLINTEND(readability-identifier-naming)

#endif  // AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__CROSSWALK_HPP_
