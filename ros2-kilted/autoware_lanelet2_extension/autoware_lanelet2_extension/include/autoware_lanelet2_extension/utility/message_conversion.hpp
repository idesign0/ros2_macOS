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

#ifndef AUTOWARE_LANELET2_EXTENSION__UTILITY__MESSAGE_CONVERSION_HPP_
#define AUTOWARE_LANELET2_EXTENSION__UTILITY__MESSAGE_CONVERSION_HPP_

// NOLINTBEGIN(readability-identifier-naming)

#include <autoware_map_msgs/msg/lanelet_map_bin.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <geometry_msgs/msg/point32.hpp>
#include <geometry_msgs/msg/polygon.hpp>

#include <lanelet2_core/Forward.h>
#include <lanelet2_routing/Forward.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

namespace lanelet::utils::conversion
{
/**
 * [toBinMsg converts lanelet2 map to ROS message. Similar implementation to
 * lanelet::io_handlers::BinHandler::write()]
 * @param map [lanelet map data]
 * @param msg [converted ROS message. Only "data" field is filled]
 */
[[deprecated("please use autoware::lanelet2_utils::to_autoware_map_msgs instead")]] void toBinMsg(
  const lanelet::LaneletMapPtr & map, autoware_map_msgs::msg::LaneletMapBin * msg);

/**
 * [fromBinMsg converts ROS message into lanelet2 data. Similar implementation
 * to lanelet::io_handlers::BinHandler::parse()]
 * @param msg [ROS message for lanelet map]
 * @param map [Converted lanelet2 data]
 */
[[deprecated("please use autoware::lanelet2_utils::from_autoware_map_msgs instead")]] void
fromBinMsg(const autoware_map_msgs::msg::LaneletMapBin & msg, lanelet::LaneletMapPtr map);
[[deprecated(
  "please use autoware::lanelet2_utils::from_autoware_map_msgs and "
  "autoware::lanelet2_utils::instantiate_routing_graph_and_traffic_rules instead")]] void
fromBinMsg(
  const autoware_map_msgs::msg::LaneletMapBin & msg, lanelet::LaneletMapPtr map,
  lanelet::traffic_rules::TrafficRulesPtr * traffic_rules,
  lanelet::routing::RoutingGraphPtr * routing_graph);

/**
 * [toGeomMsgPt converts various point types to geometry_msgs point]
 * @param src [input point(geometry_msgs::msg::Point3,
 * Eigen::VEctor3d=lanelet::BasicPoint3d, lanelet::Point3d, lanelet::Point2d) ]
 * @param dst [converted geometry_msgs point]
 */
[[deprecated("please use geometry_msgs::build instead")]] void toGeomMsgPt(
  const geometry_msgs::msg::Point32 & src, geometry_msgs::msg::Point * dst);
[[deprecated("please use geometry_msgs::build instead")]] void toGeomMsgPt(
  const Eigen::Vector3d & src, geometry_msgs::msg::Point * dst);
[[deprecated("please use autoware::lanelet2_utils::to_ros")]] void toGeomMsgPt(
  const lanelet::ConstPoint3d & src, geometry_msgs::msg::Point * dst);
[[deprecated("please use autoware::lanelet2_utils::to_ros")]] void toGeomMsgPt(
  const lanelet::ConstPoint2d & src, geometry_msgs::msg::Point * dst);

/**
 * [toGeomMsgPt converts various point types to geometry_msgs point]
 * @param src [input point(geometry_msgs::msg::Point3,
 * Eigen::VEctor3d=lanelet::BasicPoint3d, lanelet::Point3d, lanelet::Point2d) ]
 * @return    [converted geometry_msgs point]
 */
[[deprecated("please use geometry_msgs::build instead")]] geometry_msgs::msg::Point toGeomMsgPt(
  const geometry_msgs::msg::Point32 & src);
[[deprecated("please use geometry_msgs::build instead")]] geometry_msgs::msg::Point toGeomMsgPt(
  const Eigen::Vector3d & src);
[[deprecated("please use autoware::lanelet2_utils::to_ros")]] geometry_msgs::msg::Point toGeomMsgPt(
  const lanelet::ConstPoint3d & src);
[[deprecated("please use autoware::lanelet2_utils::to_ros")]] geometry_msgs::msg::Point toGeomMsgPt(
  const lanelet::ConstPoint2d & src);

[[deprecated("please use autoware::lanelet2_utils::from_ros")]] lanelet::ConstPoint3d
toLaneletPoint(const geometry_msgs::msg::Point & src);
[[deprecated("please use autoware::lanelet2_utils::from_ros")]] void toLaneletPoint(
  const geometry_msgs::msg::Point & src, lanelet::ConstPoint3d * dst);

/**
 * [toGeomMsgPoly converts lanelet polygon to geometry_msgs polygon]
 * @param ll_poly   [input polygon]
 * @param geom_poly [converted geometry_msgs point]
 */
[[deprecated(
  "toGeomMsgPoly(const lanelet::ConstPolygon3d, geometry_msgs::msg::Polygon *) will be "
  "deprecated")]] void
toGeomMsgPoly(const lanelet::ConstPolygon3d & ll_poly, geometry_msgs::msg::Polygon * geom_poly);

/**
 * [toGeomMsgPt32 converts Eigen::Vector3d(lanelet:BasicPoint3d to
 * geometry_msgs::msg::Point32)]
 * @param src [input point]
 * @param dst [converted point]
 */
[[deprecated("please use geometry_msgs::build instead")]] void toGeomMsgPt32(
  const Eigen::Vector3d & src, geometry_msgs::msg::Point32 * dst);

}  // namespace lanelet::utils::conversion

// NOLINTEND(readability-identifier-naming)

#endif  // AUTOWARE_LANELET2_EXTENSION__UTILITY__MESSAGE_CONVERSION_HPP_
