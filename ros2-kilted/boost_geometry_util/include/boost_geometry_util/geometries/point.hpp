// Copyright (c) 2022 OUXT Polaris
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

#ifndef BOOST_GEOMETRY_UTIL__GEOMETRIES__POINT_HPP_
#define BOOST_GEOMETRY_UTIL__GEOMETRIES__POINT_HPP_

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <geometry_msgs/msg/point.hpp>
#include <geometry_msgs/msg/point32.hpp>
#include <geometry_msgs/msg/vector3.hpp>

namespace geometry_msgs
{
namespace msg
{
struct Point2D
{
  double x;
  double y;
  Point2D(double x, double y);
  Point2D() = default;
};

}  // namespace msg
}  // namespace geometry_msgs

namespace boost_geometry_util
{
namespace point_2d
{
geometry_msgs::msg::Point2D construct(double x, double y);
template <typename T>
geometry_msgs::msg::Point2D construct(const T point)
{
  return construct(static_cast<double>(point.x), static_cast<double>(point.y));
}
}  // namespace point_2d

namespace point_3d
{
template <typename T>
T construct(double x, double y, double z)
{
  T point;
  point.x = x;
  point.y = y;
  point.z = z;
  return point;
}
}  // namespace point_3d

namespace vector_2d
{
geometry_msgs::msg::Vector3 construct(double x, double y);
}  // namespace vector_2d

namespace vector_3d
{
geometry_msgs::msg::Vector3 construct(double x, double y, double z);
}  // namespace vector_3d
}  // namespace boost_geometry_util

template <typename T1, typename T2>
geometry_msgs::msg::Point operator+(const T1 & v1, const T2 & v2)
{
  return boost_geometry_util::point_3d::construct<geometry_msgs::msg::Point>(
    v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

template <typename T1, typename T2>
geometry_msgs::msg::Point operator-(const T1 & v1, const T2 & v2)
{
  return boost_geometry_util::point_3d::construct<geometry_msgs::msg::Point>(
    v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

BOOST_GEOMETRY_REGISTER_POINT_2D(geometry_msgs::msg::Point2D, double, cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_POINT_3D(geometry_msgs::msg::Point, double, cs::cartesian, x, y, z)
BOOST_GEOMETRY_REGISTER_POINT_3D(geometry_msgs::msg::Point32, double, cs::cartesian, x, y, z)

#endif  // BOOST_GEOMETRY_UTIL__GEOMETRIES__POINT_HPP_
