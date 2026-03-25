// Copyright 2015-2019 Autoware Foundation
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

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-avoid-goto)

#include "autoware_lanelet2_extension/utility/utilities.hpp"

#include <gtest/gtest.h>
#include <lanelet2_core/primitives/Point.h>
#include <lanelet2_routing/RoutingGraphContainer.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>

#include <map>
#include <unordered_map>

using lanelet::Lanelet;
using lanelet::LineString3d;
using lanelet::Point3d;
using lanelet::utils::getId;

class TestSuite : public ::testing::Test  // NOLINT for gtest
{
public:
  TestSuite() : sample_map_ptr(new lanelet::LaneletMap())
  {
    // create sample lanelets
    Point3d p1;
    Point3d p2;
    Point3d p3;
    Point3d p4;
    Point3d p5;
    Point3d p6;
    Point3d p7;
    Point3d p8;
    Point3d p9;
    Point3d p10;

    p1 = Point3d(getId(), 0., 0., 0.);
    p2 = Point3d(getId(), 0., 1., 0.);
    p3 = Point3d(getId(), 1., 0., 0.);
    p4 = Point3d(getId(), 1., 1., 0.);

    LineString3d ls_left(getId(), {p1, p2});
    LineString3d ls_right(getId(), {p3, p4});

    p5 = Point3d(getId(), 0., 2., 0.);
    p6 = Point3d(getId(), 1., 2., 0.);

    LineString3d ls_left2(getId(), {p2, p5});
    LineString3d ls_right2(getId(), {p4, p6});

    p7 = Point3d(getId(), 0., 3., 0.);
    p8 = Point3d(getId(), 1., 3., 0.);

    LineString3d ls_left3(getId(), {p5, p7});
    LineString3d ls_right3(getId(), {p6, p8});

    p9 = Point3d(getId(), 0., 1., 0.);
    p10 = Point3d(getId(), 1., 1., 0.);

    LineString3d ls_left4(getId(), {p9, p5});
    LineString3d ls_right4(getId(), {p10, p6});

    road_lanelet = Lanelet(getId(), ls_left, ls_right);
    road_lanelet.attributes()[lanelet::AttributeName::Subtype] =
      lanelet::AttributeValueString::Road;

    next_lanelet = Lanelet(getId(), ls_left2, ls_right2);
    next_lanelet.attributes()[lanelet::AttributeName::Subtype] =
      lanelet::AttributeValueString::Road;

    next_lanelet2 = Lanelet(getId(), ls_left3, ls_right3);
    next_lanelet2.attributes()[lanelet::AttributeName::Subtype] =
      lanelet::AttributeValueString::Road;

    merging_lanelet = Lanelet(getId(), ls_left4, ls_right4);
    merging_lanelet.attributes()[lanelet::AttributeName::Subtype] =
      lanelet::AttributeValueString::Road;

    // create sample lanelets
    Point3d cp1 = Point3d(getId(), 0.5, 0., 0.);
    Point3d cp2 = Point3d(getId(), 0.5, 0.5, 0.);
    Point3d cp3 = Point3d(getId(), 0.5, 1., 0.);

    LineString3d ls_centerline(getId(), {cp1, cp2, cp3});
    road_lanelet.setCenterline(ls_centerline);

    sample_map_ptr->add(road_lanelet);
    sample_map_ptr->add(next_lanelet);
    sample_map_ptr->add(next_lanelet2);
    sample_map_ptr->add(merging_lanelet);
  }

  ~TestSuite() override = default;

  lanelet::LaneletMapPtr sample_map_ptr;
  Lanelet road_lanelet;
  Lanelet next_lanelet;
  Lanelet next_lanelet2;
  Lanelet merging_lanelet;

private:
};

TEST_F(TestSuite, OverwriteLaneletsCenterlineWithWaypoints)  // NOLINT for gtest
{
  double resolution = 5.0;
  bool force_overwrite = false;

  // memorize the original information of the centerline
  std::unordered_map<lanelet::Id, lanelet::Id> lanelet_centerline_map{};
  for (const auto & lanelet : sample_map_ptr->laneletLayer) {
    if (lanelet.hasCustomCenterline()) {
      lanelet_centerline_map.insert({lanelet.id(), lanelet.centerline().id()});
    }
  }

  // convert centerline to waypoints
  lanelet::utils::overwriteLaneletsCenterlineWithWaypoints(
    sample_map_ptr, resolution, force_overwrite);

  for (const auto & lanelet : sample_map_ptr->laneletLayer) {
    if (lanelet_centerline_map.find(lanelet.id()) != lanelet_centerline_map.end()) {
      // check if the lanelet has waypoints.
      ASSERT_TRUE(lanelet.hasAttribute("waypoints")) << "The lanelet does not have a waypoints.";
      // check if the waypoints points to the linestring of the centerline.
      ASSERT_TRUE(
        lanelet.attribute("waypoints").asId().value() == lanelet_centerline_map.at(lanelet.id()))
        << "The waypoint's ID is invalid.";
    }
  }

  // check if all the lanelets have a centerline
  for (const auto & lanelet : sample_map_ptr->laneletLayer) {
    ASSERT_TRUE(lanelet.hasCustomCenterline()) << "failed to calculate fine centerline";
  }
}

TEST_F(TestSuite, OverwriteLaneletsCenterline)  // NOLINT for gtest
{
  double resolution = 5.0;
  bool force_overwrite = false;
  lanelet::utils::overwriteLaneletsCenterline(sample_map_ptr, resolution, force_overwrite);

  // check if all the lanelets have a centerline
  for (const auto & lanelet : sample_map_ptr->laneletLayer) {
    ASSERT_TRUE(lanelet.hasCustomCenterline()) << "failed to calculate fine centerline";
  }
}

/*
TEST(Utilities, copyZ)  // NOLINT for gtest
{
  using lanelet::utils::copyZ;

  LineString3d ls1;
  LineString3d ls2;

  ASSERT_NO_THROW(copyZ(ls1, ls2));
  ls1.push_back(Point3d(lanelet::InvalId, 0.0, 0.0, 5.0));
  ASSERT_NO_THROW(copyZ(ls1, ls2));
  ls2.push_back(Point3d(lanelet::InvalId, 0.0, 0.0, 0.0));
  ASSERT_NO_THROW(copyZ(ls1, ls2));
  EXPECT_EQ(ls2.front().z(), ls1.front().z());
  ls1.push_back(Point3d(lanelet::InvalId, 1.0, 0.0, 2.0));
  ls2.push_back(Point3d(lanelet::InvalId, 3.0, 0.0, 0.0));
  ASSERT_NO_THROW(copyZ(ls1, ls2));
  EXPECT_EQ(ls2.front().z(), ls1.front().z());
  EXPECT_EQ(ls2.back().z(), ls1.back().z());

  ls1.push_back(Point3d(lanelet::InvalId, 2.0, 0.0, 0.0));
  ls1.push_back(Point3d(lanelet::InvalId, 3.0, 0.0, 3.0));
  lanelet::Points3d points;
  points.emplace_back(lanelet::InvalId, 0.0, 0.0, 0.0);
  points.emplace_back(lanelet::InvalId, 0.0, 0.5, 0.0);
  points.emplace_back(lanelet::InvalId, 0.0, 1.0, 0.0);
  points.emplace_back(lanelet::InvalId, 0.0, 1.5, 0.0);
  points.emplace_back(lanelet::InvalId, 0.0, 2.0, 0.0);
  points.emplace_back(lanelet::InvalId, 0.0, 2.5, 0.0);
  points.emplace_back(lanelet::InvalId, 0.0, 3.0, 0.0);
  points.emplace_back(lanelet::InvalId, 0.0, 3.5, 0.0);
  ASSERT_NO_THROW(copyZ(ls1, points));
  EXPECT_EQ(points[0].z(), 5.0);
  EXPECT_EQ(points[1].z(), 3.5);
  EXPECT_EQ(points[2].z(), 2.0);
  EXPECT_EQ(points[3].z(), 1.0);
  EXPECT_EQ(points[4].z(), 0.0);
  EXPECT_EQ(points[5].z(), 1.5);
  EXPECT_EQ(points[6].z(), 3.0);
  EXPECT_EQ(points[7].z(), 3.0);
}
*/

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

// NOLINTEND(readability-identifier-naming, cppcoreguidelines-avoid-goto)
