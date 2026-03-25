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
// Authors: Kenji Miyake, Ryohsuke Mitsudome

// NOLINTBEGIN(readability-identifier-naming)

// NOLINTEND(readability-identifier-naming)

#include "deprecated.hpp"

#include <tf2/utils.hpp>

#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include <lanelet2_core/geometry/Lanelet.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

using lanelet::utils::to2D;

namespace deprecated
{
namespace detail
{
/// @brief copy the z values between 2 containers based on the 2D arc lengths
/// @tparam T1 a container of 3D points
/// @tparam T2 a container of 3D points
/// @param from points from which the z values will be copied
/// @param to points to which the z values will be copied
template <typename T1, typename T2>
void copyZ(const T1 & from, T2 & to)
{
  if (from.empty() || to.empty()) return;
  to.front().z() = from.front().z();
  if (from.size() < 2 || to.size() < 2) return;
  to.back().z() = from.back().z();
  auto i_from = 1lu;
  auto s_from = lanelet::geometry::distance2d(from[0], from[1]);
  auto s_to = 0.0;
  auto s_from_prev = 0.0;
  for (auto i_to = 1lu; i_to + 1 < to.size(); ++i_to) {
    s_to += lanelet::geometry::distance2d(to[i_to - 1], to[i_to]);
    for (; s_from < s_to && i_from + 1 < from.size(); ++i_from) {
      s_from_prev = s_from;
      s_from += lanelet::geometry::distance2d(from[i_from], from[i_from + 1]);
    }
    const auto ratio = (s_to - s_from_prev) / (s_from - s_from_prev);
    to[i_to].z() = from[i_from - 1].z() + ratio * (from[i_from].z() - from[i_from - 1].z());
  }
}

std::vector<double> calculateSegmentDistances(const lanelet::ConstLineString3d & line_string)
{
  std::vector<double> segment_distances;
  segment_distances.reserve(line_string.size() - 1);

  for (size_t i = 1; i < line_string.size(); ++i) {
    const auto distance = lanelet::geometry::distance(line_string[i], line_string[i - 1]);
    segment_distances.push_back(distance);
  }

  return segment_distances;
}

std::vector<double> calculateAccumulatedLengths(const lanelet::ConstLineString3d & line_string)
{
  const auto segment_distances = calculateSegmentDistances(line_string);

  std::vector<double> accumulated_lengths{0};
  accumulated_lengths.reserve(segment_distances.size() + 1);
  std::partial_sum(
    std::begin(segment_distances), std::end(segment_distances),
    std::back_inserter(accumulated_lengths));

  return accumulated_lengths;
}

std::pair<size_t, size_t> findNearestIndexPair(
  const std::vector<double> & accumulated_lengths, const double target_length)
{
  // List size
  const auto N = accumulated_lengths.size();

  // Front
  if (target_length < accumulated_lengths.at(1)) {
    return std::make_pair(0, 1);
  }

  // Back
  if (target_length > accumulated_lengths.at(N - 2)) {
    return std::make_pair(N - 2, N - 1);
  }

  // Middle
  for (std::size_t i = 1; i < N; ++i) {
    if (
      accumulated_lengths.at(i - 1) <= target_length &&
      target_length <= accumulated_lengths.at(i)) {
      return std::make_pair(i - 1, i);
    }
  }

  // Throw an exception because this never happens
  throw std::runtime_error("No nearest point found.");
}

std::vector<lanelet::BasicPoint3d> resamplePoints(
  const lanelet::ConstLineString3d & line_string, const int num_segments)
{
  // Calculate length
  const auto line_length = static_cast<double>(lanelet::geometry::length(line_string));

  // Calculate accumulated lengths
  const auto accumulated_lengths = calculateAccumulatedLengths(line_string);
  if (accumulated_lengths.size() < 2) return {};

  // Create each segment
  std::vector<lanelet::BasicPoint3d> resampled_points;
  for (auto i = 0; i <= num_segments; ++i) {
    // Find two nearest points
    const auto target_length = (static_cast<double>(i) / num_segments) * line_length;
    const auto index_pair = findNearestIndexPair(accumulated_lengths, target_length);

    // Apply linear interpolation
    const lanelet::BasicPoint3d back_point = line_string[index_pair.first];
    const lanelet::BasicPoint3d front_point = line_string[index_pair.second];
    const auto direction_vector = (front_point - back_point);

    const auto back_length = accumulated_lengths.at(index_pair.first);
    const auto front_length = accumulated_lengths.at(index_pair.second);
    const auto segment_length = front_length - back_length;
    const auto target_point =
      back_point + (direction_vector * (target_length - back_length) / segment_length);

    // Add to list
    resampled_points.emplace_back(target_point);
  }

  return resampled_points;
}
}  // namespace detail

lanelet::ConstLineString3d getClosestSegment(
  const lanelet::BasicPoint2d & search_pt, const lanelet::ConstLineString3d & linestring)
{
  if (linestring.size() < 2) {
    return lanelet::LineString3d();
  }

  lanelet::ConstLineString3d closest_segment;
  double min_distance = std::numeric_limits<double>::max();

  for (size_t i = 1; i < linestring.size(); i++) {
    lanelet::BasicPoint3d prev_basic_pt = linestring[i - 1].basicPoint();
    lanelet::BasicPoint3d current_basic_pt = linestring[i].basicPoint();

    lanelet::Point3d prev_pt(
      lanelet::InvalId, prev_basic_pt.x(), prev_basic_pt.y(), prev_basic_pt.z());
    lanelet::Point3d current_pt(
      lanelet::InvalId, current_basic_pt.x(), current_basic_pt.y(), current_basic_pt.z());

    lanelet::LineString3d current_segment(lanelet::InvalId, {prev_pt, current_pt});
    double distance = lanelet::geometry::distance2d(
      lanelet::utils::to2D(current_segment).basicLineString(), search_pt);
    if (distance < min_distance) {
      closest_segment = current_segment;
      min_distance = distance;
    }
  }
  return closest_segment;
}

double getLaneletAngle(
  const lanelet::ConstLanelet & lanelet, const geometry_msgs::msg::Point & search_point)
{
  lanelet::BasicPoint2d llt_search_point(search_point.x, search_point.y);
  lanelet::ConstLineString3d segment = getClosestSegment(llt_search_point, lanelet.centerline());
  return std::atan2(
    segment.back().y() - segment.front().y(), segment.back().x() - segment.front().x());
}

lanelet::ConstLanelets getLaneletsWithinRange(
  const lanelet::ConstLanelets & lanelets, const lanelet::BasicPoint2d & search_point,
  const double range)
{
  lanelet::ConstLanelets near_lanelets;
  for (const auto & ll : lanelets) {
    lanelet::BasicPolygon2d poly = ll.polygon2d().basicPolygon();
    double distance = lanelet::geometry::distance(poly, search_point);
    if (distance <= range) {
      near_lanelets.push_back(ll);
    }
  }
  return near_lanelets;
}

lanelet::ConstLanelets getLaneletsWithinRange(
  const lanelet::ConstLanelets & lanelets, const geometry_msgs::msg::Point & search_point,
  const double range)
{
  return getLaneletsWithinRange(
    lanelets, lanelet::BasicPoint2d(search_point.x, search_point.y), range);
}

lanelet::ConstLanelets getAllNeighborsRight(
  const lanelet::routing::RoutingGraphPtr & graph, const lanelet::ConstLanelet & lanelet)
{
  lanelet::ConstLanelets lanelets;
  auto right_lane =
    (!!graph->right(lanelet)) ? graph->right(lanelet) : graph->adjacentRight(lanelet);
  while (!!right_lane) {
    lanelets.push_back(right_lane.get());
    right_lane = (!!graph->right(right_lane.get())) ? graph->right(right_lane.get())
                                                    : graph->adjacentRight(right_lane.get());
  }
  return lanelets;
}

lanelet::ConstLanelets getAllNeighborsLeft(
  const lanelet::routing::RoutingGraphPtr & graph, const lanelet::ConstLanelet & lanelet)
{
  lanelet::ConstLanelets lanelets;
  auto left_lane = (!!graph->left(lanelet)) ? graph->left(lanelet) : graph->adjacentLeft(lanelet);
  while (!!left_lane) {
    lanelets.push_back(left_lane.get());
    left_lane = (!!graph->left(left_lane.get())) ? graph->left(left_lane.get())
                                                 : graph->adjacentLeft(left_lane.get());
  }
  return lanelets;
}

lanelet::ConstLanelets getAllNeighbors(
  const lanelet::routing::RoutingGraphPtr & graph, const lanelet::ConstLanelet & lanelet)
{
  lanelet::ConstLanelets lanelets;

  lanelet::ConstLanelets left_lanelets = getAllNeighborsLeft(graph, lanelet);
  lanelet::ConstLanelets right_lanelets = getAllNeighborsRight(graph, lanelet);

  std::reverse(left_lanelets.begin(), left_lanelets.end());
  lanelets.insert(lanelets.end(), left_lanelets.begin(), left_lanelets.end());
  lanelets.push_back(lanelet);
  lanelets.insert(lanelets.end(), right_lanelets.begin(), right_lanelets.end());

  return lanelets;
}

lanelet::ConstLanelets getLaneChangeableNeighbors(
  const lanelet::routing::RoutingGraphPtr & graph, const lanelet::ConstLanelet & lanelet)
{
  return graph->besides(lanelet);
}

bool getClosestLanelet(
  const lanelet::ConstLanelets & lanelets, const geometry_msgs::msg::Pose & search_pose,
  lanelet::ConstLanelet * closest_lanelet_ptr)
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
      lanelet::ConstLineString3d segment = getClosestSegment(search_point, llt.centerline());
      double angle_diff = M_PI;
      if (!segment.empty()) {
        double segment_angle = std::atan2(
          segment.back().y() - segment.front().y(), segment.back().x() - segment.front().x());
        angle_diff = std::abs(normalize_radian(segment_angle - pose_yaw));
      }
      if (angle_diff < min_angle) {
        min_angle = angle_diff;
        *closest_lanelet_ptr = llt;
      }
    }
  }

  return found;
}

double getLateralDistanceToCenterline(
  const lanelet::ConstLanelet & lanelet, const geometry_msgs::msg::Pose & pose)
{
  const auto & centerline_2d = lanelet::utils::to2D(lanelet.centerline());
  const auto lanelet_point = toLaneletPoint(pose.position);
  return lanelet::geometry::signedDistance(
    centerline_2d, lanelet::utils::to2D(lanelet_point).basicPoint());
}

lanelet::ConstLanelet combineLaneletsShape(const lanelet::ConstLanelets & lanelets)
{
  const auto addUniquePoint = [](lanelet::Points3d & points, const lanelet::Point3d & new_point) {
    constexpr double distance_threshold = 0.01;
    const auto is_duplicate = std::any_of(
      points.cbegin(), points.cend(),
      [&new_point, distance_threshold](const auto & existing_point) {
        return boost::geometry::distance(existing_point.basicPoint(), new_point.basicPoint()) <=
               distance_threshold;
      });
    if (!is_duplicate) points.emplace_back(new_point);
  };

  const auto addUniquePoints = [&addUniquePoint](
                                 lanelet::Points3d & output, const auto & input_points) {
    std::for_each(
      input_points.begin(), input_points.end(), [&output, &addUniquePoint](const auto & pt) {
        addUniquePoint(output, lanelet::Point3d(pt));
      });
  };

  lanelet::Points3d lefts, rights, centers;
  for (const auto & llt : lanelets) {
    addUniquePoints(lefts, llt.leftBound());
    addUniquePoints(rights, llt.rightBound());
    addUniquePoints(centers, llt.centerline());
  }
  const auto left_bound = lanelet::LineString3d(lanelet::InvalId, lefts);
  const auto right_bound = lanelet::LineString3d(lanelet::InvalId, rights);
  const auto center_line = lanelet::LineString3d(lanelet::InvalId, centers);
  auto combined_lanelet = lanelet::Lanelet(lanelet::InvalId, left_bound, right_bound);
  combined_lanelet.setCenterline(center_line);
  return combined_lanelet;
}

lanelet::ConstLanelet getExpandedLanelet(
  const lanelet::ConstLanelet & lanelet_obj, const double left_offset, const double right_offset)
{
  using lanelet::geometry::offsetNoThrow;
  using lanelet::geometry::internal::checkForInversion;

  const auto & orig_left_bound_2d = lanelet_obj.leftBound2d().basicLineString();
  const auto & orig_right_bound_2d = lanelet_obj.rightBound2d().basicLineString();

  // Note: The lanelet::geometry::offset throws exception when the undesired inversion is found.
  // Use offsetNoThrow until the logic is updated to handle the inversion.
  // TODO(Horibe) update
  auto expanded_left_bound_2d = offsetNoThrow(orig_left_bound_2d, left_offset);
  auto expanded_right_bound_2d = offsetNoThrow(orig_right_bound_2d, right_offset);

  rclcpp::Clock clock{RCL_ROS_TIME};
  try {
    checkForInversion(orig_left_bound_2d, expanded_left_bound_2d, left_offset);
    checkForInversion(orig_right_bound_2d, expanded_right_bound_2d, right_offset);
  } catch (const lanelet::GeometryError & e) {
    RCLCPP_ERROR_THROTTLE(
      rclcpp::get_logger("autoware_lanelet2_extension"), clock, 1000,
      "Fail to expand lanelet. output may be undesired. Lanelet points interval in map data could "
      "be too narrow.");
  }

  // Note: modify front and back points so that the successive lanelets will not have any
  // longitudinal space between them.
  {  // front
    const double diff_x = orig_right_bound_2d.front().x() - orig_left_bound_2d.front().x();
    const double diff_y = orig_right_bound_2d.front().y() - orig_left_bound_2d.front().y();
    const double theta = std::atan2(diff_y, diff_x);
    expanded_right_bound_2d.front().x() =
      orig_right_bound_2d.front().x() - right_offset * std::cos(theta);
    expanded_right_bound_2d.front().y() =
      orig_right_bound_2d.front().y() - right_offset * std::sin(theta);
    expanded_left_bound_2d.front().x() =
      orig_left_bound_2d.front().x() - left_offset * std::cos(theta);
    expanded_left_bound_2d.front().y() =
      orig_left_bound_2d.front().y() - left_offset * std::sin(theta);
  }
  {  // back
    const double diff_x = orig_right_bound_2d.back().x() - orig_left_bound_2d.back().x();
    const double diff_y = orig_right_bound_2d.back().y() - orig_left_bound_2d.back().y();
    const double theta = std::atan2(diff_y, diff_x);
    expanded_right_bound_2d.back().x() =
      orig_right_bound_2d.back().x() - right_offset * std::cos(theta);
    expanded_right_bound_2d.back().y() =
      orig_right_bound_2d.back().y() - right_offset * std::sin(theta);
    expanded_left_bound_2d.back().x() =
      orig_left_bound_2d.back().x() - left_offset * std::cos(theta);
    expanded_left_bound_2d.back().y() =
      orig_left_bound_2d.back().y() - left_offset * std::sin(theta);
  }

  const auto toPoints3d = [](const lanelet::BasicLineString2d & ls2d) {
    lanelet::Points3d output;
    for (const auto & pt : ls2d) {
      output.push_back(lanelet::Point3d(lanelet::InvalId, pt.x(), pt.y(), 0.0));
    }
    return output;
  };

  lanelet::Points3d ex_lefts = toPoints3d(expanded_left_bound_2d);
  lanelet::Points3d ex_rights = toPoints3d(expanded_right_bound_2d);
  detail::copyZ(lanelet_obj.leftBound3d(), ex_lefts);
  detail::copyZ(lanelet_obj.rightBound3d(), ex_rights);

  const auto & extended_left_bound_3d = lanelet::LineString3d(lanelet::InvalId, ex_lefts);
  const auto & expanded_right_bound_3d = lanelet::LineString3d(lanelet::InvalId, ex_rights);
  const auto & lanelet = lanelet::Lanelet(
    lanelet_obj.id(), extended_left_bound_3d, expanded_right_bound_3d, lanelet_obj.attributes());

  return lanelet;
}

void toGeomMsgPt(const geometry_msgs::msg::Point32 & src, geometry_msgs::msg::Point * dst)
{
  if (dst == nullptr) {
    std::cerr << __FUNCTION__ << "pointer is null!";
    return;
  }
  dst->x = src.x;
  dst->y = src.y;
  dst->z = src.z;
}
void toGeomMsgPt(const Eigen::Vector3d & src, geometry_msgs::msg::Point * dst)
{
  if (dst == nullptr) {
    std::cerr << __FUNCTION__ << "pointer is null!";
    return;
  }
  dst->x = src.x();
  dst->y = src.y();
  dst->z = src.z();
}
void toGeomMsgPt(const lanelet::ConstPoint3d & src, geometry_msgs::msg::Point * dst)
{
  if (dst == nullptr) {
    std::cerr << __FUNCTION__ << "pointer is null!";
    return;
  }
  dst->x = src.x();
  dst->y = src.y();
  dst->z = src.z();
}
void toGeomMsgPt(const lanelet::ConstPoint2d & src, geometry_msgs::msg::Point * dst)
{
  if (dst == nullptr) {
    std::cerr << __FUNCTION__ << "pointer is null!" << std::endl;
    return;
  }
  dst->x = src.x();
  dst->y = src.y();
  dst->z = 0;
}

void toGeomMsgPt32(const Eigen::Vector3d & src, geometry_msgs::msg::Point32 * dst)
{
  if (dst == nullptr) {
    std::cerr << __FUNCTION__ << "pointer is null!" << std::endl;
    return;
  }
  dst->x = static_cast<float>(src.x());
  dst->y = static_cast<float>(src.y());
  dst->z = static_cast<float>(src.z());
}

void toLaneletPoint(const geometry_msgs::msg::Point & src, lanelet::ConstPoint3d * dst)
{
  *dst = lanelet::Point3d(lanelet::InvalId, src.x, src.y, src.z);
}

lanelet::ConstPoint3d toLaneletPoint(const geometry_msgs::msg::Point & src)
{
  lanelet::ConstPoint3d dst;
  toLaneletPoint(src, &dst);
  return dst;
}

lanelet::LineString3d generateFineCenterline(
  const lanelet::ConstLanelet & lanelet_obj, const double resolution)
{
  // Get length of longer border
  const double left_length =
    static_cast<double>(lanelet::geometry::length(lanelet_obj.leftBound()));
  const double right_length =
    static_cast<double>(lanelet::geometry::length(lanelet_obj.rightBound()));
  const double longer_distance = (left_length > right_length) ? left_length : right_length;
  const int num_segments = std::max(static_cast<int>(ceil(longer_distance / resolution)), 1);

  // Resample points
  const auto left_points = detail::resamplePoints(lanelet_obj.leftBound(), num_segments);
  const auto right_points = detail::resamplePoints(lanelet_obj.rightBound(), num_segments);

  // Create centerline
  lanelet::LineString3d centerline(lanelet::utils::getId());
  for (int i = 0; i < num_segments + 1; i++) {
    // Add ID for the average point of left and right
    const auto center_basic_point = (right_points.at(i) + left_points.at(i)) / 2;
    const lanelet::Point3d center_point(
      lanelet::utils::getId(), center_basic_point.x(), center_basic_point.y(),
      center_basic_point.z());
    centerline.push_back(center_point);
  }
  return centerline;
}
}  // namespace deprecated
