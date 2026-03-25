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

#include <boost_geometry_util/geometries/point.hpp>

namespace geometry_msgs
{
namespace msg
{
Point2D::Point2D(double x, double y) : x(x), y(y) {}
}  // namespace msg
}  // namespace geometry_msgs

namespace boost_geometry_util
{
namespace point_2d
{
geometry_msgs::msg::Point2D construct(double x, double y)
{
  return geometry_msgs::msg::Point2D(x, y);
}
}  // namespace point_2d
namespace vector_2d
{
geometry_msgs::msg::Vector3 construct(double x, double y)
{
  geometry_msgs::msg::Vector3 vec;
  vec.x = x;
  vec.y = y;
  vec.z = 0;
  return vec;
}
}  // namespace vector_2d

namespace vector_3d
{
geometry_msgs::msg::Vector3 construct(double x, double y, double z)
{
  geometry_msgs::msg::Vector3 vec;
  vec.x = x;
  vec.y = y;
  vec.z = z;
  return vec;
}

}  // namespace vector_3d
}  // namespace boost_geometry_util
