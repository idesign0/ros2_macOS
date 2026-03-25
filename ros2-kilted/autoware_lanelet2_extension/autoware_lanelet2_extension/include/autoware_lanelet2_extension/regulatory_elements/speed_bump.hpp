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
// Authors: Ryohsuke Mitsudome, Mehmet Dogru

#ifndef AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__SPEED_BUMP_HPP_
#define AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__SPEED_BUMP_HPP_

// NOLINTBEGIN(readability-identifier-naming)

#include <autoware_lanelet2_extension/regulatory_elements/Forward.hpp>

#include <lanelet2_core/primitives/Lanelet.h>

#include <memory>

namespace lanelet::autoware
{

inline namespace format_v2
{
class SpeedBump : public lanelet::RegulatoryElement
{
public:
  using Ptr = std::shared_ptr<SpeedBump>;
  static constexpr char RuleName[] = "speed_bump";

  static Ptr make(Id id, const AttributeMap & attributes, const Polygon3d & speed_bump)
  {
    return Ptr{new SpeedBump(id, attributes, speed_bump)};
  }

  /**
   * @brief get the relevant speed bumps
   * @return speed bumps
   */
  [[nodiscard]] ConstPolygon3d speedBump() const;
  [[nodiscard]] Polygon3d speedBump();

  /**
   * @brief add a new speed bump
   * @param primitive speed bump to add
   */
  void addSpeedBump(const Polygon3d & primitive);

  /**
   * @brief remove a speed bump
   * @param primitive speed bump to remove
   * @return true if the speed bump existed and was removed
   */
  bool removeSpeedBump(const Polygon3d & primitive);

private:
  SpeedBump(Id id, const AttributeMap & attributes, const Polygon3d & speed_bump);

  // the following lines are required so that lanelet2 can create this object
  // when loading a map with this regulatory element
  friend class RegisterRegulatoryElement<SpeedBump>;
  explicit SpeedBump(const lanelet::RegulatoryElementDataPtr & data);
};
}  // namespace format_v2

}  // namespace lanelet::autoware

// NOLINTEND(readability-identifier-naming)

#endif  // AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__SPEED_BUMP_HPP_
