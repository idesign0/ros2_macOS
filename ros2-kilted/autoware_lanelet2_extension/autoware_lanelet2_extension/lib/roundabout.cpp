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

#include "autoware_lanelet2_extension/regulatory_elements/roundabout.hpp"

#include <boost/variant.hpp>

#include <lanelet2_core/primitives/RegulatoryElement.h>

#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

namespace lanelet::autoware
{
namespace
{

template <typename T>
RuleParameters toRuleParameters(const std::vector<T> & primitives)
{
  auto cast_func = [](const auto & elem) { return static_cast<RuleParameter>(elem); };
  return utils::transform(primitives, cast_func);
}

RegulatoryElementDataPtr constructRoundabout(
  Id id, const AttributeMap & attributes, const lanelet::Lanelets & roundabout_entry_lanelets,
  const lanelet::Lanelets & roundabout_exit_lanelets,
  const lanelet::Lanelets & roundabout_internal_lanelets)
{
  RuleParameterMap rpm;

  if (!roundabout_entry_lanelets.empty()) {
    rpm.insert(
      std::make_pair(
        Roundabout::AutowareRoleNameString::Entry, toRuleParameters(roundabout_entry_lanelets)));
  }

  if (!roundabout_exit_lanelets.empty()) {
    rpm.insert(
      std::make_pair(
        Roundabout::AutowareRoleNameString::Exit, toRuleParameters(roundabout_exit_lanelets)));
  }

  if (!roundabout_internal_lanelets.empty()) {
    rpm.insert(
      std::make_pair(
        Roundabout::AutowareRoleNameString::Internal,
        toRuleParameters(roundabout_internal_lanelets)));
  }
  auto data = std::make_shared<RegulatoryElementData>(id, std::move(rpm), attributes);
  data->attributes[AttributeName::Type] = AttributeValueString::RegulatoryElement;
  data->attributes[AttributeName::Subtype] = "roundabout";
  return data;
}
}  // namespace

Roundabout::Roundabout(const RegulatoryElementDataPtr & data) : RegulatoryElement(data)
{
  cacheLaneletIds();
}

Roundabout::Roundabout(
  Id id, const AttributeMap & attributes, const lanelet::Lanelets & roundabout_entry_lanelets,
  const lanelet::Lanelets & roundabout_exit_lanelets,
  const lanelet::Lanelets & roundabout_internal_lanelets)
: Roundabout(constructRoundabout(
    id, attributes, roundabout_entry_lanelets, roundabout_exit_lanelets,
    roundabout_internal_lanelets))
{
}

lanelet::ConstLanelets Roundabout::roundaboutLanelets() const
{
  lanelet::ConstLanelets lanelets;
  auto entry_lanelets = getParameters<lanelet::ConstLanelet>(AutowareRoleNameString::Entry);
  auto exit_lanelets = getParameters<lanelet::ConstLanelet>(AutowareRoleNameString::Exit);
  auto internal_lanelets = getParameters<lanelet::ConstLanelet>(AutowareRoleNameString::Internal);

  lanelets.insert(lanelets.end(), entry_lanelets.begin(), entry_lanelets.end());
  lanelets.insert(lanelets.end(), exit_lanelets.begin(), exit_lanelets.end());
  lanelets.insert(lanelets.end(), internal_lanelets.begin(), internal_lanelets.end());

  return lanelets;
}

lanelet::ConstLanelets Roundabout::roundaboutEntryLanelets() const
{
  return getParameters<lanelet::ConstLanelet>(AutowareRoleNameString::Entry);
}

lanelet::ConstLanelets Roundabout::roundaboutExitLanelets() const
{
  return getParameters<lanelet::ConstLanelet>(AutowareRoleNameString::Exit);
}
lanelet::ConstLanelets Roundabout::roundaboutInternalLanelets() const
{
  return getParameters<lanelet::ConstLanelet>(AutowareRoleNameString::Internal);
}

void Roundabout::cacheLaneletIds()
{
  auto cache = [](const auto & lanelets) {
    std::unordered_set<lanelet::Id> ids;
    ids.reserve(lanelets.size());
    for (const auto & llt : lanelets) {
      ids.insert(llt.id());
    }
    return ids;
  };
  entry_lanelet_ids_ = cache(roundaboutEntryLanelets());
  exit_lanelet_ids_ = cache(roundaboutExitLanelets());
  internal_lanelet_ids_ = cache(roundaboutInternalLanelets());
}

bool Roundabout::isEntryLanelet(const lanelet::Id & lanelet_id) const
{
  return entry_lanelet_ids_.find(lanelet_id) != entry_lanelet_ids_.end();
}

bool Roundabout::isInternalLanelet(const lanelet::Id & lanelet_id) const
{
  return internal_lanelet_ids_.find(lanelet_id) != internal_lanelet_ids_.end();
}

bool Roundabout::isExitLanelet(const lanelet::Id & lanelet_id) const
{
  return exit_lanelet_ids_.find(lanelet_id) != exit_lanelet_ids_.end();
}

bool Roundabout::isRoundaboutLanelet(const lanelet::Id & lanelet_id) const
{
  return isEntryLanelet(lanelet_id) || isExitLanelet(lanelet_id) || isInternalLanelet(lanelet_id);
}

RegisterRegulatoryElement<Roundabout> regRoundabout;

}  // namespace lanelet::autoware

// NOLINTEND(readability-identifier-naming)
