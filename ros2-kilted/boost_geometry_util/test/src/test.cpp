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

// headers in Google Test
#include <gtest/gtest.h>

#include <boost/geometry.hpp>
#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost_geometry_util/geometries/geometries.hpp>

namespace bg = boost::geometry;

#define EXPECT_POINT2D_EQ(POINT, X, Y)                 \
  EXPECT_DOUBLE_EQ(boost::geometry::get<0>(POINT), X); \
  EXPECT_DOUBLE_EQ(boost::geometry::get<1>(POINT), Y);

#define EXPECT_POINT3D_EQ(POINT, X, Y, Z)              \
  EXPECT_DOUBLE_EQ(boost::geometry::get<0>(POINT), X); \
  EXPECT_DOUBLE_EQ(boost::geometry::get<1>(POINT), Y); \
  EXPECT_DOUBLE_EQ(boost::geometry::get<2>(POINT), Z);

#define EXPECT_BOX2D_EQ(BOX, MIN_CORNER_X, MIN_CORNER_Y, MAX_CORNER_X, MAX_CORNER_Y) \
  EXPECT_POINT2D_EQ(BOX.min_corner, MIN_CORNER_X, MIN_CORNER_Y);                     \
  EXPECT_POINT2D_EQ(BOX.max_corner, MAX_CORNER_X, MAX_CORNER_Y);

#define TEST_POINT_TYPE_FOREACH(IDENTIFIER, ...)      \
  IDENTIFIER<geometry_msgs::msg::Point>(__VA_ARGS__); \
  IDENTIFIER<geometry_msgs::msg::Point32>(__VA_ARGS__);

void testPoint2D(double x, double y)
{
  EXPECT_POINT2D_EQ(boost_geometry_util::point_2d::construct(x, y), x, y);
}

TEST(TestSuite, Point2D) { testPoint2D(1, 2); }

template <typename T>
void testPoint3D(double x, double y, double z)
{
  EXPECT_POINT3D_EQ(boost_geometry_util::point_3d::construct<T>(x, y, z), x, y, z);
}

template <typename T>
void testPoint3DApply(double x, double y, double z, double vec_x, double vec_y, double vec_z)
{
  EXPECT_POINT3D_EQ(
    boost_geometry_util::point_3d::construct<T>(x, y, z) +
      boost_geometry_util::vector_3d::construct(vec_x, vec_y, vec_z),
    x + vec_x, y + vec_y, z + vec_z);
}

template <typename T>
void testPoint3DSubtract(double x, double y, double z, double vec_x, double vec_y, double vec_z)
{
  EXPECT_POINT3D_EQ(
    boost_geometry_util::point_3d::construct<T>(x, y, z) -
      boost_geometry_util::vector_3d::construct(vec_x, vec_y, vec_z),
    x - vec_x, y - vec_y, z - vec_z);
}

TEST(TestSuite, Point3D)
{
  TEST_POINT_TYPE_FOREACH(testPoint3D, 1.0, 2.0, 3.0);
  TEST_POINT_TYPE_FOREACH(testPoint3DApply, 1.0, 2.0, 3.0, 4, 5, 6);
  TEST_POINT_TYPE_FOREACH(testPoint3DSubtract, 1.0, 2.0, 3.0, 4, 5, 6);
}

template <typename T>
void testBox(double x_min, double y_min, double z_min, double x_max, double y_max, double z_max)
{
  EXPECT_NO_THROW(bg::model::box<T> box(
    boost_geometry_util::point_3d::construct<T>(x_min, y_min, z_min),
    boost_geometry_util::point_3d::construct<T>(x_max, y_max, z_max)));
}

TEST(TestSuite, Box) { TEST_POINT_TYPE_FOREACH(testBox, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0); }

template <typename T>
void testLineString(
  double x_begin, double y_begin, double z_begin, double x_end, double y_end, double z_end)
{
  const bg::model::linestring<T> line0 = {
    boost_geometry_util::point_3d::construct<T>(x_begin, y_begin, z_begin),
    boost_geometry_util::point_3d::construct<T>(x_end, y_end, z_end)};
  EXPECT_NO_THROW(bg::correct(line0));
  std::vector<geometry_msgs::msg::Point2D> line_2d =
    boost_geometry_util::linestring::construct(line0);
}

TEST(TestSuite, LineString) { TEST_POINT_TYPE_FOREACH(testLineString, 0, 2, 0, 2, 2, 0); }

template <typename T>
void testDisjoint(
  double x0_min, double y0_min, double z0_min, double x0_max, double y0_max, double z0_max,
  double x1_min, double y1_min, double z1_min, double x1_max, double y1_max, double z1_max,
  bool disjoint)
{
  bg::model::box<T> box0(
    boost_geometry_util::point_3d::construct<T>(x0_min, y0_min, z0_min),
    boost_geometry_util::point_3d::construct<T>(x0_max, y0_max, z0_max));
  bg::model::box<T> box1(
    boost_geometry_util::point_3d::construct<T>(x1_min, y1_min, z1_min),
    boost_geometry_util::point_3d::construct<T>(x1_max, y1_max, z1_max));
  EXPECT_EQ(bg::disjoint(box0, box1), disjoint);
}

template <typename T>
void testDisjoint(
  double x_min, double y_min, double z_min, double x_max, double y_max, double z_max, /*    */
  double x, double y, double z, bool disjoint)
{
  bg::model::box<T> box(
    boost_geometry_util::point_3d::construct<T>(x_min, y_min, z_min),
    boost_geometry_util::point_3d::construct<T>(x_max, y_max, z_max));
  const auto p = boost_geometry_util::point_3d::construct<T>(x, y, z);
  EXPECT_EQ(bg::disjoint(box, p), disjoint);
}

TEST(TestSuite, Disjoint)
{
  TEST_POINT_TYPE_FOREACH(testDisjoint, 0, 0, 0, 3, 3, 0, 4, 4, 0, 7, 7, 0, true);
  TEST_POINT_TYPE_FOREACH(testDisjoint, 0, 0, 0, 3, 3, 0, 2, 2, 0, 5, 5, 0, false);
  TEST_POINT_TYPE_FOREACH(testDisjoint, 0, 0, 0, 3, 3, 0, 5, 5, 0, true);
  TEST_POINT_TYPE_FOREACH(testDisjoint, 0, 0, 0, 3, 3, 0, 2, 2, 0, false);
}

void testArea(double x_min, double y_min, double x_max, double y_max)
{
  bg::model::box box(
    boost_geometry_util::point_2d::construct(x_min, y_min),
    boost_geometry_util::point_2d::construct(x_max, y_max));
  EXPECT_DOUBLE_EQ(bg::area(box), std::abs(x_max - x_min) * std::abs(y_max - y_min));
}

TEST(TestSuite, Area)
{
  testArea(0, 0, 3, 3);
  testArea(1, 1, 3, 3);
}

/*
TEST(TestSuite, ConvexHull)
{
  std::vector<boost_geometry_util::Point2D> linestring = {
    boost_geometry_util::Point2D(2.0, 1.3), boost_geometry_util::Point2D(2.4, 1.7),
    boost_geometry_util::Point2D(3.6, 1.2), boost_geometry_util::Point2D(4.6, 1.6),
    boost_geometry_util::Point2D(4.1, 3.0), boost_geometry_util::Point2D(5.3, 2.8),
    boost_geometry_util::Point2D(5.4, 1.2), boost_geometry_util::Point2D(4.9, 0.8),
    boost_geometry_util::Point2D(3.6, 0.7), boost_geometry_util::Point2D(2.0, 1.3)};
  bg::model::polygon<boost_geometry_util::Point2D> poly = boost_geometry_util::toPolygon();
  bg::model::polygon<boost_geometry_util::Point2D> hull;
  bg::convex_hull(poly, hull);
}
*/

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
