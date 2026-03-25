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

#ifndef BOOST_GEOMETRY_UTIL__GEOMETRIES__LINESTRING_HPP_
#define BOOST_GEOMETRY_UTIL__GEOMETRIES__LINESTRING_HPP_

#include <boost/geometry/geometries/register/linestring.hpp>
#include <boost_geometry_util/geometries/point.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <vector>

namespace boost_geometry_util
{
namespace linestring
{
template <typename T>
std::vector<T> construct(const T & origin, const geometry_msgs::msg::Vector3 & vec)
{
  std::vector<T> linestring;
  linestring.emplace_back(origin);
}

template <typename T>
std::vector<geometry_msgs::msg::Point2D> construct(const std::vector<T> & points)
{
  std::vector<geometry_msgs::msg::Point2D> result;
  std::transform(points.begin(), points.end(), std::back_inserter(result), [](const T & p) {
    return geometry_msgs::msg::Point2D(p.x, p.y);
  });
  return result;
}
}  // namespace linestring
}  // namespace boost_geometry_util

BOOST_GEOMETRY_REGISTER_LINESTRING(std::vector<geometry_msgs::msg::Point2D>)
BOOST_GEOMETRY_REGISTER_LINESTRING(std::vector<geometry_msgs::msg::Point>)
BOOST_GEOMETRY_REGISTER_LINESTRING(std::vector<geometry_msgs::msg::Point32>)

#endif  // BOOST_GEOMETRY_UTIL__GEOMETRIES__LINESTRING_HPP_
