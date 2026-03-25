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

// NOLINTBEGIN(readability-identifier-naming)

#include "autoware_lanelet2_extension/regulatory_elements/speed_bump.hpp"

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

Polygons3d getPoly(const RuleParameterMap & paramsMap, RoleName role)
{
  auto params = paramsMap.find(role);
  if (params == paramsMap.end()) {
    return {};
  }

  Polygons3d result;
  for (auto & param : params->second) {
    auto p = boost::get<Polygon3d>(&param);
    if (p != nullptr) {
      result.push_back(*p);
    }
  }
  return result;
}

ConstPolygons3d getConstPoly(const RuleParameterMap & params, RoleName role)
{
  auto cast_func = [](auto & poly) { return static_cast<ConstPolygon3d>(poly); };
  return utils::transform(getPoly(params, role), cast_func);
}

RegulatoryElementDataPtr constructSpeedBumpData(
  Id id, const AttributeMap & attributes, const Polygon3d & speed_bump)
{
  RuleParameterMap rpm;
  RuleParameters rule_parameters = {speed_bump};
  rpm.insert(std::make_pair(RoleNameString::Refers, rule_parameters));

  auto data = std::make_shared<RegulatoryElementData>(id, rpm, attributes);
  data->attributes[AttributeName::Type] = AttributeValueString::RegulatoryElement;
  data->attributes[AttributeName::Subtype] = "speed_bump";
  return data;
}
}  // namespace

SpeedBump::SpeedBump(const RegulatoryElementDataPtr & data) : RegulatoryElement(data)
{
  if (getConstPoly(data->parameters, RoleName::Refers).size() != 1) {
    throw InvalidInputError("There must be exactly one speed bump defined!");
  }
}

SpeedBump::SpeedBump(Id id, const AttributeMap & attributes, const Polygon3d & speed_bump)
: SpeedBump(constructSpeedBumpData(id, attributes, speed_bump))
{
}

ConstPolygon3d SpeedBump::speedBump() const
{
  return getConstPoly(parameters(), RoleName::Refers).front();
}

Polygon3d SpeedBump::speedBump()
{
  return getPoly(parameters(), RoleName::Refers).front();
}

void SpeedBump::addSpeedBump(const Polygon3d & primitive)
{
  parameters()["speed_bump"].emplace_back(primitive);
}

bool SpeedBump::removeSpeedBump(const Polygon3d & primitive)
{
  return findAndErase(primitive, &parameters().find("speed_bump")->second);
}

RegisterRegulatoryElement<SpeedBump> regSpeedBump;

}  // namespace lanelet::autoware

// NOLINTEND(readability-identifier-naming)
