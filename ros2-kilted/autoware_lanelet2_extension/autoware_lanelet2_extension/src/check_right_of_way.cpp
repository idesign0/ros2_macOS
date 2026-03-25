// Copyright 2015-2022 Autoware Foundation
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

#include <autoware_lanelet2_extension/projection/mgrs_projector.hpp>
#include <autoware_lanelet2_extension/utility/query.hpp>
#include <autoware_lanelet2_extension/utility/utilities.hpp>
#include <range/v3/algorithm/move.hpp>
#include <range/v3/core.hpp>
#include <range/v3/functional/overload.hpp>
#include <range/v3/utility/copy.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/unique.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_io/Io.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <set>
#include <utility>
#include <vector>

namespace impl
{
lanelet::ConstLanelets getConflictingLanelets(
  const lanelet::routing::RoutingGraphConstPtr & graph, const lanelet::ConstLanelet & lanelet)
{
  const auto & llt_or_areas = graph->conflicting(lanelet);
  lanelet::ConstLanelets lanelets;
  lanelets.reserve(llt_or_areas.size());
  for (const auto & l_or_a : llt_or_areas) {
    auto llt_opt = l_or_a.lanelet();
    if (!!llt_opt) {
      lanelets.push_back(llt_opt.get());
    }
  }
  return lanelets;
}
}  // namespace impl

int main(int argc, char ** argv)
{
  if (argc < 2) {
    std::cout << "usage: ros2 run autoware_lanelet2_extension check_right_of_way <map_path>";
    return 0;
  }

  // NOLINTBEGIN
  const std::string map_path = std::string(argv[1]);
  // NOLINTEND

  lanelet::ErrorMessages errors{};
  lanelet::projection::MGRSProjector projector{};
  lanelet::LaneletMapPtr map = lanelet::load(map_path, projector, &errors);
  for (auto && error : errors) std::cout << error << std::endl;

  lanelet::traffic_rules::TrafficRulesPtr traffic_rules =
    lanelet::traffic_rules::TrafficRulesFactory::create(
      lanelet::Locations::Germany, lanelet::Participants::Vehicle);
  lanelet::routing::RoutingGraphPtr routing_graph_ptr =
    lanelet::routing::RoutingGraph::build(*map, *traffic_rules);

  auto rows = map->regulatoryElementLayer  // filter elem whose Subtype is RightOfWay
              |
              ranges::views::filter([](auto && elem) {
                const auto & attrs = elem->attributes();
                auto it = attrs.find(lanelet::AttributeName::Subtype);
                return it != attrs.end() && it->second == lanelet::AttributeValueString::RightOfWay;
              })  // transform to lanelet::RightOfWay
              | ranges::views::transform([](auto && elem) {
                  return std::dynamic_pointer_cast<lanelet::RightOfWay>(elem);
                });

  for (auto && row : rows) {
    const auto & right_of_ways = row->rightOfWayLanelets();
    const auto & yields = row->yieldLanelets();
    std::set<lanelet::Id> yield_ids;
    for (auto && yield : yields) {
      yield_ids.insert(yield.id());
    }

    std::set<lanelet::Id> conflicting_ids;
    for (auto && right_of_way : right_of_ways) {
      const std::vector<lanelet::ConstLanelet> & conflicting_lanelets =
        impl::getConflictingLanelets(routing_graph_ptr, right_of_way);
      for (auto && conflict : conflicting_lanelets) conflicting_ids.insert(conflict.id());
    }

    std::vector<lanelet::Id> unnecessary_yields;
    set_difference(
      yield_ids.begin(), yield_ids.end(), conflicting_ids.begin(), conflicting_ids.end(),
      std::inserter(unnecessary_yields, unnecessary_yields.end()));
    if (unnecessary_yields.empty()) continue;
    std::cout << "RightOfWay " << row->id() << ": [";
    const char * delim = "";
    for (auto && unnecessary_yield : unnecessary_yields)
      std::cout << std::exchange(delim, ",") << unnecessary_yield;
    std::cout << "] are unnecessary yield" << std::endl;
  }
}
