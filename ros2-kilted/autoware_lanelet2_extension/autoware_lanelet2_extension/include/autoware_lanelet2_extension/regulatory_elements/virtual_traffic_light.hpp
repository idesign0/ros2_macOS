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

#ifndef AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__VIRTUAL_TRAFFIC_LIGHT_HPP_
#define AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__VIRTUAL_TRAFFIC_LIGHT_HPP_

// NOLINTBEGIN(readability-identifier-naming)

#include <autoware_lanelet2_extension/regulatory_elements/Forward.hpp>

#include <lanelet2_core/primitives/LineString.h>

#include <memory>

namespace lanelet::autoware
{

inline namespace format_v2
{
class VirtualTrafficLight : public lanelet::RegulatoryElement
{
public:
  using Ptr = std::shared_ptr<VirtualTrafficLight>;
  using ConstPtr = std::shared_ptr<const VirtualTrafficLight>;
  static constexpr char RuleName[] = "virtual_traffic_light";

  static Ptr make(
    const Id id, const AttributeMap & attributes, const LineString3d & virtual_traffic_light)
  {
    return Ptr{new VirtualTrafficLight(id, attributes, virtual_traffic_light)};
  }

  [[nodiscard]] ConstLineString3d getVirtualTrafficLight() const
  {
    return getParameters<ConstLineString3d>(RoleName::Refers).front();
  }

  [[nodiscard]] Optional<ConstLineString3d> getStopLine() const
  {
    const auto vec = getParameters<ConstLineString3d>(RoleName::RefLine);
    if (vec.empty()) {
      return {};
    }
    return vec.front();
  }

  [[nodiscard]] ConstLineString3d getStartLine() const
  {
    return getParameters<ConstLineString3d>("start_line").front();
  }

  [[nodiscard]] ConstLineStrings3d getEndLines() const
  {
    return getParameters<ConstLineString3d>("end_line");
  }

private:
  VirtualTrafficLight(
    const Id id, const AttributeMap & attributes, const LineString3d & virtual_traffic_light);

  // the following lines are required so that lanelet2 can create this object
  // when loading a map with this regulatory element
  friend class RegisterRegulatoryElement<VirtualTrafficLight>;
  explicit VirtualTrafficLight(const lanelet::RegulatoryElementDataPtr & data);
};
}  // namespace format_v2

}  // namespace lanelet::autoware

// NOLINTEND(readability-identifier-naming)

#endif  // AUTOWARE_LANELET2_EXTENSION__REGULATORY_ELEMENTS__VIRTUAL_TRAFFIC_LIGHT_HPP_
