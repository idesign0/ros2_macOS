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

// NOLINTBEGIN(readability-identifier-naming)

#include "autoware_lanelet2_extension/regulatory_elements/autoware_traffic_light.hpp"
#include "autoware_lanelet2_extension/regulatory_elements/bus_stop_area.hpp"
#include "autoware_lanelet2_extension/regulatory_elements/roundabout.hpp"

#include <boost/optional/optional_io.hpp>

#include <gtest/gtest.h>
#include <lanelet2_core/Attribute.h>
#include <lanelet2_core/LaneletMap.h>

#include <cmath>
#include <vector>

using lanelet::LineString3d;
using lanelet::LineStringOrPolygon3d;
using lanelet::Point3d;
using lanelet::Points3d;
using lanelet::Polygon3d;
using lanelet::utils::getId;

namespace
{
template <typename T>
std::vector<T> convertToVector(T item)
{
  std::vector<T> vector = {item};
  return vector;
}
}  // namespace

class TestSuite : public ::testing::Test  // NOLINT for gtest
{
public:
  TestSuite() = default;
  ~TestSuite() override = default;
};

TEST(TestSuite, FactoryConstructsTrafficLight)  // NOLINT for gtest
{
  Point3d p1;
  Point3d p2;
  Point3d p3;
  Point3d p4;
  Point3d p5;
  Point3d p6;
  Point3d p7;
  LineStringOrPolygon3d traffic_light_base;
  LineString3d traffic_light_bulbs;
  LineString3d stop_line;

  p1 = Point3d(getId(), 0., 1., 4.);
  p2 = Point3d(getId(), 1., 1., 4.);

  p3 = Point3d(getId(), 0., 1., 4.5);
  p4 = Point3d(getId(), 0.5, 1., 4.5);
  p5 = Point3d(getId(), 1., 1., 4.5);

  p6 = Point3d(getId(), 0., 0., 0.);
  p7 = Point3d(getId(), 1., 0., 0.);

  Points3d base = {p1, p2};
  Points3d bulbs = {p3, p4, p5};
  Points3d stop = {p6, p7};

  traffic_light_base = LineString3d(getId(), base);
  traffic_light_bulbs = LineString3d(getId(), bulbs);
  stop_line = LineString3d(getId(), stop);

  auto tl = lanelet::autoware::AutowareTrafficLight::make(
    getId(), lanelet::AttributeMap(), convertToVector(traffic_light_base), stop_line,
    convertToVector(traffic_light_bulbs));

  auto factoryTl = lanelet::RegulatoryElementFactory::create(
    tl->attribute(lanelet::AttributeName::Subtype).value(),
    std::const_pointer_cast<lanelet::RegulatoryElementData>(tl->constData()));
  EXPECT_TRUE(!!std::dynamic_pointer_cast<lanelet::TrafficLight>(factoryTl));
}

TEST(TestSuite, TrafficLightWorksAsExpected)  // NOLINT for gtest
{
  Point3d p1;
  Point3d p2;
  Point3d p3;
  Point3d p4;
  Point3d p5;
  Point3d p6;
  Point3d p7;

  LineStringOrPolygon3d traffic_light_base;
  LineStringOrPolygon3d traffic_light_base2;
  LineString3d traffic_light_bulbs;
  LineString3d traffic_light_bulbs2;
  LineString3d stop_line;

  p1 = Point3d(getId(), 0., 1., 4.);
  p2 = Point3d(getId(), 1., 1., 4.);

  p3 = Point3d(getId(), 0., 1., 4.5);
  p4 = Point3d(getId(), 0.5, 1., 4.5);
  p5 = Point3d(getId(), 1., 1., 4.5);

  p6 = Point3d(getId(), 0., 0., 0.);
  p7 = Point3d(getId(), 1., 0., 0.);

  Points3d base = {p1, p2};
  Points3d bulbs = {p3, p4, p5};
  Points3d stop = {p6, p7};

  traffic_light_base = {LineString3d(getId(), base)};
  traffic_light_base2 = {LineString3d(getId(), base)};
  traffic_light_bulbs = {LineString3d(getId(), bulbs)};
  traffic_light_bulbs2 = {LineString3d(getId(), bulbs)};
  stop_line = LineString3d(getId(), stop);

  auto tl = lanelet::autoware::AutowareTrafficLight::make(
    getId(), lanelet::AttributeMap(), convertToVector(traffic_light_base), stop_line,
    convertToVector(traffic_light_bulbs));
  tl->setStopLine(stop_line);
  EXPECT_EQ(stop_line, tl->stopLine());
  tl->addTrafficLight(traffic_light_base2);
  EXPECT_EQ(2ul, tl->trafficLights().size());
  tl->addLightBulbs(traffic_light_bulbs2);
  EXPECT_EQ(2ul, tl->lightBulbs().size());
  tl->removeLightBulbs(traffic_light_bulbs);
  EXPECT_EQ(1ul, tl->lightBulbs().size());
}

TEST(TestSuite, BusStopAreInstantiation)  // NOLINT for gtest
{
  /*
    p4 <---- p3
     |       ^
     |       |
     V       |
    p1 ----> p2
   */
  const Point3d p1{getId(), 0, 0, 0};
  const Point3d p2{getId(), 3, 0, 0};
  const Point3d p3{getId(), 3, 3, 0};
  const Point3d p4{getId(), 0, 3, 0};
  const Polygon3d polygon{LineString3d{getId(), Points3d{p1, p2, p3, p4}}};
  auto bus_stop_area_reg_elem = lanelet::autoware::format_v2::BusStopArea::make(
    getId(), lanelet::AttributeMap(), convertToVector(polygon));
  EXPECT_EQ(bus_stop_area_reg_elem->busStopAreas().size(), 1);
  EXPECT_NO_THROW(bus_stop_area_reg_elem->busStopAreas().at(0));
  const auto bus_stop_area = bus_stop_area_reg_elem->busStopAreas().at(0);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(TestSuite, RoundaboutInstantiation)  // NOLINT for gtest
{
  // create sample lanelets
  const Point3d p1(getId(), 0.0, 0.0, 0.0);
  const Point3d p2(getId(), 0.0, 1.0, 0.0);
  const LineString3d ls_left(getId(), {p1, p2});
  const Point3d p3(getId(), 1.0, 0.0, 0.0);
  const Point3d p4(getId(), 1.0, 0.0, 0.0);
  const LineString3d ls_right(getId(), {p3, p4});

  lanelet::Lanelet roundabout_entry_lanelet1(getId(), ls_left, ls_right);
  lanelet::Lanelet roundabout_entry_lanelet2(getId(), ls_left, ls_right);
  lanelet::Lanelet roundabout_exit_lanelet1(getId(), ls_left, ls_right);
  lanelet::Lanelet roundabout_exit_lanelet2(getId(), ls_left, ls_right);
  lanelet::Lanelet roundabout_exit_lanelet3(getId(), ls_left, ls_right);
  lanelet::Lanelet roundabout_internal_lanelet1(getId(), ls_left, ls_right);
  lanelet::Lanelet roundabout_internal_lanelet2(getId(), ls_left, ls_right);

  lanelet::Lanelets roundabout_entry_lanelets = {
    roundabout_entry_lanelet1, roundabout_entry_lanelet2};
  lanelet::Lanelets roundabout_exit_lanelets = {roundabout_exit_lanelet1, roundabout_exit_lanelet2};
  lanelet::Lanelets roundabout_internal_lanelets = {
    roundabout_internal_lanelet1, roundabout_internal_lanelet2};
  // create roundabout
  auto roundabout_reg_elem = lanelet::autoware::Roundabout::make(
    getId(), lanelet::AttributeMap{}, roundabout_entry_lanelets, roundabout_exit_lanelets,
    roundabout_internal_lanelets);
  EXPECT_EQ(roundabout_reg_elem->roundaboutLanelets().size(), 6);
  EXPECT_EQ(
    roundabout_reg_elem->roundaboutEntryLanelets().size(), roundabout_entry_lanelets.size());
  EXPECT_EQ(roundabout_reg_elem->roundaboutExitLanelets().size(), roundabout_exit_lanelets.size());
  EXPECT_EQ(
    roundabout_reg_elem->roundaboutInternalLanelets().size(), roundabout_internal_lanelets.size());
  EXPECT_TRUE(roundabout_reg_elem->isEntryLanelet(roundabout_entry_lanelet1.id()));
  EXPECT_TRUE(roundabout_reg_elem->isEntryLanelet(roundabout_entry_lanelet2.id()));
  EXPECT_FALSE(roundabout_reg_elem->isEntryLanelet(roundabout_exit_lanelet1.id()));
  EXPECT_FALSE(roundabout_reg_elem->isEntryLanelet(roundabout_exit_lanelet2.id()));
  EXPECT_FALSE(roundabout_reg_elem->isEntryLanelet(roundabout_exit_lanelet3.id()));
  EXPECT_FALSE(roundabout_reg_elem->isEntryLanelet(roundabout_internal_lanelet1.id()));
  EXPECT_FALSE(roundabout_reg_elem->isEntryLanelet(roundabout_internal_lanelet2.id()));
  EXPECT_TRUE(roundabout_reg_elem->isExitLanelet(roundabout_exit_lanelet1.id()));
  EXPECT_TRUE(roundabout_reg_elem->isExitLanelet(roundabout_exit_lanelet2.id()));
  EXPECT_FALSE(roundabout_reg_elem->isExitLanelet(roundabout_exit_lanelet3.id()));
  EXPECT_FALSE(roundabout_reg_elem->isExitLanelet(roundabout_entry_lanelet1.id()));
  EXPECT_FALSE(roundabout_reg_elem->isExitLanelet(roundabout_entry_lanelet2.id()));
  EXPECT_FALSE(roundabout_reg_elem->isExitLanelet(roundabout_internal_lanelet1.id()));
  EXPECT_FALSE(roundabout_reg_elem->isExitLanelet(roundabout_internal_lanelet2.id()));
  EXPECT_TRUE(roundabout_reg_elem->isInternalLanelet(roundabout_internal_lanelet1.id()));
  EXPECT_TRUE(roundabout_reg_elem->isInternalLanelet(roundabout_internal_lanelet2.id()));
  EXPECT_FALSE(roundabout_reg_elem->isInternalLanelet(roundabout_exit_lanelet3.id()));
  EXPECT_FALSE(roundabout_reg_elem->isInternalLanelet(roundabout_entry_lanelet1.id()));
  EXPECT_FALSE(roundabout_reg_elem->isInternalLanelet(roundabout_entry_lanelet2.id()));
  EXPECT_FALSE(roundabout_reg_elem->isInternalLanelet(roundabout_exit_lanelet1.id()));
  EXPECT_FALSE(roundabout_reg_elem->isInternalLanelet(roundabout_exit_lanelet2.id()));
  EXPECT_FALSE(roundabout_reg_elem->isInternalLanelet(roundabout_exit_lanelet3.id()));
  EXPECT_TRUE(roundabout_reg_elem->isRoundaboutLanelet(roundabout_entry_lanelet1.id()));
  EXPECT_FALSE(roundabout_reg_elem->isRoundaboutLanelet(roundabout_exit_lanelet3.id()));
}

// NOLINTEND(readability-identifier-naming)
