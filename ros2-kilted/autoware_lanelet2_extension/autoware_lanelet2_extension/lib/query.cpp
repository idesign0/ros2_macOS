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
// Authors: Simon Thompson, Ryohsuke Mitsudome

// NOLINTBEGIN(readability-identifier-naming)

#include "autoware_lanelet2_extension/utility/query.hpp"

#include "./deprecated.hpp"
#include "autoware_lanelet2_extension/regulatory_elements/autoware_traffic_light.hpp"
#include "autoware_lanelet2_extension/regulatory_elements/bus_stop_area.hpp"
#include "autoware_lanelet2_extension/regulatory_elements/crosswalk.hpp"
#include "autoware_lanelet2_extension/regulatory_elements/detection_area.hpp"
#include "autoware_lanelet2_extension/regulatory_elements/no_parking_area.hpp"
#include "autoware_lanelet2_extension/regulatory_elements/no_stopping_area.hpp"
#include "autoware_lanelet2_extension/regulatory_elements/speed_bump.hpp"

#include <tf2/utils.hpp>

#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/Lanelet.h>
#include <lanelet2_core/primitives/Lanelet.h>
#include <lanelet2_routing/RoutingGraph.h>

#include <deque>
#include <iostream>
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

using lanelet::utils::to2D;

namespace lanelet::utils
{

namespace query
{
inline namespace format_v2
{
lanelet::ConstLanelets crosswalkLanelets(const lanelet::ConstLanelets & lls)
{
  return subtypeLanelets(lls, lanelet::AttributeValueString::Crosswalk);
}

lanelet::ConstLanelets walkwayLanelets(const lanelet::ConstLanelets & lls)
{
  return subtypeLanelets(lls, lanelet::AttributeValueString::Walkway);
}

lanelet::ConstLanelets shoulderLanelets(const lanelet::ConstLanelets & lls)
{
  return subtypeLanelets(lls, "road_shoulder");
}

lanelet::ConstLanelets bicycleLaneLanelets(const lanelet::ConstLanelets & lls)
{
  return subtypeLanelets(lls, "bicycle_lane");
}

std::vector<lanelet::TrafficLightConstPtr> trafficLights(const lanelet::ConstLanelets & lanelets)
{
  std::vector<lanelet::TrafficLightConstPtr> tl_reg_elems;

  for (const auto & ll : lanelets) {
    std::vector<lanelet::TrafficLightConstPtr> ll_tl_re =
      ll.regulatoryElementsAs<lanelet::TrafficLight>();

    // insert unique tl into array
    for (const auto & tl_ptr : ll_tl_re) {
      lanelet::Id id = tl_ptr->id();
      bool unique_id = true;
      for (const auto & tl_reg_elem : tl_reg_elems) {
        if (id == tl_reg_elem->id()) {
          unique_id = false;
          break;
        }
      }
      if (unique_id) {
        tl_reg_elems.push_back(tl_ptr);
      }
    }
  }
  return tl_reg_elems;
}

std::vector<lanelet::AutowareTrafficLightConstPtr> autowareTrafficLights(
  const lanelet::ConstLanelets & lanelets)
{
  std::vector<lanelet::AutowareTrafficLightConstPtr> tl_reg_elems;

  for (const auto & ll : lanelets) {
    std::vector<lanelet::AutowareTrafficLightConstPtr> ll_tl_re =
      ll.regulatoryElementsAs<lanelet::autoware::AutowareTrafficLight>();

    // insert unique tl into array
    for (const auto & tl_ptr : ll_tl_re) {
      lanelet::Id id = tl_ptr->id();
      bool unique_id = true;

      for (const auto & tl_reg_elem : tl_reg_elems) {
        if (id == tl_reg_elem->id()) {
          unique_id = false;
          break;
        }
      }

      if (unique_id) {
        tl_reg_elems.push_back(tl_ptr);
      }
    }
  }
  return tl_reg_elems;
}

std::vector<lanelet::DetectionAreaConstPtr> detectionAreas(const lanelet::ConstLanelets & lanelets)
{
  std::vector<lanelet::DetectionAreaConstPtr> da_reg_elems;

  for (const auto & ll : lanelets) {
    std::vector<lanelet::DetectionAreaConstPtr> ll_da_re =
      ll.regulatoryElementsAs<lanelet::autoware::DetectionArea>();

    // insert unique tl into array
    for (const auto & da_ptr : ll_da_re) {
      lanelet::Id id = da_ptr->id();
      bool unique_id = true;

      for (const auto & da_reg_elem : da_reg_elems) {
        if (id == da_reg_elem->id()) {
          unique_id = false;
          break;
        }
      }

      if (unique_id) {
        da_reg_elems.push_back(da_ptr);
      }
    }
  }
  return da_reg_elems;
}

std::vector<lanelet::NoStoppingAreaConstPtr> noStoppingAreas(
  const lanelet::ConstLanelets & lanelets)
{
  std::vector<lanelet::NoStoppingAreaConstPtr> no_reg_elems;

  for (const auto & ll : lanelets) {
    std::vector<lanelet::NoStoppingAreaConstPtr> ll_no_re =
      ll.regulatoryElementsAs<lanelet::autoware::NoStoppingArea>();

    // insert unique tl into array
    for (const auto & no_ptr : ll_no_re) {
      lanelet::Id id = no_ptr->id();
      bool unique_id = true;

      for (const auto & no_reg_elem : no_reg_elems) {
        if (id == no_reg_elem->id()) {
          unique_id = false;
          break;
        }
      }

      if (unique_id) {
        no_reg_elems.push_back(no_ptr);
      }
    }
  }
  return no_reg_elems;
}

std::vector<lanelet::NoParkingAreaConstPtr> noParkingAreas(const lanelet::ConstLanelets & lanelets)
{
  std::vector<lanelet::NoParkingAreaConstPtr> no_pa_reg_elems;

  for (const auto & ll : lanelets) {
    std::vector<lanelet::NoParkingAreaConstPtr> ll_no_pa_re =
      ll.regulatoryElementsAs<lanelet::autoware::NoParkingArea>();

    // insert unique NoParkingArea into array
    for (const auto & no_pa_ptr : ll_no_pa_re) {
      lanelet::Id id = no_pa_ptr->id();
      bool unique_id = true;

      for (const auto & no_reg_elem : no_pa_reg_elems) {
        if (id == no_reg_elem->id()) {
          unique_id = false;
          break;
        }
      }

      if (unique_id) {
        no_pa_reg_elems.push_back(no_pa_ptr);
      }
    }
  }
  return no_pa_reg_elems;
}

std::vector<lanelet::BusStopAreaConstPtr> busStopAreas(const lanelet::ConstLanelets & lanelets)
{
  std::vector<lanelet::BusStopAreaConstPtr> bus_stop_area_reg_elems;
  std::set<lanelet::Id> found_ids;

  for (const auto & ll : lanelets) {
    std::vector<lanelet::BusStopAreaConstPtr> reg_elems =
      ll.regulatoryElementsAs<lanelet::autoware::BusStopArea>();
    for (const auto & reg_elem : reg_elems) {
      const auto id = reg_elem->id();
      if (found_ids.find(id) == found_ids.end()) {
        found_ids.insert(id);
        bus_stop_area_reg_elems.push_back(reg_elem);
      }
    }
  }
  return bus_stop_area_reg_elems;
}

std::vector<lanelet::SpeedBumpConstPtr> speedBumps(const lanelet::ConstLanelets & lanelets)
{
  std::vector<lanelet::SpeedBumpConstPtr> sb_reg_elems;

  for (const auto & ll : lanelets) {
    std::vector<lanelet::SpeedBumpConstPtr> ll_sb_re =
      ll.regulatoryElementsAs<lanelet::autoware::SpeedBump>();

    // insert unique id into array
    for (const auto & sb_ptr : ll_sb_re) {
      lanelet::Id id = sb_ptr->id();
      bool unique_id = true;

      for (const auto & sb_reg_elem : sb_reg_elems) {
        if (id == sb_reg_elem->id()) {
          unique_id = false;
          break;
        }
      }

      if (unique_id) {
        sb_reg_elems.push_back(sb_ptr);
      }
    }
  }
  return sb_reg_elems;
}

std::vector<lanelet::CrosswalkConstPtr> crosswalks(const lanelet::ConstLanelets & lanelets)
{
  std::vector<lanelet::CrosswalkConstPtr> cw_reg_elems;

  for (const auto & ll : lanelets) {
    std::vector<lanelet::CrosswalkConstPtr> ll_cw_re =
      ll.regulatoryElementsAs<lanelet::autoware::Crosswalk>();

    // insert unique id into array
    for (const auto & cw_ptr : ll_cw_re) {
      lanelet::Id id = cw_ptr->id();
      bool unique_id = true;

      for (const auto & cw_reg_elem : cw_reg_elems) {
        if (id == cw_reg_elem->id()) {
          unique_id = false;
          break;
        }
      }

      if (unique_id) {
        cw_reg_elems.push_back(cw_ptr);
      }
    }
  }
  return cw_reg_elems;
}

lanelet::ConstLineStrings3d curbstones(const lanelet::LaneletMapConstPtr & lanelet_map_ptr)
{
  lanelet::ConstLineStrings3d curbstones;
  for (const auto & ls : lanelet_map_ptr->lineStringLayer) {
    const std::string type = ls.attributeOr(lanelet::AttributeName::Type, "none");
    if (type == "curbstone") {
      curbstones.push_back(ls);
    }
  }
  return curbstones;
}

lanelet::ConstPolygons3d getAllObstaclePolygons(const lanelet::LaneletMapConstPtr & lanelet_map_ptr)
{
  lanelet::ConstPolygons3d obstacle_polygons;
  for (const auto & poly : lanelet_map_ptr->polygonLayer) {
    const std::string type = poly.attributeOr(lanelet::AttributeName::Type, "none");
    if (type == "obstacle") {
      obstacle_polygons.push_back(poly);
    }
  }
  return obstacle_polygons;
}

lanelet::ConstPolygons3d getAllParkingLots(const lanelet::LaneletMapConstPtr & lanelet_map_ptr)
{
  lanelet::ConstPolygons3d parking_lots;
  for (const auto & poly : lanelet_map_ptr->polygonLayer) {
    const std::string type = poly.attributeOr(lanelet::AttributeName::Type, "none");
    if (type == "parking_lot") {
      parking_lots.push_back(poly);
    }
  }
  return parking_lots;
}

lanelet::ConstLineStrings3d getAllLinestringsWithType(
  const lanelet::LaneletMapConstPtr & lanelet_map_ptr, const std::string & type)
{
  lanelet::ConstLineStrings3d linestrings_with_type;
  for (const auto & ls : lanelet_map_ptr->lineStringLayer) {
    const std::string ls_type = ls.attributeOr(lanelet::AttributeName::Type, "none");
    if (ls_type == type) {
      linestrings_with_type.push_back(ls);
    }
  }
  return linestrings_with_type;
}

lanelet::ConstLineStrings3d getAllPartitions(const lanelet::LaneletMapConstPtr & lanelet_map_ptr)
{
  lanelet::ConstLineStrings3d partitions;
  for (const auto & ls : lanelet_map_ptr->lineStringLayer) {
    const std::string type = ls.attributeOr(lanelet::AttributeName::Type, "none");
    if (type == "guard_rail" || type == "fence" || type == "wall") {
      partitions.push_back(ls);
    }
  }
  return partitions;
}

lanelet::ConstLineStrings3d getAllFences(const lanelet::LaneletMapConstPtr & lanelet_map_ptr)
{
  lanelet::ConstLineStrings3d fences;
  for (const auto & ls : lanelet_map_ptr->lineStringLayer) {
    const std::string type = ls.attributeOr(lanelet::AttributeName::Type, "none");
    if (type == "fence") {
      fences.push_back(ls);
    }
  }
  return fences;
}

lanelet::ConstLineStrings3d getAllPedestrianPolygonMarkings(
  const lanelet::LaneletMapConstPtr & lanelet_map_ptr)
{
  lanelet::ConstLineStrings3d pedestrian_polygon_markings;
  for (const auto & ls : lanelet_map_ptr->lineStringLayer) {
    const std::string type = ls.attributeOr(lanelet::AttributeName::Type, "none");
    if ((type == "pedestrian_marking") && (ls.size() >= 3)) {
      pedestrian_polygon_markings.push_back(ls);
    }
  }
  return pedestrian_polygon_markings;
}

lanelet::ConstLineStrings3d getAllPedestrianLineMarkings(
  const lanelet::LaneletMapConstPtr & lanelet_map_ptr)
{
  lanelet::ConstLineStrings3d pedestrian_line_markings;
  for (const auto & ls : lanelet_map_ptr->lineStringLayer) {
    const std::string type = ls.attributeOr(lanelet::AttributeName::Type, "none");
    if ((type == "pedestrian_marking") && (ls.size() < 3)) {
      pedestrian_line_markings.push_back(ls);
    }
  }
  return pedestrian_line_markings;
}

lanelet::ConstLineStrings3d getAllParkingSpaces(const lanelet::LaneletMapConstPtr & lanelet_map_ptr)
{
  lanelet::ConstLineStrings3d parking_spaces;
  for (const auto & ls : lanelet_map_ptr->lineStringLayer) {
    const std::string type = ls.attributeOr(lanelet::AttributeName::Type, "none");
    if (type == "parking_space") {
      parking_spaces.push_back(ls);
    }
  }
  return parking_spaces;
}

lanelet::ConstLineStrings3d getAllWaypoints(const lanelet::LaneletMapConstPtr & lanelet_map_ptr)
{
  lanelet::ConstLineStrings3d waypoints;
  for (const auto & ll : lanelet_map_ptr->laneletLayer) {
    if (ll.hasAttribute("waypoints")) {
      const auto waypoints_id = ll.attribute("waypoints").asId().value();
      waypoints.push_back(lanelet_map_ptr->lineStringLayer.get(waypoints_id));
    }
  }
  return waypoints;
}

bool getLinkedLanelet(
  const lanelet::ConstLineString3d & parking_space,
  const lanelet::LaneletMapConstPtr & lanelet_map_ptr, lanelet::ConstLanelet * linked_lanelet)
{
  const auto & all_lanelets = laneletLayer(lanelet_map_ptr);
  const auto & all_road_lanelets = roadLanelets(all_lanelets);
  const auto & all_parking_lots = getAllParkingLots(lanelet_map_ptr);
  return getLinkedLanelet(parking_space, all_road_lanelets, all_parking_lots, linked_lanelet);
}

bool getLinkedLanelet(
  const lanelet::ConstLineString3d & parking_space,
  const lanelet::ConstLanelets & all_road_lanelets,
  const lanelet::ConstPolygons3d & all_parking_lots, lanelet::ConstLanelet * linked_lanelet)
{
  const auto & linked_lanelets =
    getLinkedLanelets(parking_space, all_road_lanelets, all_parking_lots);
  if (linked_lanelets.empty()) {
    return false;
  }

  double min_distance = std::numeric_limits<double>::max();
  for (const auto & lanelet : linked_lanelets) {
    const double distance = boost::geometry::distance(
      to2D(parking_space).basicLineString(), lanelet.polygon2d().basicPolygon());
    if (distance < min_distance) {
      *linked_lanelet = lanelet;
      min_distance = distance;
    }
  }
  return true;
}

lanelet::ConstLanelets getLinkedLanelets(
  const lanelet::ConstLineString3d & parking_space,
  const lanelet::LaneletMapConstPtr & lanelet_map_ptr)
{
  const auto & all_lanelets = laneletLayer(lanelet_map_ptr);
  const auto & all_road_lanelets = roadLanelets(all_lanelets);
  const auto & all_parking_lots = getAllParkingLots(lanelet_map_ptr);

  return getLinkedLanelets(parking_space, all_road_lanelets, all_parking_lots);
}

lanelet::ConstLanelets getLinkedLanelets(
  const lanelet::ConstLineString3d & parking_space,
  const lanelet::ConstLanelets & all_road_lanelets,
  const lanelet::ConstPolygons3d & all_parking_lots)
{
  lanelet::ConstLanelets linked_lanelets;

  // get lanelets within same parking lot
  lanelet::ConstPolygon3d linked_parking_lot;
  if (!getLinkedParkingLot(parking_space, all_parking_lots, &linked_parking_lot)) {
    return linked_lanelets;
  }
  const auto & candidate_lanelets = getLinkedLanelets(linked_parking_lot, all_road_lanelets);

  // get lanelets that are close to parking space and facing to parking space
  for (const auto & lanelet : candidate_lanelets) {
    // check if parking space is close to lanelet
    const double distance = boost::geometry::distance(
      to2D(parking_space).basicLineString(), lanelet.polygon2d().basicPolygon());
    constexpr double distance_thresh = 5.0;
    if (distance > distance_thresh) {
      continue;
    }

    // check if parking space is facing lanelet
    const Eigen::Vector3d direction =
      parking_space.back().basicPoint() - parking_space.front().basicPoint();
    const Eigen::Vector3d new_pt = parking_space.front().basicPoint() - direction * distance_thresh;

    const lanelet::Point3d check_line_p1(lanelet::InvalId, new_pt.x(), new_pt.y(), new_pt.z());
    const lanelet::Point3d check_line_p2(lanelet::InvalId, parking_space.back().basicPoint());
    const lanelet::LineString3d check_line(lanelet::InvalId, {check_line_p1, check_line_p2});

    const double new_distance = boost::geometry::distance(
      to2D(check_line).basicLineString(), lanelet.polygon2d().basicPolygon());
    if (new_distance < std::numeric_limits<double>::epsilon()) {
      linked_lanelets.push_back(lanelet);
    }
  }

  return linked_lanelets;
}

// get overlapping lanelets
lanelet::ConstLanelets getLinkedLanelets(
  const lanelet::ConstPolygon3d & parking_lot, const lanelet::ConstLanelets & all_road_lanelets)
{
  lanelet::ConstLanelets linked_lanelets;
  for (const auto & lanelet : all_road_lanelets) {
    const double distance = boost::geometry::distance(
      lanelet.polygon2d().basicPolygon(), to2D(parking_lot).basicPolygon());
    if (distance < std::numeric_limits<double>::epsilon()) {
      linked_lanelets.push_back(lanelet);
    }
  }
  return linked_lanelets;
}

lanelet::ConstLineStrings3d getLinkedParkingSpaces(
  const lanelet::ConstLanelet & lanelet, const lanelet::LaneletMapConstPtr & lanelet_map_ptr)
{
  const auto & all_parking_spaces = getAllParkingSpaces(lanelet_map_ptr);
  const auto & all_parking_lots = getAllParkingLots(lanelet_map_ptr);
  return getLinkedParkingSpaces(lanelet, all_parking_spaces, all_parking_lots);
}

lanelet::ConstLineStrings3d getLinkedParkingSpaces(
  const lanelet::ConstLanelet & lanelet, const lanelet::ConstLineStrings3d & all_parking_spaces,
  const lanelet::ConstPolygons3d & all_parking_lots)
{
  lanelet::ConstLineStrings3d linked_parking_spaces;

  // get parking spaces that are in same parking lot.
  lanelet::ConstPolygon3d linked_parking_lot;
  if (!getLinkedParkingLot(lanelet, all_parking_lots, &linked_parking_lot)) {
    return linked_parking_spaces;
  }
  const auto & possible_parking_spaces =
    getLinkedParkingSpaces(linked_parking_lot, all_parking_spaces);

  // check for parking spaces that are within 5m and facing towards lanelet
  for (const auto & parking_space : possible_parking_spaces) {
    // check if parking space is close to lanelet
    const double distance = boost::geometry::distance(
      to2D(parking_space).basicLineString(), lanelet.polygon2d().basicPolygon());
    constexpr double distance_thresh = 5.0;
    if (distance > distance_thresh) {
      continue;
    }

    // check if parking space is facing lanelet
    const Eigen::Vector3d direction =
      parking_space.back().basicPoint() - parking_space.front().basicPoint();
    const Eigen::Vector3d new_pt = parking_space.front().basicPoint() - direction * distance_thresh;

    const lanelet::Point3d check_line_p1(lanelet::InvalId, new_pt.x(), new_pt.y(), new_pt.z());
    const lanelet::Point3d check_line_p2(lanelet::InvalId, parking_space.back().basicPoint());
    const lanelet::LineString3d check_line(lanelet::InvalId, {check_line_p1, check_line_p2});

    const double new_distance = boost::geometry::distance(
      to2D(check_line).basicLineString(), lanelet.polygon2d().basicPolygon());
    if (new_distance < std::numeric_limits<double>::epsilon()) {
      linked_parking_spaces.push_back(parking_space);
    }
  }
  return linked_parking_spaces;
}

// get overlapping parking lot
bool getLinkedParkingLot(
  const lanelet::ConstLanelet & lanelet, const lanelet::ConstPolygons3d & all_parking_lots,
  lanelet::ConstPolygon3d * linked_parking_lot)
{
  for (const auto & parking_lot : all_parking_lots) {
    const double distance = boost::geometry::distance(
      lanelet.polygon2d().basicPolygon(), to2D(parking_lot).basicPolygon());
    if (distance < std::numeric_limits<double>::epsilon()) {
      *linked_parking_lot = parking_lot;
      return true;
    }
  }
  return false;
}

// get overlapping parking lot
bool getLinkedParkingLot(
  const lanelet::BasicPoint2d & current_position, const lanelet::ConstPolygons3d & all_parking_lots,
  lanelet::ConstPolygon3d * linked_parking_lot)
{
  for (const auto & parking_lot : all_parking_lots) {
    const double distance =
      boost::geometry::distance(current_position, to2D(parking_lot).basicPolygon());
    if (distance < std::numeric_limits<double>::epsilon()) {
      *linked_parking_lot = parking_lot;
      return true;
    }
  }
  return false;
}
bool getLinkedParkingLot(
  const lanelet::BasicPoint2d & current_position,
  const lanelet::LaneletMapConstPtr & lanelet_map_ptr, lanelet::ConstPolygon3d * linked_parking_lot)
{
  auto candidates =
    lanelet_map_ptr->polygonLayer.search(lanelet::geometry::boundingBox2d(current_position));
  candidates.erase(
    std::remove_if(
      candidates.begin(), candidates.end(),
      [](const auto & c) {
        const std::string type = c.attributeOr(lanelet::AttributeName::Type, "none");
        return type != "parking_lot";
      }),
    candidates.end());
  return getLinkedParkingLot(current_position, candidates, linked_parking_lot);
}

// get overlapping parking lot
bool getLinkedParkingLot(
  const lanelet::ConstLineString3d & parking_space,
  const lanelet::ConstPolygons3d & all_parking_lots, lanelet::ConstPolygon3d * linked_parking_lot)
{
  for (const auto & parking_lot : all_parking_lots) {
    const double distance = boost::geometry::distance(
      to2D(parking_space).basicLineString(), to2D(parking_lot).basicPolygon());
    if (distance < std::numeric_limits<double>::epsilon()) {
      *linked_parking_lot = parking_lot;
      return true;
    }
  }
  return false;
}

lanelet::ConstLineStrings3d getLinkedParkingSpaces(
  const lanelet::ConstPolygon3d & parking_lot,
  const lanelet::ConstLineStrings3d & all_parking_spaces)
{
  lanelet::ConstLineStrings3d linked_parking_spaces;
  for (const auto & parking_space : all_parking_spaces) {
    const double distance = boost::geometry::distance(
      to2D(parking_space).basicLineString(), to2D(parking_lot).basicPolygon());
    if (distance < std::numeric_limits<double>::epsilon()) {
      linked_parking_spaces.push_back(parking_space);
    }
  }
  return linked_parking_spaces;
}

// return all stop lines and ref lines from a given set of lanelets
std::vector<lanelet::ConstLineString3d> stopLinesLanelets(const lanelet::ConstLanelets & lanelets)
{
  std::vector<lanelet::ConstLineString3d> stoplines;

  for (const auto & ll : lanelets) {
    std::vector<lanelet::ConstLineString3d> ll_stoplines;
    ll_stoplines = stopLinesLanelet(ll);
    stoplines.insert(stoplines.end(), ll_stoplines.begin(), ll_stoplines.end());
  }

  return stoplines;
}

// return all stop and ref lines from a given lanelet
std::vector<lanelet::ConstLineString3d> stopLinesLanelet(const lanelet::ConstLanelet & ll)
{
  std::vector<lanelet::ConstLineString3d> stoplines;

  // find stop lines referenced by right of way reg. elems.
  std::vector<std::shared_ptr<const lanelet::RightOfWay>> right_of_way_reg_elems =
    ll.regulatoryElementsAs<const lanelet::RightOfWay>();

  if (!right_of_way_reg_elems.empty()) {
    // lanelet has a right of way elem element
    for (auto j = right_of_way_reg_elems.begin(); j < right_of_way_reg_elems.end(); j++) {
      if ((*j)->getManeuver(ll) == lanelet::ManeuverType::Yield) {
        // lanelet has a yield reg. elem.
        lanelet::Optional<lanelet::ConstLineString3d> row_stopline_opt = (*j)->stopLine();
        if (!!row_stopline_opt) {
          stoplines.push_back(row_stopline_opt.get());
        }
      }
    }
  }

  // find stop lines referenced by traffic lights
  std::vector<std::shared_ptr<const lanelet::TrafficLight>> traffic_light_reg_elems =
    ll.regulatoryElementsAs<const lanelet::TrafficLight>();

  if (!traffic_light_reg_elems.empty()) {
    // lanelet has a traffic light elem element
    for (auto j = traffic_light_reg_elems.begin(); j < traffic_light_reg_elems.end(); j++) {
      lanelet::Optional<lanelet::ConstLineString3d> traffic_light_stopline_opt = (*j)->stopLine();
      if (!!traffic_light_stopline_opt) {
        stoplines.push_back(traffic_light_stopline_opt.get());
      }
    }
  }
  // find stop lines referenced by traffic signs
  std::vector<std::shared_ptr<const lanelet::TrafficSign>> traffic_sign_reg_elems =
    ll.regulatoryElementsAs<const lanelet::TrafficSign>();

  if (!traffic_sign_reg_elems.empty()) {
    // lanelet has a traffic sign reg elem - can have multiple ref lines (but
    // stop sign shod have 1
    for (auto j = traffic_sign_reg_elems.begin(); j < traffic_sign_reg_elems.end(); j++) {
      lanelet::ConstLineStrings3d traffic_sign_stoplines = (*j)->refLines();
      if (!traffic_sign_stoplines.empty()) {
        stoplines.push_back(traffic_sign_stoplines.front());
      }
    }
  }
  return stoplines;
}

std::vector<lanelet::ConstLineString3d> stopSignStopLines(
  const lanelet::ConstLanelets & lanelets, const std::string & stop_sign_id)
{
  std::vector<lanelet::ConstLineString3d> stoplines;

  std::set<lanelet::Id> checklist;

  for (const auto & ll : lanelets) {
    // find stop lines referenced by traffic signs
    std::vector<std::shared_ptr<const lanelet::TrafficSign>> traffic_sign_reg_elems =
      ll.regulatoryElementsAs<const lanelet::TrafficSign>();

    if (!traffic_sign_reg_elems.empty()) {
      // lanelet has a traffic sign reg elem - can have multiple ref lines (but
      // stop sign shod have 1
      for (const auto & ts : traffic_sign_reg_elems) {
        // skip if traffic sign is not stop sign
        if (ts->type() != stop_sign_id) {
          continue;
        }

        lanelet::ConstLineStrings3d traffic_sign_stoplines = ts->refLines();

        // only add new items
        if (!traffic_sign_stoplines.empty()) {
          auto id = traffic_sign_stoplines.front().id();
          if (checklist.find(id) == checklist.end()) {
            checklist.insert(id);
            stoplines.push_back(traffic_sign_stoplines.front());
          }
        }
      }
    }
  }
  return stoplines;
}
}  // namespace format_v2
}  // namespace query

// returns all lanelets in laneletLayer - don't know how to convert
// PrimitiveLayer<Lanelets> -> std::vector<Lanelets>
lanelet::ConstLanelets query::laneletLayer(const lanelet::LaneletMapConstPtr & ll_map)
{
  lanelet::ConstLanelets lanelets;
  if (!ll_map) {
    std::cerr << "No map received!";
    return lanelets;
  }

  for (const auto & li : ll_map->laneletLayer) {
    lanelets.push_back(li);
  }

  return lanelets;
}

lanelet::ConstLanelets query::subtypeLanelets(
  const lanelet::ConstLanelets & lls, const char subtype[])
{
  lanelet::ConstLanelets subtype_lanelets;

  for (const auto & ll : lls) {
    if (ll.hasAttribute(lanelet::AttributeName::Subtype)) {
      const lanelet::Attribute & attr = ll.attribute(lanelet::AttributeName::Subtype);
      if (attr.value() == subtype) {
        subtype_lanelets.push_back(ll);
      }
    }
  }

  return subtype_lanelets;
}

lanelet::ConstLanelets query::roadLanelets(const lanelet::ConstLanelets & lls)
{
  return query::subtypeLanelets(lls, lanelet::AttributeValueString::Road);
}

lanelet::ConstPolygons3d query::getAllPolygonsByType(
  const lanelet::LaneletMapConstPtr & lanelet_map_ptr, const std::string & polygon_type)
{
  lanelet::ConstPolygons3d polygons;
  for (const auto & poly : lanelet_map_ptr->polygonLayer) {
    const std::string type = poly.attributeOr(lanelet::AttributeName::Type, "none");
    if (type == polygon_type) {
      polygons.push_back(poly);
    }
  }
  return polygons;
}

ConstLanelets query::getLaneletsWithinRange(
  const lanelet::ConstLanelets & lanelets, const lanelet::BasicPoint2d & search_point,
  const double range)
{
  ConstLanelets near_lanelets;
  for (const auto & ll : lanelets) {
    lanelet::BasicPolygon2d poly = ll.polygon2d().basicPolygon();
    double distance = lanelet::geometry::distance(poly, search_point);
    if (distance <= range) {
      near_lanelets.push_back(ll);
    }
  }
  return near_lanelets;
}

ConstLanelets query::getLaneletsWithinRange(
  const lanelet::ConstLanelets & lanelets, const geometry_msgs::msg::Point & search_point,
  const double range)
{
  return deprecated::getLaneletsWithinRange(
    lanelets, lanelet::BasicPoint2d(search_point.x, search_point.y), range);
}

ConstLanelets query::getLaneChangeableNeighbors(
  const routing::RoutingGraphPtr & graph, const ConstLanelet & lanelet)
{
  return graph->besides(lanelet);
}

ConstLanelets query::getLaneChangeableNeighbors(
  const routing::RoutingGraphPtr & graph, const ConstLanelets & road_lanelets,
  const geometry_msgs::msg::Point & search_point)
{
  const auto lanelets = deprecated::getLaneletsWithinRange(
    road_lanelets, search_point, std::numeric_limits<double>::epsilon());
  ConstLanelets road_slices;
  for (const auto & llt : lanelets) {
    const auto tmp_road_slice = deprecated::getLaneChangeableNeighbors(graph, llt);
    road_slices.insert(road_slices.end(), tmp_road_slice.begin(), tmp_road_slice.end());
  }
  return road_slices;
}

ConstLanelets query::getAllNeighbors(
  const routing::RoutingGraphPtr & graph, const ConstLanelet & lanelet)
{
  ConstLanelets lanelets;

  ConstLanelets left_lanelets = deprecated::getAllNeighborsLeft(graph, lanelet);
  ConstLanelets right_lanelets = deprecated::getAllNeighborsRight(graph, lanelet);

  std::reverse(left_lanelets.begin(), left_lanelets.end());
  lanelets.insert(lanelets.end(), left_lanelets.begin(), left_lanelets.end());
  lanelets.push_back(lanelet);
  lanelets.insert(lanelets.end(), right_lanelets.begin(), right_lanelets.end());

  return lanelets;
}

ConstLanelets query::getAllNeighborsRight(
  const routing::RoutingGraphPtr & graph, const ConstLanelet & lanelet)
{
  ConstLanelets lanelets;
  auto right_lane =
    (!!graph->right(lanelet)) ? graph->right(lanelet) : graph->adjacentRight(lanelet);
  while (!!right_lane) {
    lanelets.push_back(right_lane.get());
    right_lane = (!!graph->right(right_lane.get())) ? graph->right(right_lane.get())
                                                    : graph->adjacentRight(right_lane.get());
  }
  return lanelets;
}

ConstLanelets query::getAllNeighborsLeft(
  const routing::RoutingGraphPtr & graph, const ConstLanelet & lanelet)
{
  ConstLanelets lanelets;
  auto left_lane = (!!graph->left(lanelet)) ? graph->left(lanelet) : graph->adjacentLeft(lanelet);
  while (!!left_lane) {
    lanelets.push_back(left_lane.get());
    left_lane = (!!graph->left(left_lane.get())) ? graph->left(left_lane.get())
                                                 : graph->adjacentLeft(left_lane.get());
  }
  return lanelets;
}

ConstLanelets query::getAllNeighbors(
  const routing::RoutingGraphPtr & graph, const ConstLanelets & road_lanelets,
  const geometry_msgs::msg::Point & search_point)
{
  const auto lanelets = deprecated::getLaneletsWithinRange(
    road_lanelets, search_point, std::numeric_limits<double>::epsilon());
  ConstLanelets road_slices;
  for (const auto & llt : lanelets) {
    const auto tmp_road_slice = deprecated::getAllNeighbors(graph, llt);
    road_slices.insert(road_slices.end(), tmp_road_slice.begin(), tmp_road_slice.end());
  }
  return road_slices;
}

bool query::getClosestLanelet(
  const ConstLanelets & lanelets, const geometry_msgs::msg::Pose & search_pose,
  ConstLanelet * closest_lanelet_ptr)
{
  if (closest_lanelet_ptr == nullptr) {
    std::cerr << "argument closest_lanelet_ptr is null! Failed to find closest lanelet"
              << std::endl;
    return false;
  }

  if (lanelets.empty()) {
    return false;
  }

  bool found = false;

  lanelet::BasicPoint2d search_point(search_pose.position.x, search_pose.position.y);

  // find by distance
  lanelet::ConstLanelets candidate_lanelets;
  {
    double min_distance = std::numeric_limits<double>::max();
    for (const auto & llt : lanelets) {
      double distance =
        boost::geometry::comparable_distance(llt.polygon2d().basicPolygon(), search_point);

      if (std::abs(distance - min_distance) <= std::numeric_limits<double>::epsilon()) {
        candidate_lanelets.push_back(llt);
      } else if (distance < min_distance) {
        found = true;
        candidate_lanelets.clear();
        candidate_lanelets.push_back(llt);
        min_distance = distance;
      }
    }
  }

  if (candidate_lanelets.size() == 1) {
    *closest_lanelet_ptr = candidate_lanelets.at(0);
    return found;
  }

  // find by angle
  {
    double min_angle = std::numeric_limits<double>::max();
    double pose_yaw = tf2::getYaw(search_pose.orientation);
    for (const auto & llt : candidate_lanelets) {
      lanelet::ConstLineString3d segment =
        deprecated::getClosestSegment(search_point, llt.centerline());
      double angle_diff = M_PI;
      if (!segment.empty()) {
        double segment_angle = std::atan2(
          segment.back().y() - segment.front().y(), segment.back().x() - segment.front().x());
        angle_diff = std::abs(deprecated::normalize_radian(segment_angle - pose_yaw));
      }
      if (angle_diff < min_angle) {
        min_angle = angle_diff;
        *closest_lanelet_ptr = llt;
      }
    }
  }

  return found;
}

bool query::getClosestLaneletWithConstrains(
  const ConstLanelets & lanelets, const geometry_msgs::msg::Pose & search_pose,
  ConstLanelet * closest_lanelet_ptr, const double dist_threshold, const double yaw_threshold)
{
  bool found = false;

  if (closest_lanelet_ptr == nullptr) {
    std::cerr << "argument closest_lanelet_ptr is null! Failed to find closest lanelet"
              << std::endl;
    return found;
  }

  if (lanelets.empty()) {
    return found;
  }

  lanelet::BasicPoint2d search_point(search_pose.position.x, search_pose.position.y);

  // find by distance
  std::vector<std::pair<lanelet::ConstLanelet, double>> candidate_lanelets;
  {
    for (const auto & llt : lanelets) {
      double distance = boost::geometry::distance(llt.polygon2d().basicPolygon(), search_point);

      if (distance <= dist_threshold) {
        candidate_lanelets.emplace_back(llt, distance);
      }
    }

    if (!candidate_lanelets.empty()) {
      // sort by distance
      std::sort(
        candidate_lanelets.begin(), candidate_lanelets.end(),
        [](
          const std::pair<lanelet::ConstLanelet, double> & x,
          std::pair<lanelet::ConstLanelet, double> & y) { return x.second < y.second; });
    } else {
      return found;
    }
  }

  // find closest lanelet within yaw_threshold
  {
    double min_angle = std::numeric_limits<double>::max();
    double min_distance = std::numeric_limits<double>::max();
    double pose_yaw = tf2::getYaw(search_pose.orientation);
    for (const auto & llt_pair : candidate_lanelets) {
      const auto & distance = llt_pair.second;

      double lanelet_angle = deprecated::getLaneletAngle(llt_pair.first, search_pose.position);
      double angle_diff = std::abs(deprecated::normalize_radian(lanelet_angle - pose_yaw));

      if (angle_diff > std::abs(yaw_threshold)) continue;
      if (min_distance < distance) break;

      if (angle_diff < min_angle) {
        min_angle = angle_diff;
        min_distance = distance;
        *closest_lanelet_ptr = llt_pair.first;
        found = true;
      }
    }
  }

  return found;
}

bool query::getCurrentLanelets(
  const ConstLanelets & lanelets, const geometry_msgs::msg::Point & search_point,
  ConstLanelets * current_lanelets_ptr)
{
  if (current_lanelets_ptr == nullptr) {
    std::cerr << "argument closest_lanelet_ptr is null! Failed to find closest lanelet"
              << std::endl;
    return false;
  }

  if (lanelets.empty()) {
    return false;
  }

  lanelet::BasicPoint2d search_point_2d(search_point.x, search_point.y);
  for (const auto & llt : lanelets) {
    if (lanelet::geometry::inside(llt, search_point_2d)) {
      current_lanelets_ptr->push_back(llt);
    }
  }

  return !current_lanelets_ptr->empty();  // return found
}

bool query::getCurrentLanelets(
  const ConstLanelets & lanelets, const geometry_msgs::msg::Pose & search_pose,
  ConstLanelets * current_lanelets_ptr)
{
  if (current_lanelets_ptr == nullptr) {
    std::cerr << "argument closest_lanelet_ptr is null! Failed to find closest lanelet"
              << std::endl;
    return false;
  }

  if (lanelets.empty()) {
    return false;
  }

  lanelet::BasicPoint2d search_point(search_pose.position.x, search_pose.position.y);
  for (const auto & llt : lanelets) {
    if (lanelet::geometry::inside(llt, search_point)) {
      current_lanelets_ptr->push_back(llt);
    }
  }

  return !current_lanelets_ptr->empty();  // return found
}

std::vector<std::deque<lanelet::ConstLanelet>> getSucceedingLaneletSequencesRecursive(
  const routing::RoutingGraphPtr & graph, const lanelet::ConstLanelet & lanelet,
  const double length)
{
  std::vector<std::deque<lanelet::ConstLanelet>> succeeding_lanelet_sequences;

  const auto next_lanelets = graph->following(lanelet);
  const double lanelet_length = lanelet::geometry::length3d(lanelet);

  // end condition of the recursive function
  if (next_lanelets.empty() || lanelet_length >= length) {
    succeeding_lanelet_sequences.push_back({lanelet});
    return succeeding_lanelet_sequences;
  }

  for (const auto & next_lanelet : next_lanelets) {
    // get lanelet sequence after next_lanelet
    auto tmp_lanelet_sequences =
      getSucceedingLaneletSequencesRecursive(graph, next_lanelet, length - lanelet_length);
    for (auto & tmp_lanelet_sequence : tmp_lanelet_sequences) {
      tmp_lanelet_sequence.push_front(lanelet);
      succeeding_lanelet_sequences.push_back(tmp_lanelet_sequence);
    }
  }
  return succeeding_lanelet_sequences;
}

std::vector<std::deque<lanelet::ConstLanelet>> getPrecedingLaneletSequencesRecursive(
  const routing::RoutingGraphPtr & graph, const lanelet::ConstLanelet & lanelet,
  const double length, const lanelet::ConstLanelets & exclude_lanelets)
{
  std::vector<std::deque<lanelet::ConstLanelet>> preceding_lanelet_sequences;

  const auto prev_lanelets = graph->previous(lanelet);
  const double lanelet_length = lanelet::geometry::length3d(lanelet);

  // end condition of the recursive function
  if (prev_lanelets.empty() || lanelet_length >= length) {
    preceding_lanelet_sequences.push_back({lanelet});
    return preceding_lanelet_sequences;
  }

  for (const auto & prev_lanelet : prev_lanelets) {
    if (lanelet::utils::contains(exclude_lanelets, prev_lanelet)) {
      // if prev_lanelet is included in exclude_lanelets,
      // remove prev_lanelet from preceding_lanelet_sequences
      continue;
    }

    // get lanelet sequence after prev_lanelet
    auto tmp_lanelet_sequences = getPrecedingLaneletSequencesRecursive(
      graph, prev_lanelet, length - lanelet_length, exclude_lanelets);
    for (auto & tmp_lanelet_sequence : tmp_lanelet_sequences) {
      tmp_lanelet_sequence.push_back(lanelet);
      preceding_lanelet_sequences.push_back(tmp_lanelet_sequence);
    }
  }

  if (preceding_lanelet_sequences.empty()) {
    preceding_lanelet_sequences.push_back({lanelet});
  }
  return preceding_lanelet_sequences;
}

std::vector<lanelet::ConstLanelets> query::getSucceedingLaneletSequences(
  const routing::RoutingGraphPtr & graph, const lanelet::ConstLanelet & lanelet,
  const double length)
{
  std::vector<ConstLanelets> lanelet_sequences_vec;
  const auto next_lanelets = graph->following(lanelet);
  for (const auto & next_lanelet : next_lanelets) {
    const auto lanelet_sequences_deq =
      getSucceedingLaneletSequencesRecursive(graph, next_lanelet, length);
    for (const auto & lanelet_sequence : lanelet_sequences_deq) {
      lanelet_sequences_vec.emplace_back(lanelet_sequence.begin(), lanelet_sequence.end());
    }
  }
  return lanelet_sequences_vec;
}

std::vector<lanelet::ConstLanelets> query::getPrecedingLaneletSequences(
  const routing::RoutingGraphPtr & graph, const lanelet::ConstLanelet & lanelet,
  const double length, const lanelet::ConstLanelets & exclude_lanelets)
{
  std::vector<ConstLanelets> lanelet_sequences_vec;
  const auto prev_lanelets = graph->previous(lanelet);
  for (const auto & prev_lanelet : prev_lanelets) {
    if (lanelet::utils::contains(exclude_lanelets, prev_lanelet)) {
      // if prev_lanelet is included in exclude_lanelets,
      // remove prev_lanelet from preceding_lanelet_sequences
      continue;
    }
    // convert deque into vector
    const auto lanelet_sequences_deq =
      getPrecedingLaneletSequencesRecursive(graph, prev_lanelet, length, exclude_lanelets);
    for (const auto & lanelet_sequence : lanelet_sequences_deq) {
      lanelet_sequences_vec.emplace_back(lanelet_sequence.begin(), lanelet_sequence.end());
    }
  }
  return lanelet_sequences_vec;
}
}  // namespace lanelet::utils

// NOLINTEND(readability-identifier-naming)
