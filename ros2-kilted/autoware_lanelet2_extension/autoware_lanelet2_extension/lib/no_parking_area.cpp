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

// NOLINTBEGIN(readability-identifier-naming)

#include "autoware_lanelet2_extension/regulatory_elements/no_parking_area.hpp"

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

RegulatoryElementDataPtr constructNoParkingAreaData(
  Id id, const AttributeMap & attributes, const Polygons3d & NoParkingAreas)
{
  RuleParameterMap rpm = {{RoleNameString::Refers, toRuleParameters(NoParkingAreas)}};

  auto data = std::make_shared<RegulatoryElementData>(id, std::move(rpm), attributes);
  data->attributes[AttributeName::Type] = AttributeValueString::RegulatoryElement;
  data->attributes[AttributeName::Subtype] = "no_parking_area";
  return data;
}
}  // namespace

NoParkingArea::NoParkingArea(const RegulatoryElementDataPtr & data) : RegulatoryElement(data)
{
  if (getConstPoly(data->parameters, RoleName::Refers).empty()) {
    throw InvalidInputError("no parking area defined!");
  }
}

NoParkingArea::NoParkingArea(
  Id id, const AttributeMap & attributes, const Polygons3d & no_parking_areas)
: NoParkingArea(constructNoParkingAreaData(id, attributes, no_parking_areas))
{
}

ConstPolygons3d NoParkingArea::noParkingAreas() const
{
  return getConstPoly(parameters(), RoleName::Refers);
}
Polygons3d NoParkingArea::noParkingAreas()
{
  return getPoly(parameters(), RoleName::Refers);
}

void NoParkingArea::addNoParkingArea(const Polygon3d & primitive)
{
  parameters()["no_parking_area"].emplace_back(primitive);
}

bool NoParkingArea::removeNoParkingArea(const Polygon3d & primitive)
{
  return findAndErase(primitive, &parameters().find("no_parking_area")->second);
}

RegisterRegulatoryElement<NoParkingArea> regNoParkingArea;

}  // namespace lanelet::autoware

// NOLINTEND(readability-identifier-naming)
