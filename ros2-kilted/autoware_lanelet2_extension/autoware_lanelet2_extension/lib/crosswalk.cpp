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

// NOLINTBEGIN(readability-identifier-naming)

#include "autoware_lanelet2_extension/regulatory_elements/crosswalk.hpp"

#include <boost/variant.hpp>

#include <lanelet2_core/primitives/RegulatoryElement.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

namespace lanelet::autoware
{
namespace
{
template <typename T>
bool findAndErase(const T & primitive, RuleParameters * member)
{
  if (member == nullptr) {
    std::cerr << __FUNCTION__ << ": member is null pointer";
    return false;
  }
  auto it = std::find(member->begin(), member->end(), RuleParameter(primitive));
  if (it == member->end()) {
    return false;
  }
  member->erase(it);
  return true;
}

template <typename T>
Optional<T> tryGetFront(const std::vector<T> & vec)
{
  if (vec.empty()) {
    return {};
  }
  return vec.front();
}

template <typename T>
RuleParameters toRuleParameters(const std::vector<T> & primitives)
{
  auto cast_func = [](const auto & elem) { return static_cast<RuleParameter>(elem); };
  return utils::transform(primitives, cast_func);
}

RegulatoryElementDataPtr constructCrosswalk(
  Id id, const AttributeMap & attributes, const Lanelet & crosswalkLanelet,
  const Polygon3d & crosswalkArea, const LineStrings3d & stopLine)
{
  RuleParameterMap rpm;

  {
    RuleParameters rule_parameters = {crosswalkArea};
    rpm.insert(
      std::make_pair(Crosswalk::AutowareRoleNameString::CrosswalkPolygon, rule_parameters));
  }

  {
    RuleParameters rule_parameters = {crosswalkLanelet};
    rpm.insert(std::make_pair(RoleNameString::Refers, rule_parameters));
  }

  for (const auto & line : stopLine) {
    RuleParameters rule_parameters = {line};
    rpm.insert(std::make_pair(RoleNameString::RefLine, rule_parameters));
  }

  auto data = std::make_shared<RegulatoryElementData>(id, std::move(rpm), attributes);
  data->attributes[AttributeName::Type] = AttributeValueString::RegulatoryElement;
  data->attributes[AttributeName::Subtype] = "crosswalk";
  return data;
}
}  // namespace

Crosswalk::Crosswalk(const RegulatoryElementDataPtr & data) : RegulatoryElement(data)
{
}

Crosswalk::Crosswalk(
  Id id, const AttributeMap & attributes, const Lanelet & crosswalk_lanelet,
  const Polygon3d & crosswalk_area, const LineStrings3d & stop_line)
: Crosswalk(constructCrosswalk(id, attributes, crosswalk_lanelet, crosswalk_area, stop_line))
{
}

ConstPolygons3d Crosswalk::crosswalkAreas() const
{
  return getParameters<ConstPolygon3d>(AutowareRoleNameString::CrosswalkPolygon);
}

ConstLineStrings3d Crosswalk::stopLines() const
{
  return getParameters<ConstLineString3d>(RoleName::RefLine);
}

ConstLanelet Crosswalk::crosswalkLanelet() const
{
  return getParameters<ConstLanelet>(RoleName::Refers).front();
}

void Crosswalk::addCrosswalkArea(const Polygon3d & primitive)
{
  parameters()["crosswalk"].emplace_back(primitive);
}

bool Crosswalk::removeCrosswalkArea(const Polygon3d & primitive)
{
  return findAndErase(primitive, &parameters().find("crosswalk")->second);
}

RegisterRegulatoryElement<Crosswalk> regCrosswalk;

}  // namespace lanelet::autoware

// NOLINTEND(readability-identifier-naming)
