/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <gtest/gtest.h>
#include <limits>
#include <gz/math/Helpers.hh>

#include "gz/msgs/Utility.hh"
#include "gz/msgs/PointCloudPackedUtils.hh"

#include "gz/msgs/wrench.pb.h"

using namespace gz;

/////////////////////////////////////////////////
TEST(UtilityTest, CovertMathVector3ToMsgs)
{
  msgs::Vector3d msg = msgs::Convert(math::Vector3d(1, 2, 3));
  EXPECT_DOUBLE_EQ(1, msg.x());
  EXPECT_DOUBLE_EQ(2, msg.y());
  EXPECT_DOUBLE_EQ(3, msg.z());
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertMsgsVector3dToMath)
{
  msgs::Vector3d msg = msgs::Convert(math::Vector3d(1, 2, 3));
  math::Vector3d v = msgs::Convert(msg);
  EXPECT_DOUBLE_EQ(1, v.X());
  EXPECT_DOUBLE_EQ(2, v.Y());
  EXPECT_DOUBLE_EQ(3, v.Z());
}

/////////////////////////////////////////////////
TEST(UtilityTest, CovertMathVector2ToMsgs)
{
  msgs::Vector2d msg = msgs::Convert(math::Vector2d(1, 2));
  EXPECT_DOUBLE_EQ(1, msg.x());
  EXPECT_DOUBLE_EQ(2, msg.y());
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertMsgsVector2dToMath)
{
  msgs::Vector2d msg = msgs::Convert(math::Vector2d(1, 2));
  math::Vector2d v = msgs::Convert(msg);
  EXPECT_DOUBLE_EQ(1, v.X());
  EXPECT_DOUBLE_EQ(2, v.Y());
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertMathQuaterionToMsgs)
{
  msgs::Quaternion msg =
    msgs::Convert(math::Quaterniond(GZ_PI * 0.25, GZ_PI * 0.5, GZ_PI));

  EXPECT_TRUE(math::equal(msg.x(), -0.65328148243818818));
  EXPECT_TRUE(math::equal(msg.y(), 0.27059805007309856));
  EXPECT_TRUE(math::equal(msg.z(), 0.65328148243818829));
  EXPECT_TRUE(math::equal(msg.w(), 0.27059805007309851));
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertMsgsQuaterionToMath)
{
  msgs::Quaternion msg =
    msgs::Convert(math::Quaterniond(GZ_PI * 0.25, GZ_PI * 0.5, GZ_PI));
  math::Quaterniond v = msgs::Convert(msg);

  // TODO(anyone): to real unit test move math::equal to EXPECT_DOUBLE_EQ
  EXPECT_TRUE(math::equal(v.X(), -0.65328148243818818));
  EXPECT_TRUE(math::equal(v.Y(), 0.27059805007309856));
  EXPECT_TRUE(math::equal(v.Z(), 0.65328148243818829));
  EXPECT_TRUE(math::equal(v.W(), 0.27059805007309851));
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertPoseMathToMsgs)
{
  msgs::Pose msg = msgs::Convert(math::Pose3d(
        math::Vector3d(1, 2, 3),
        math::Quaterniond(GZ_PI * 0.25, GZ_PI * 0.5, GZ_PI)));

  EXPECT_DOUBLE_EQ(1, msg.position().x());
  EXPECT_DOUBLE_EQ(2, msg.position().y());
  EXPECT_DOUBLE_EQ(3, msg.position().z());

  EXPECT_TRUE(math::equal(msg.orientation().x(), -0.65328148243818818));
  EXPECT_TRUE(math::equal(msg.orientation().y(), 0.27059805007309856));
  EXPECT_TRUE(math::equal(msg.orientation().z(), 0.65328148243818829));
  EXPECT_TRUE(math::equal(msg.orientation().w(), 0.27059805007309851));
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertMsgPoseToMath)
{
  msgs::Pose msg = msgs::Convert(
      math::Pose3d(math::Vector3d(1, 2, 3),
        math::Quaterniond(GZ_PI * 0.25, GZ_PI * 0.5, GZ_PI)));
  math::Pose3d v = msgs::Convert(msg);

  EXPECT_DOUBLE_EQ(1, v.Pos().X());
  EXPECT_DOUBLE_EQ(2, v.Pos().Y());
  EXPECT_DOUBLE_EQ(3, v.Pos().Z());
  EXPECT_TRUE(math::equal(v.Rot().X(), -0.65328148243818818));
  EXPECT_TRUE(math::equal(v.Rot().Y(), 0.27059805007309856));
  EXPECT_TRUE(math::equal(v.Rot().Z(), 0.65328148243818829));
  EXPECT_TRUE(math::equal(v.Rot().W(), 0.27059805007309851));
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertMsgPoseToMathIdentity)
{
  // Setting position but not orientation should still
  // result in a valid identity quaternion
  msgs::Pose msg;
  msg.mutable_position()->set_x(1.0);
  msg.mutable_position()->set_y(2.0);
  msg.mutable_position()->set_z(2.0);

  math::Pose3d v = msgs::Convert(msg);
  EXPECT_TRUE(math::equal(v.Rot().W(), 1.0));
  EXPECT_TRUE(math::equal(v.Rot().X(), 0.0));
  EXPECT_TRUE(math::equal(v.Rot().Y(), 0.0));
  EXPECT_TRUE(math::equal(v.Rot().Z(), 0.0));
}

/////////////////////////////////////////////////
TEST(MsgsTest, ConvertMathColorToMsgs)
{
  msgs::Color msg = msgs::Convert(math::Color(.1f, .2f, .3f, 1.0f));

  EXPECT_FLOAT_EQ(0.1f, msg.r());
  EXPECT_FLOAT_EQ(0.2f, msg.g());
  EXPECT_FLOAT_EQ(0.3f, msg.b());
  EXPECT_FLOAT_EQ(1.0f, msg.a());
}

/////////////////////////////////////////////////
TEST(MsgsTest, ConvertMsgsColorToMath)
{
  msgs::Color msg = msgs::Convert(math::Color(.1f, .2f, .3f, 1.0f));
  math::Color v = msgs::Convert(msg);

  EXPECT_FLOAT_EQ(0.1f, v.R());
  EXPECT_FLOAT_EQ(0.2f, v.G());
  EXPECT_FLOAT_EQ(0.3f, v.B());
  EXPECT_FLOAT_EQ(1.0f, v.A());
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertMathPlaneToMsgs)
{
  msgs::PlaneGeom msg = msgs::Convert(
      math::Planed(math::Vector3d(0, 0, 1),
        math::Vector2d(123, 456), 1.0));

  EXPECT_DOUBLE_EQ(0, msg.normal().x());
  EXPECT_DOUBLE_EQ(0, msg.normal().y());
  EXPECT_DOUBLE_EQ(1, msg.normal().z());

  EXPECT_DOUBLE_EQ(123, msg.size().x());
  EXPECT_DOUBLE_EQ(456, msg.size().y());
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertMsgsPlaneToMath)
{
  msgs::PlaneGeom msg = msgs::Convert(
      math::Planed(math::Vector3d(0, 0, 1),
        math::Vector2d(123, 456), 1.0));
  math::Planed v = msgs::Convert(msg);

  EXPECT_DOUBLE_EQ(0, v.Normal().X());
  EXPECT_DOUBLE_EQ(0, v.Normal().Y());
  EXPECT_DOUBLE_EQ(1, v.Normal().Z());

  EXPECT_DOUBLE_EQ(123, v.Size().X());
  EXPECT_DOUBLE_EQ(456, v.Size().Y());

  EXPECT_TRUE(math::equal(1.0, v.Offset()));
}

/////////////////////////////////////////////////
TEST(MsgsTest, ConvertMathInertialToMsgs)
{
  const auto pose = math::Pose3d(5, 6, 7, 0.4, 0.5, 0.6);
  msgs::Inertial msg = msgs::Convert(
      math::Inertiald(
        math::MassMatrix3d(12.0,
          math::Vector3d(2, 3, 4),
          math::Vector3d(0.1, 0.2, 0.3)),
        pose));

  EXPECT_DOUBLE_EQ(12.0, msg.mass());
  EXPECT_DOUBLE_EQ(2.0, msg.ixx());
  EXPECT_DOUBLE_EQ(3.0, msg.iyy());
  EXPECT_DOUBLE_EQ(4.0, msg.izz());
  EXPECT_DOUBLE_EQ(0.1, msg.ixy());
  EXPECT_DOUBLE_EQ(0.2, msg.ixz());
  EXPECT_DOUBLE_EQ(0.3, msg.iyz());
  EXPECT_EQ(pose, msgs::Convert(msg.pose()));
  EXPECT_TRUE(msg.fluid_added_mass().empty());
}

/////////////////////////////////////////////////
TEST(MsgsTest, ConvertMsgsInertialToMath)
{
  const auto pose = math::Pose3d(5, 6, 7, 0.4, 0.5, 0.6);
  msgs::Inertial msg = msgs::Convert(
      math::Inertiald(
        math::MassMatrix3d(12.0,
          math::Vector3d(2, 3, 4),
          math::Vector3d(0.1, 0.2, 0.3)),
        pose));
  gz::math::Inertial inertial = msgs::Convert(msg);

  EXPECT_DOUBLE_EQ(12.0, inertial.MassMatrix().Mass());
  EXPECT_DOUBLE_EQ(2.0, inertial.MassMatrix().Ixx());
  EXPECT_DOUBLE_EQ(3.0, inertial.MassMatrix().Iyy());
  EXPECT_DOUBLE_EQ(4.0, inertial.MassMatrix().Izz());
  EXPECT_DOUBLE_EQ(0.1, inertial.MassMatrix().Ixy());
  EXPECT_DOUBLE_EQ(0.2, inertial.MassMatrix().Ixz());
  EXPECT_DOUBLE_EQ(0.3, inertial.MassMatrix().Iyz());
  EXPECT_EQ(pose, inertial.Pose());
  EXPECT_FALSE(inertial.FluidAddedMass().has_value());
}

/////////////////////////////////////////////////
TEST(MsgsTest, ConvertMathInertialAddedMassToMsgs)
{
  const auto pose = math::Pose3d(5, 6, 7, 0.4, 0.5, 0.6);
  math::Matrix6d addedMass{
      0.1, 0.2, 0.3, 0.4, 0.5, 0.6,
      0.2, 0.7, 0.8, 0.9, 1.0, 1.1,
      0.3, 0.8, 1.2, 1.3, 1.4, 1.5,
      0.4, 0.9, 1.3, 1.6, 1.7, 1.8,
      0.5, 1.0, 1.4, 1.7, 1.9, 2.0,
      0.6, 1.1, 1.5, 1.8, 2.0, 2.1};

  auto msg = msgs::Convert(
      math::Inertiald(
        math::MassMatrix3d(12.0,
          math::Vector3d(2, 3, 4),
          math::Vector3d(0.1, 0.2, 0.3)),
        pose, addedMass));

  EXPECT_DOUBLE_EQ(12.0, msg.mass());
  EXPECT_DOUBLE_EQ(2.0, msg.ixx());
  EXPECT_DOUBLE_EQ(3.0, msg.iyy());
  EXPECT_DOUBLE_EQ(4.0, msg.izz());
  EXPECT_DOUBLE_EQ(0.1, msg.ixy());
  EXPECT_DOUBLE_EQ(0.2, msg.ixz());
  EXPECT_DOUBLE_EQ(0.3, msg.iyz());
  EXPECT_EQ(pose, msgs::Convert(msg.pose()));
  ASSERT_EQ(21, msg.fluid_added_mass().size());
  EXPECT_DOUBLE_EQ(0.1, msg.fluid_added_mass(0));
  EXPECT_DOUBLE_EQ(0.2, msg.fluid_added_mass(1));
  EXPECT_DOUBLE_EQ(0.3, msg.fluid_added_mass(2));
  EXPECT_DOUBLE_EQ(0.4, msg.fluid_added_mass(3));
  EXPECT_DOUBLE_EQ(0.5, msg.fluid_added_mass(4));
  EXPECT_DOUBLE_EQ(0.6, msg.fluid_added_mass(5));
  EXPECT_DOUBLE_EQ(0.7, msg.fluid_added_mass(6));
  EXPECT_DOUBLE_EQ(0.8, msg.fluid_added_mass(7));
  EXPECT_DOUBLE_EQ(0.9, msg.fluid_added_mass(8));
  EXPECT_DOUBLE_EQ(1.0, msg.fluid_added_mass(9));
  EXPECT_DOUBLE_EQ(1.1, msg.fluid_added_mass(10));
  EXPECT_DOUBLE_EQ(1.2, msg.fluid_added_mass(11));
  EXPECT_DOUBLE_EQ(1.3, msg.fluid_added_mass(12));
  EXPECT_DOUBLE_EQ(1.4, msg.fluid_added_mass(13));
  EXPECT_DOUBLE_EQ(1.5, msg.fluid_added_mass(14));
  EXPECT_DOUBLE_EQ(1.6, msg.fluid_added_mass(15));
  EXPECT_DOUBLE_EQ(1.7, msg.fluid_added_mass(16));
  EXPECT_DOUBLE_EQ(1.8, msg.fluid_added_mass(17));
  EXPECT_DOUBLE_EQ(1.9, msg.fluid_added_mass(18));
  EXPECT_DOUBLE_EQ(2.0, msg.fluid_added_mass(19));
  EXPECT_DOUBLE_EQ(2.1, msg.fluid_added_mass(20));
}

/////////////////////////////////////////////////
TEST(MsgsTest, ConvertMsgsInertialAddedMassToMath)
{
  const auto pose = math::Pose3d(5, 6, 7, 0.4, 0.5, 0.6);
  math::Matrix6d addedMass{
      0.1, 0.2, 0.3, 0.4, 0.5, 0.6,
      0.2, 0.7, 0.8, 0.9, 1.0, 1.1,
      0.3, 0.8, 1.2, 1.3, 1.4, 1.5,
      0.4, 0.9, 1.3, 1.6, 1.7, 1.8,
      0.5, 1.0, 1.4, 1.7, 1.9, 2.0,
      0.6, 1.1, 1.5, 1.8, 2.0, 2.1};

  auto msg = msgs::Convert(
      math::Inertiald(
        math::MassMatrix3d(12.0,
          math::Vector3d(2, 3, 4),
          math::Vector3d(0.1, 0.2, 0.3)),
        pose, addedMass));
  auto inertial = msgs::Convert(msg);

  EXPECT_DOUBLE_EQ(12.0, inertial.MassMatrix().Mass());
  EXPECT_DOUBLE_EQ(2.0, inertial.MassMatrix().Ixx());
  EXPECT_DOUBLE_EQ(3.0, inertial.MassMatrix().Iyy());
  EXPECT_DOUBLE_EQ(4.0, inertial.MassMatrix().Izz());
  EXPECT_DOUBLE_EQ(0.1, inertial.MassMatrix().Ixy());
  EXPECT_DOUBLE_EQ(0.2, inertial.MassMatrix().Ixz());
  EXPECT_DOUBLE_EQ(0.3, inertial.MassMatrix().Iyz());
  EXPECT_EQ(pose, inertial.Pose());
  ASSERT_TRUE(inertial.FluidAddedMass().has_value());
  EXPECT_EQ(addedMass, inertial.FluidAddedMass().value());
}

/////////////////////////////////////////////////
TEST(MsgsTest, ConvertMathMassMatrix3ToMsgs)
{
  msgs::Inertial msg = msgs::Convert(
      math::MassMatrix3d(12.0,
        math::Vector3d(2, 3, 4),
        math::Vector3d(0.1, 0.2, 0.3)));

  EXPECT_DOUBLE_EQ(12.0, msg.mass());
  EXPECT_DOUBLE_EQ(2.0, msg.ixx());
  EXPECT_DOUBLE_EQ(3.0, msg.iyy());
  EXPECT_DOUBLE_EQ(4.0, msg.izz());
  EXPECT_DOUBLE_EQ(0.1, msg.ixy());
  EXPECT_DOUBLE_EQ(0.2, msg.ixz());
  EXPECT_DOUBLE_EQ(0.3, msg.iyz());

  auto convertedPose = msgs::Convert(msg.pose());
  EXPECT_TRUE(convertedPose.Equal(math::Pose3d::Zero, 1e-1));
}

/////////////////////////////////////////////////
TEST(MsgsTest, ConvertMathSphericalCoordinatesToMsgs)
{
  EXPECT_EQ(msgs::SphericalCoordinatesType::ECEF,
    msgs::ConvertCoord(math::SphericalCoordinates::CoordinateType::ECEF));
  EXPECT_EQ(msgs::SphericalCoordinatesType::GLOBAL,
    msgs::ConvertCoord(math::SphericalCoordinates::CoordinateType::GLOBAL));
  EXPECT_EQ(msgs::SphericalCoordinatesType::SPHERICAL,
    msgs::ConvertCoord(math::SphericalCoordinates::CoordinateType::SPHERICAL));
  EXPECT_EQ(msgs::SphericalCoordinatesType::LOCAL,
    msgs::ConvertCoord(math::SphericalCoordinates::CoordinateType::LOCAL));
  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  EXPECT_EQ(msgs::SphericalCoordinatesType::LOCAL2,
    msgs::ConvertCoord(math::SphericalCoordinates::CoordinateType::LOCAL2));
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION

  EXPECT_EQ(math::SphericalCoordinates::CoordinateType::ECEF,
    msgs::Convert(msgs::SphericalCoordinatesType::ECEF));
  EXPECT_EQ(math::SphericalCoordinates::CoordinateType::GLOBAL,
    msgs::Convert(msgs::SphericalCoordinatesType::GLOBAL));
  EXPECT_EQ(math::SphericalCoordinates::CoordinateType::SPHERICAL,
    msgs::Convert(msgs::SphericalCoordinatesType::SPHERICAL));
  EXPECT_EQ(math::SphericalCoordinates::CoordinateType::LOCAL,
    msgs::Convert(msgs::SphericalCoordinatesType::LOCAL));
  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  EXPECT_EQ(math::SphericalCoordinates::CoordinateType::LOCAL2,
    msgs::Convert(msgs::SphericalCoordinatesType::LOCAL2));
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION


  auto msg = msgs::Convert(
      math::SphericalCoordinates(
      math::SphericalCoordinates::SurfaceType::EARTH_WGS84,
      GZ_DTOR(1.1), GZ_DTOR(2.2), 3.3, GZ_DTOR(0.4)));

  EXPECT_EQ(msgs::SphericalCoordinates::EARTH_WGS84, msg.surface_model());
  EXPECT_DOUBLE_EQ(1.1, msg.latitude_deg());
  EXPECT_DOUBLE_EQ(2.2, msg.longitude_deg());
  EXPECT_DOUBLE_EQ(3.3, msg.elevation());
  EXPECT_DOUBLE_EQ(0.4, msg.heading_deg());

  auto math = msgs::Convert(msg);

  EXPECT_EQ(math::SphericalCoordinates::EARTH_WGS84, math.Surface());
  EXPECT_DOUBLE_EQ(1.1, math.LatitudeReference().Degree());
  EXPECT_DOUBLE_EQ(2.2, math.LongitudeReference().Degree());
  EXPECT_DOUBLE_EQ(3.3, math.ElevationReference());
  EXPECT_DOUBLE_EQ(0.4, math.HeadingOffset().Degree());

  // For Moon's surface.
  auto msgMoon = msgs::Convert(
      math::SphericalCoordinates(
      math::SphericalCoordinates::SurfaceType::MOON_SCS,
      GZ_DTOR(1.1), GZ_DTOR(2.2), 3.3, GZ_DTOR(0.4)));

  EXPECT_EQ(msgs::SphericalCoordinates::MOON_SCS,
      msgMoon.surface_model());
  EXPECT_DOUBLE_EQ(1.1, msgMoon.latitude_deg());
  EXPECT_DOUBLE_EQ(2.2, msgMoon.longitude_deg());
  EXPECT_DOUBLE_EQ(3.3, msgMoon.elevation());
  EXPECT_DOUBLE_EQ(0.4, msgMoon.heading_deg());

  auto mathMoon = msgs::Convert(msgMoon);

  EXPECT_EQ(math::SphericalCoordinates::MOON_SCS,
      mathMoon.Surface());
  EXPECT_DOUBLE_EQ(1.1, mathMoon.LatitudeReference().Degree());
  EXPECT_DOUBLE_EQ(2.2, mathMoon.LongitudeReference().Degree());
  EXPECT_DOUBLE_EQ(3.3, mathMoon.ElevationReference());
  EXPECT_DOUBLE_EQ(0.4, mathMoon.HeadingOffset().Degree());

  // For custom surfaces.
  auto sc = math::SphericalCoordinates(
      math::SphericalCoordinates::CUSTOM_SURFACE,
      12000, 10000);
  sc.SetLatitudeReference(GZ_DTOR(1.1));
  sc.SetLongitudeReference(GZ_DTOR(2.2));
  sc.SetElevationReference(3.3);
  sc.SetHeadingOffset(GZ_DTOR(0.4));

  auto msgCustom = msgs::Convert(sc);

  EXPECT_EQ(msgs::SphericalCoordinates::CUSTOM_SURFACE,
      msgCustom.surface_model());
  EXPECT_DOUBLE_EQ(1.1, msgCustom.latitude_deg());
  EXPECT_DOUBLE_EQ(2.2, msgCustom.longitude_deg());
  EXPECT_DOUBLE_EQ(3.3, msgCustom.elevation());
  EXPECT_DOUBLE_EQ(0.4, msgCustom.heading_deg());
  EXPECT_DOUBLE_EQ(12000, msgCustom.surface_axis_equatorial());
  EXPECT_DOUBLE_EQ(10000, msgCustom.surface_axis_polar());

  auto mathCustom = msgs::Convert(msgCustom);

  EXPECT_EQ(math::SphericalCoordinates::CUSTOM_SURFACE,
      mathCustom.Surface());
  EXPECT_DOUBLE_EQ(1.1, mathCustom.LatitudeReference().Degree());
  EXPECT_DOUBLE_EQ(2.2, mathCustom.LongitudeReference().Degree());
  EXPECT_DOUBLE_EQ(3.3, mathCustom.ElevationReference());
  EXPECT_DOUBLE_EQ(0.4, mathCustom.HeadingOffset().Degree());
  EXPECT_DOUBLE_EQ(12000, mathCustom.SurfaceAxisEquatorial());
  EXPECT_DOUBLE_EQ(10000, mathCustom.SurfaceAxisPolar());
}

/////////////////////////////////////////////////
TEST(MsgsTest, ConvertMsgsSphericalCoordinatesTypeToMath)
{
  EXPECT_EQ(Convert(msgs::SphericalCoordinatesType::ECEF),
    math::SphericalCoordinates::CoordinateType::ECEF);
  EXPECT_EQ(Convert(msgs::SphericalCoordinatesType::GLOBAL),
    math::SphericalCoordinates::CoordinateType::GLOBAL);
  EXPECT_EQ(Convert(msgs::SphericalCoordinatesType::SPHERICAL),
    math::SphericalCoordinates::CoordinateType::SPHERICAL);
  EXPECT_EQ(Convert(msgs::SphericalCoordinatesType::LOCAL),
    math::SphericalCoordinates::CoordinateType::LOCAL);
  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  EXPECT_EQ(Convert(msgs::SphericalCoordinatesType::LOCAL2),
    math::SphericalCoordinates::CoordinateType::LOCAL2);
  EXPECT_EQ(Convert((msgs::SphericalCoordinatesType)500000),
    math::SphericalCoordinates::CoordinateType::LOCAL2);
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
}

/////////////////////////////////////////////////
TEST(MsgsTest, ConvertMathSphericalCoordinatedTypeToMsg)
{
  EXPECT_EQ(msgs::ConvertCoord(
      math::SphericalCoordinates::CoordinateType::ECEF),
    msgs::SphericalCoordinatesType::ECEF);
  EXPECT_EQ(msgs::ConvertCoord(
      math::SphericalCoordinates::CoordinateType::GLOBAL),
    msgs::SphericalCoordinatesType::GLOBAL);
  EXPECT_EQ(msgs::ConvertCoord(
      math::SphericalCoordinates::CoordinateType::SPHERICAL),
    msgs::SphericalCoordinatesType::SPHERICAL);
  EXPECT_EQ(msgs::ConvertCoord(
      math::SphericalCoordinates::CoordinateType::LOCAL),
    msgs::SphericalCoordinatesType::LOCAL);
  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  EXPECT_EQ(msgs::ConvertCoord(
      math::SphericalCoordinates::CoordinateType::LOCAL2),
    msgs::SphericalCoordinatesType::LOCAL2);
  EXPECT_EQ(msgs::ConvertCoord(
    (math::SphericalCoordinates::CoordinateType)500000),
    msgs::SphericalCoordinatesType::LOCAL2);
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertStringMsg)
{
  msgs::StringMsg msg = msgs::Convert(std::string("a string msg"));
  EXPECT_EQ("a string msg", msg.data());

  std::string s = msgs::Convert(msg);
  EXPECT_EQ(s, "a string msg");
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertBoolean)
{
  msgs::Boolean msg = msgs::Convert(static_cast<bool>(true));
  EXPECT_EQ(true, msg.data());

  bool s = msgs::Convert(msg);
  EXPECT_EQ(s, true);
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertInt32)
{
  msgs::Int32 msg = msgs::Convert(static_cast<int32_t>(-543));
  EXPECT_EQ(-543, msg.data());

  int s = msgs::Convert(msg);
  EXPECT_EQ(s, -543);
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertUInt32)
{
  msgs::UInt32 msg = msgs::Convert(123456u);
  EXPECT_EQ(123456u, msg.data());

  uint32_t s = msgs::Convert(msg);
  EXPECT_EQ(s, 123456u);
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertInt64)
{
  msgs::Int64 msg = msgs::Convert(std::numeric_limits<int64_t>::min());
  EXPECT_EQ(std::numeric_limits<int64_t>::min(), msg.data());

  int64_t s = msgs::Convert(msg);
  EXPECT_EQ(s, std::numeric_limits<int64_t>::min());
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertUInt64)
{
  msgs::UInt64 msg = msgs::Convert(std::numeric_limits<uint64_t>::max());
  EXPECT_EQ(std::numeric_limits<uint64_t>::max(), msg.data());

  uint64_t s = msgs::Convert(msg);
  EXPECT_EQ(s, std::numeric_limits<uint64_t>::max());
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertDouble)
{
  msgs::Double msg = msgs::Convert(static_cast<double>(-33.888));
  EXPECT_DOUBLE_EQ(-33.888, msg.data());

  double s = msgs::Convert(msg);
  EXPECT_DOUBLE_EQ(s, -33.888);
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertFloat)
{
  msgs::Float msg = msgs::Convert(0.999f);
  EXPECT_DOUBLE_EQ(0.999f, msg.data());

  float s = msgs::Convert(msg);
  EXPECT_DOUBLE_EQ(s, 0.999f);
}


/////////////////////////////////////////////////
TEST(UtilityTest, ConvertFuelMetadata)
{
  msgs::FuelMetadata metaMsg;
  std::string modelConfigInput, worldConfigInput;

  // test ConvertFuelMetadata(string, msgs::FuelMetadata)
  {
    EXPECT_FALSE(msgs::ConvertFuelMetadata(modelConfigInput, metaMsg));

    metaMsg.Clear();
    modelConfigInput = "<test/>";
    EXPECT_FALSE(msgs::ConvertFuelMetadata(modelConfigInput, metaMsg));

    metaMsg.Clear();
    modelConfigInput = "<model>test</model>";
    EXPECT_FALSE(msgs::ConvertFuelMetadata(modelConfigInput, metaMsg));

    // Test <model>
    metaMsg.Clear();
    modelConfigInput = R"(
      <model>
        <name>test_model</name>
      </model>
    )";
    EXPECT_FALSE(msgs::ConvertFuelMetadata(modelConfigInput, metaMsg));

    metaMsg.Clear();
    modelConfigInput = R"(<?xml version='1.0'?>
  <model>
    <sdf version='1.7'>model.sdf</sdf>
    <name>test_model</name>
    <version>3</version>
    <description>A model for testing</description>
    <author>
      <name>Foo Bar</name>
      <email>foo@bar.org</email>
    </author>
    <depend>
      <model>
        <uri>model://some_model</uri>
      </model>
    </depend>
  </model>
)";

    EXPECT_TRUE(msgs::ConvertFuelMetadata(modelConfigInput, metaMsg));
    EXPECT_EQ("test_model", metaMsg.name());
    EXPECT_EQ(3, metaMsg.version());
    EXPECT_EQ("A model for testing", metaMsg.description());
    EXPECT_EQ("model.sdf", metaMsg.model().file());
    EXPECT_EQ("sdf", metaMsg.model().file_format().name());
    EXPECT_EQ(1, metaMsg.model().file_format().version().major());
    EXPECT_EQ(7, metaMsg.model().file_format().version().minor());
    EXPECT_EQ(1, metaMsg.authors().size());
    EXPECT_EQ("Foo Bar", metaMsg.authors(0).name());
    EXPECT_EQ("foo@bar.org", metaMsg.authors(0).email());
    EXPECT_EQ(1, metaMsg.dependencies().size());
    EXPECT_EQ("model://some_model", metaMsg.dependencies(0).uri());

    // Test <world>
    metaMsg.Clear();
    worldConfigInput = R"(<?xml version='1.0'?>
  <world>
    <sdf version='1.7'>world.sdf</sdf>
    <name>test_world</name>
    <version>2</version>
    <description>A world for testing</description>
  </world>
)";
    EXPECT_TRUE(msgs::ConvertFuelMetadata(worldConfigInput, metaMsg));
    EXPECT_EQ("test_world", metaMsg.name());
    EXPECT_EQ(2, metaMsg.version());
    EXPECT_EQ("A world for testing", metaMsg.description());
    EXPECT_EQ("world.sdf", metaMsg.world().file());
    EXPECT_EQ("sdf", metaMsg.world().file_format().name());
    EXPECT_EQ(1, metaMsg.world().file_format().version().major());
    EXPECT_EQ(7, metaMsg.world().file_format().version().minor());
    EXPECT_EQ(0, metaMsg.authors().size());
    EXPECT_EQ(0, metaMsg.dependencies().size());
  }

  // test ConvertFuelMetadata(msgs::FuelMetadata, string)
  {
    std::string modelConfig;

    // Test <world>
    metaMsg.Clear();
    metaMsg.mutable_world()->set_file("world.sdf");
    EXPECT_FALSE(msgs::ConvertFuelMetadata(metaMsg, modelConfig));

    metaMsg.set_name("test_world");
    metaMsg.set_description("A world for testing");
    metaMsg.set_version(2);
    metaMsg.mutable_world()->mutable_file_format()->set_name("sdf");
    metaMsg.mutable_world()->mutable_file_format()
           ->mutable_version()->set_major(1);
    metaMsg.mutable_world()->mutable_file_format()
           ->mutable_version()->set_minor(7);
    EXPECT_TRUE(msgs::ConvertFuelMetadata(metaMsg, modelConfig));

    EXPECT_EQ(worldConfigInput, modelConfig);

    // Test <model>
    metaMsg.Clear();
    metaMsg.mutable_model()->set_file("model.sdf");
    EXPECT_FALSE(msgs::ConvertFuelMetadata(metaMsg, modelConfig));

    metaMsg.set_name("test_model");
    metaMsg.set_description("A model for testing");
    metaMsg.set_version(3);
    metaMsg.mutable_model()->mutable_file_format()->set_name("sdf");
    metaMsg.mutable_model()->mutable_file_format()
           ->mutable_version()->set_major(1);
    metaMsg.mutable_model()->mutable_file_format()
           ->mutable_version()->set_minor(7);
    EXPECT_TRUE(msgs::ConvertFuelMetadata(metaMsg, modelConfig));

    metaMsg.add_authors()->set_name("Foo Bar");
    metaMsg.mutable_authors(0)->set_email("foo@bar.org");
    metaMsg.add_dependencies()->set_uri("model://some_model");
    EXPECT_TRUE(msgs::ConvertFuelMetadata(metaMsg, modelConfig));
    EXPECT_EQ(modelConfigInput, modelConfig);
  }
}

/////////////////////////////////////////////////
TEST(UtilityTest, ConvertTimePoint)
{
  std::chrono::steady_clock::duration timePoint =
    std::chrono::steady_clock::duration::zero();
  msgs::Time msg = msgs::Convert(timePoint);
  EXPECT_EQ(0, msg.sec());
  EXPECT_EQ(0, msg.nsec());

  std::chrono::steady_clock::duration s = msgs::Convert(msg);
  EXPECT_EQ(s, std::chrono::steady_clock::duration::zero());

  timePoint = std::chrono::seconds(200) + std::chrono::nanoseconds(999);
  msg = msgs::Convert(timePoint);
  EXPECT_EQ(200, msg.sec());
  EXPECT_EQ(999, msg.nsec());
}

/////////////////////////////////////////////////
TEST(UtilityTest, SetVector3)
{
  msgs::Vector3d msg;
  msgs::Set(&msg, math::Vector3d(1, 2, 3));
  EXPECT_DOUBLE_EQ(1, msg.x());
  EXPECT_DOUBLE_EQ(2, msg.y());
  EXPECT_DOUBLE_EQ(3, msg.z());
}

/////////////////////////////////////////////////
TEST(UtilityTest, SetVector2d)
{
  msgs::Vector2d msg;
  msgs::Set(&msg, math::Vector2d(1, 2));
  EXPECT_DOUBLE_EQ(1, msg.x());
  EXPECT_DOUBLE_EQ(2, msg.y());
}

/////////////////////////////////////////////////
TEST(UtilityTest, SetQuaternion)
{
  msgs::Quaternion msg;
  msgs::Set(&msg, math::Quaterniond(GZ_PI * 0.25, GZ_PI * 0.5, GZ_PI));
  EXPECT_TRUE(math::equal(msg.x(), -0.65328148243818818));
  EXPECT_TRUE(math::equal(msg.y(), 0.27059805007309856));
  EXPECT_TRUE(math::equal(msg.z(), 0.65328148243818829));
  EXPECT_TRUE(math::equal(msg.w(), 0.27059805007309851));
}

/////////////////////////////////////////////////
TEST(UtilityTest, SetPose)
{
  msgs::Pose msg;
  msgs::Set(&msg, math::Pose3d(math::Vector3d(1, 2, 3),
        math::Quaterniond(GZ_PI * 0.25, GZ_PI * 0.5, GZ_PI)));

  EXPECT_DOUBLE_EQ(1, msg.position().x());
  EXPECT_DOUBLE_EQ(2, msg.position().y());
  EXPECT_DOUBLE_EQ(3, msg.position().z());

  EXPECT_TRUE(math::equal(msg.orientation().x(), -0.65328148243818818));
  EXPECT_TRUE(math::equal(msg.orientation().y(), 0.27059805007309856));
  EXPECT_TRUE(math::equal(msg.orientation().z(), 0.65328148243818829));
  EXPECT_TRUE(math::equal(msg.orientation().w(), 0.27059805007309851));
}

/////////////////////////////////////////////////
TEST(MsgsTest, SetColor)
{
  msgs::Color msg;
  msgs::Set(&msg, math::Color(.1f, .2f, .3f, 1.0f));
  EXPECT_FLOAT_EQ(0.1f, msg.r());
  EXPECT_FLOAT_EQ(0.2f, msg.g());
  EXPECT_FLOAT_EQ(0.3f, msg.b());
  EXPECT_FLOAT_EQ(1.0f, msg.a());
}

/////////////////////////////////////////////////
TEST(UtilityTest, SetPlane)
{
  msgs::PlaneGeom msg;
  msgs::Set(&msg, math::Planed(
        math::Vector3d(0, 0, 1),
        math::Vector2d(123, 456), 1.0));

  EXPECT_DOUBLE_EQ(0, msg.normal().x());
  EXPECT_DOUBLE_EQ(0, msg.normal().y());
  EXPECT_DOUBLE_EQ(1, msg.normal().z());

  EXPECT_DOUBLE_EQ(123, msg.size().x());
  EXPECT_DOUBLE_EQ(456, msg.size().y());

  EXPECT_TRUE(math::equal(1.0, msg.d()));
}

/////////////////////////////////////////////////
TEST(MsgsTest, SetInertial)
{
  const auto pose = math::Pose3d(5, 6, 7, 0.4, 0.5, 0.6);
  msgs::Inertial msg;
  msgs::Set(&msg, math::Inertiald(
      math::MassMatrix3d(
        12.0,
        math::Vector3d(2, 3, 4),
        math::Vector3d(0.1, 0.2, 0.3)),
      pose));

  EXPECT_DOUBLE_EQ(12.0, msg.mass());
  EXPECT_DOUBLE_EQ(2.0, msg.ixx());
  EXPECT_DOUBLE_EQ(3.0, msg.iyy());
  EXPECT_DOUBLE_EQ(4.0, msg.izz());
  EXPECT_DOUBLE_EQ(0.1, msg.ixy());
  EXPECT_DOUBLE_EQ(0.2, msg.ixz());
  EXPECT_DOUBLE_EQ(0.3, msg.iyz());
  EXPECT_EQ(pose, msgs::Convert(msg.pose()));
}

/////////////////////////////////////////////////
TEST(MsgsTest, SetMassMatrix3)
{
  msgs::Inertial msg;
  msgs::Set(&msg, math::MassMatrix3d(
        12.0,
        math::Vector3d(2, 3, 4),
        math::Vector3d(0.1, 0.2, 0.3)));

  EXPECT_DOUBLE_EQ(12.0, msg.mass());
  EXPECT_DOUBLE_EQ(2.0, msg.ixx());
  EXPECT_DOUBLE_EQ(3.0, msg.iyy());
  EXPECT_DOUBLE_EQ(4.0, msg.izz());
  EXPECT_DOUBLE_EQ(0.1, msg.ixy());
  EXPECT_DOUBLE_EQ(0.2, msg.ixz());
  EXPECT_DOUBLE_EQ(0.3, msg.iyz());
  EXPECT_EQ(math::Pose3d::Zero, msgs::Convert(msg.pose()));
}

/////////////////////////////////////////////////
TEST(MsgsTest, SetSphericalCoordinates)
{
  msgs::SphericalCoordinates msg;
  msgs::Set(&msg, math::SphericalCoordinates(
      math::SphericalCoordinates::SurfaceType::EARTH_WGS84,
      GZ_DTOR(1.1), GZ_DTOR(2.2), 3.3, GZ_DTOR(0.4)));

  EXPECT_EQ(msgs::SphericalCoordinates::EARTH_WGS84, msg.surface_model());
  EXPECT_DOUBLE_EQ(1.1, msg.latitude_deg());
  EXPECT_DOUBLE_EQ(2.2, msg.longitude_deg());
  EXPECT_DOUBLE_EQ(3.3, msg.elevation());
  EXPECT_DOUBLE_EQ(0.4, msg.heading_deg());

  // For Moon's surface.
  msgs::SphericalCoordinates msgMoon;
  msgs::Set(&msgMoon, math::SphericalCoordinates(
      math::SphericalCoordinates::SurfaceType::MOON_SCS,
      GZ_DTOR(1.2), GZ_DTOR(2.3), 3.4, GZ_DTOR(0.5)));

  EXPECT_EQ(msgs::SphericalCoordinates::MOON_SCS,
      msgMoon.surface_model());
  EXPECT_DOUBLE_EQ(1.2, msgMoon.latitude_deg());
  EXPECT_DOUBLE_EQ(2.3, msgMoon.longitude_deg());
  EXPECT_DOUBLE_EQ(3.4, msgMoon.elevation());
  EXPECT_DOUBLE_EQ(0.5, msgMoon.heading_deg());

  // For a custom surface.
  msgs::SphericalCoordinates msgCustom;
  auto sc = math::SphericalCoordinates(
      math::SphericalCoordinates::CUSTOM_SURFACE,
      12000, 10000);
  sc.SetLatitudeReference(GZ_DTOR(1.9));
  sc.SetLongitudeReference(GZ_DTOR(2.8));
  sc.SetElevationReference(3.7);
  sc.SetHeadingOffset(GZ_DTOR(0.6));

  msgs::Set(&msgCustom, sc);

  EXPECT_EQ(msgs::SphericalCoordinates::CUSTOM_SURFACE,
      msgCustom.surface_model());
  EXPECT_DOUBLE_EQ(1.9, msgCustom.latitude_deg());
  EXPECT_DOUBLE_EQ(2.8, msgCustom.longitude_deg());
  EXPECT_DOUBLE_EQ(3.7, msgCustom.elevation());
  EXPECT_DOUBLE_EQ(0.6, msgCustom.heading_deg());
  EXPECT_DOUBLE_EQ(12000, msgCustom.surface_axis_equatorial());
  EXPECT_DOUBLE_EQ(10000, msgCustom.surface_axis_polar());
}

/////////////////////////////////////////////////
TEST(MsgsTest, SetStringMsg)
{
  msgs::StringMsg msg;

  msgs::Set(&msg, "a string msg");
  EXPECT_EQ("a string msg", msg.data());

  msgs::Set(&msg, std::string("a string msg2"));
  EXPECT_EQ("a string msg2", msg.data());
}

/////////////////////////////////////////////////
TEST(MsgsTest, SetBoolean)
{
  msgs::Boolean msg;
  msgs::Set(&msg, false);

  EXPECT_FALSE(msg.data());
}

/////////////////////////////////////////////////
TEST(MsgsTest, SetInt32)
{
  msgs::Int32 msg;
  msgs::Set(&msg, 9976);

  EXPECT_EQ(9976, msg.data());
}

/////////////////////////////////////////////////
TEST(MsgsTest, SetUInt32)
{
  msgs::UInt32 msg;
  msgs::Set(&msg, 0u);

  EXPECT_EQ(0u, msg.data());
}

/////////////////////////////////////////////////
TEST(MsgsTest, SetInt64)
{
  msgs::Int64 msg;
  msgs::Set(&msg, std::numeric_limits<int64_t>::max());

  EXPECT_EQ(std::numeric_limits<int64_t>::max(), msg.data());
}

/////////////////////////////////////////////////
TEST(MsgsTest, SetUInt64)
{
  msgs::UInt64 msg;
  msgs::Set(&msg, std::numeric_limits<uint64_t>::min());

  EXPECT_EQ(std::numeric_limits<uint64_t>::min(), msg.data());
}

/////////////////////////////////////////////////
TEST(MsgsTest, SetDouble)
{
  msgs::Double msg;
  msgs::Set(&msg, 0.123);

  EXPECT_DOUBLE_EQ(0.123, msg.data());
}

/////////////////////////////////////////////////
TEST(MsgsTest, SetFloat)
{
  msgs::Float msg;
  msgs::Set(&msg, -367.0f);

  EXPECT_DOUBLE_EQ(-367.0f, msg.data());
}

/////////////////////////////////////////////////
TEST(UtilityTest, Initialization)
{
  {
    msgs::Vector3d msg;
    EXPECT_DOUBLE_EQ(0, msg.x());
    EXPECT_DOUBLE_EQ(0, msg.y());
    EXPECT_DOUBLE_EQ(0, msg.z());
  }

  {
    msgs::Wrench msg;
    EXPECT_DOUBLE_EQ(0, msg.force().x());
    EXPECT_DOUBLE_EQ(0, msg.force().y());
    EXPECT_DOUBLE_EQ(0, msg.force().z());
    EXPECT_DOUBLE_EQ(0, msg.torque().x());
    EXPECT_DOUBLE_EQ(0, msg.torque().y());
    EXPECT_DOUBLE_EQ(0, msg.torque().z());
  }
}

//////////////////////////////////////////////////
void CompareMsgsJointTypeToString(const msgs::Joint::Type _type)
{
  EXPECT_EQ(_type, msgs::ConvertJointType(msgs::ConvertJointType(_type)));
}

//////////////////////////////////////////////////
TEST(MsgsTest, ConvertMsgsJointTypeToString)
{
  CompareMsgsJointTypeToString(msgs::Joint::REVOLUTE);
  CompareMsgsJointTypeToString(msgs::Joint::REVOLUTE2);
  CompareMsgsJointTypeToString(msgs::Joint::PRISMATIC);
  CompareMsgsJointTypeToString(msgs::Joint::UNIVERSAL);
  CompareMsgsJointTypeToString(msgs::Joint::BALL);
  CompareMsgsJointTypeToString(msgs::Joint::SCREW);
  CompareMsgsJointTypeToString(msgs::Joint::GEARBOX);
  CompareMsgsJointTypeToString(msgs::Joint::FIXED);
  CompareMsgsJointTypeToString(msgs::Joint::CONTINUOUS);

  EXPECT_EQ(msgs::ConvertJointType("bad type"), msgs::Joint::REVOLUTE);
  EXPECT_EQ(msgs::ConvertJointType(msgs::Joint::Type(100)), "unknown");
}

//////////////////////////////////////////////////
void CompareMsgsGeometryTypeToString(const msgs::Geometry::Type _type)
{
  EXPECT_EQ(_type, msgs::ConvertGeometryType(msgs::ConvertGeometryType(_type)));
}

//////////////////////////////////////////////////
TEST(MsgsTest, ConvertMsgsGeometryTypeToString)
{
  CompareMsgsGeometryTypeToString(msgs::Geometry::BOX);
  CompareMsgsGeometryTypeToString(msgs::Geometry::SPHERE);
  CompareMsgsGeometryTypeToString(msgs::Geometry::CAPSULE);
  CompareMsgsGeometryTypeToString(msgs::Geometry::CONE);
  CompareMsgsGeometryTypeToString(msgs::Geometry::CYLINDER);
  CompareMsgsGeometryTypeToString(msgs::Geometry::ELLIPSOID);
  CompareMsgsGeometryTypeToString(msgs::Geometry::PLANE);
  CompareMsgsGeometryTypeToString(msgs::Geometry::IMAGE);
  CompareMsgsGeometryTypeToString(msgs::Geometry::HEIGHTMAP);
  CompareMsgsGeometryTypeToString(msgs::Geometry::MESH);
  CompareMsgsGeometryTypeToString(msgs::Geometry::POLYLINE);

  EXPECT_EQ(msgs::ConvertGeometryType("bad type"), msgs::Geometry::BOX);
  EXPECT_EQ(msgs::ConvertGeometryType(msgs::Geometry::Type(100)), "unknown");
}

//////////////////////////////////////////////////
void CompareMsgsShaderTypeToString(const msgs::Material::ShaderType _type)
{
  EXPECT_EQ(_type, msgs::ConvertShaderType(msgs::ConvertShaderType(_type)));
}

//////////////////////////////////////////////////
TEST(MsgsTest, ConvertMsgsShaderTypeToString)
{
  CompareMsgsShaderTypeToString(msgs::Material::NORMAL_MAP_OBJECT_SPACE);
  CompareMsgsShaderTypeToString(msgs::Material::NORMAL_MAP_TANGENT_SPACE);
  CompareMsgsShaderTypeToString(msgs::Material::PIXEL);
  CompareMsgsShaderTypeToString(msgs::Material::VERTEX);

  EXPECT_EQ(msgs::ConvertShaderType("bad type"), msgs::Material::VERTEX);
  EXPECT_EQ(msgs::ConvertShaderType(msgs::Material::ShaderType(100)),
      "unknown");
}

//////////////////////////////////////////////////
void CompareMsgsPixelFormatTypeToString(const msgs::PixelFormatType _type)
{
  EXPECT_EQ(_type,
      msgs::ConvertPixelFormatType(msgs::ConvertPixelFormatType(_type)))
      << msgs::ConvertPixelFormatType(_type);
}

//////////////////////////////////////////////////
TEST(MsgsTest, ConvertMsgsPixelFormatTypeToString)
{
  CompareMsgsPixelFormatTypeToString(
      msgs::PixelFormatType::UNKNOWN_PIXEL_FORMAT);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::L_INT8);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::L_INT16);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::RGB_INT8);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::RGBA_INT8);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::BGRA_INT8);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::RGB_INT16);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::RGB_INT32);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::BGR_INT8);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::BGR_INT16);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::BGR_INT32);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::R_FLOAT16);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::RGB_FLOAT16);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::R_FLOAT32);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::RGB_FLOAT32);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::BAYER_RGGB8);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::BAYER_BGGR8);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::BAYER_GBRG8);
  CompareMsgsPixelFormatTypeToString(msgs::PixelFormatType::BAYER_GRBG8);

  EXPECT_EQ(msgs::ConvertPixelFormatType("bad type"),
      msgs::PixelFormatType::UNKNOWN_PIXEL_FORMAT);
  EXPECT_EQ(msgs::ConvertPixelFormatType(msgs::PixelFormatType(100)),
      "UNKNOWN_PIXEL_FORMAT");
}

/////////////////////////////////////////////////
TEST(UtilityTest, CovertMathAxisAlignedBox)
{
  // Convert from math to msg
  msgs::AxisAlignedBox msg = msgs::Convert(
      math::AxisAlignedBox(-1, -2, -3, 12, 15, 20));
  EXPECT_DOUBLE_EQ(-1, msg.min_corner().x());
  EXPECT_DOUBLE_EQ(-2, msg.min_corner().y());
  EXPECT_DOUBLE_EQ(-3, msg.min_corner().z());

  EXPECT_DOUBLE_EQ(12, msg.max_corner().x());
  EXPECT_DOUBLE_EQ(15, msg.max_corner().y());
  EXPECT_DOUBLE_EQ(20, msg.max_corner().z());

  // Convert from msg to math
  math::AxisAlignedBox aab = msgs::Convert(msg);
  EXPECT_DOUBLE_EQ(-1, aab.Min().X());
  EXPECT_DOUBLE_EQ(-2, aab.Min().Y());
  EXPECT_DOUBLE_EQ(-3, aab.Min().Z());

  EXPECT_DOUBLE_EQ(12, aab.Max().X());
  EXPECT_DOUBLE_EQ(15, aab.Max().Y());
  EXPECT_DOUBLE_EQ(20, aab.Max().Z());

  // Set msg from math
  aab = msgs::Convert(msg);
  msgs::AxisAlignedBox msg2;
  msgs::Set(&msg2, aab);
  EXPECT_DOUBLE_EQ(-1, msg2.min_corner().x());
  EXPECT_DOUBLE_EQ(-2, msg2.min_corner().y());
  EXPECT_DOUBLE_EQ(-3, msg2.min_corner().z());

  EXPECT_DOUBLE_EQ(12, msg2.max_corner().x());
  EXPECT_DOUBLE_EQ(15, msg2.max_corner().y());
  EXPECT_DOUBLE_EQ(20, msg2.max_corner().z());

  // Try different values with Set
  aab.Min() = math::Vector3d(-23.4, 56.2, -123.5);
  aab.Max() = math::Vector3d(34.1, 92.1, -723.2);
  msgs::Set(&msg2, aab);
  EXPECT_DOUBLE_EQ(-23.4, msg2.min_corner().x());
  EXPECT_DOUBLE_EQ(56.2, msg2.min_corner().y());
  EXPECT_DOUBLE_EQ(-123.5, msg2.min_corner().z());

  EXPECT_DOUBLE_EQ(34.1, msg2.max_corner().x());
  EXPECT_DOUBLE_EQ(92.1, msg2.max_corner().y());
  EXPECT_DOUBLE_EQ(-723.2, msg2.max_corner().z());
}

/////////////////////////////////////////////////
TEST(UtilityTest, InitPointCloudPacked)
{
  msgs::PointCloudPacked pc;

  msgs::InitPointCloudPacked(pc, "my_frame", true,
      {{"xyz", msgs::PointCloudPacked::Field::FLOAT32},
      {"rgb", msgs::PointCloudPacked::Field::FLOAT32}});

  EXPECT_EQ(4, pc.field_size());
  if (4u == sizeof(size_t))
  {
    // 32-bit architecture
    EXPECT_EQ(16u, pc.point_step());
  }
  else
  {
    // otherwise assume 64-bit architecture
    EXPECT_EQ(24u, pc.point_step());
  }

  EXPECT_EQ("x", pc.field(0).name());
  EXPECT_EQ(0u, pc.field(0).offset());
  EXPECT_EQ(msgs::PointCloudPacked::Field::FLOAT32, pc.field(0).datatype());
  EXPECT_EQ(1u, pc.field(0).count());

  EXPECT_EQ("y", pc.field(1).name());
  EXPECT_EQ(4u, pc.field(1).offset());
  EXPECT_EQ(msgs::PointCloudPacked::Field::FLOAT32, pc.field(1).datatype());
  EXPECT_EQ(1u, pc.field(1).count());

  EXPECT_EQ("z", pc.field(2).name());
  EXPECT_EQ(8u, pc.field(2).offset());
  EXPECT_EQ(msgs::PointCloudPacked::Field::FLOAT32, pc.field(2).datatype());
  EXPECT_EQ(1u, pc.field(2).count());

  EXPECT_EQ("rgb", pc.field(3).name());
  if (4u == sizeof(size_t))
  {
    // 32-bit architecture
    EXPECT_EQ(12u, pc.field(3).offset());
  }
  else
  {
    // otherwise assume 64-bit architecture
    EXPECT_EQ(16u, pc.field(3).offset());
  }
  EXPECT_EQ(msgs::PointCloudPacked::Field::FLOAT32, pc.field(3).datatype());
  EXPECT_EQ(1u, pc.field(3).count());

  // Don't align to word boundaries
  msgs::InitPointCloudPacked(pc, "my_frame", false,
      {{"xyz", msgs::PointCloudPacked::Field::FLOAT32},
      {"rgb", msgs::PointCloudPacked::Field::FLOAT32}});
  EXPECT_EQ(4, pc.field_size());
  EXPECT_EQ(16u, pc.point_step());

  EXPECT_EQ("x", pc.field(0).name());
  EXPECT_EQ(0u, pc.field(0).offset());
  EXPECT_EQ(msgs::PointCloudPacked::Field::FLOAT32, pc.field(0).datatype());
  EXPECT_EQ(1u, pc.field(0).count());

  EXPECT_EQ("y", pc.field(1).name());
  EXPECT_EQ(4u, pc.field(1).offset());
  EXPECT_EQ(msgs::PointCloudPacked::Field::FLOAT32, pc.field(1).datatype());
  EXPECT_EQ(1u, pc.field(1).count());

  EXPECT_EQ("z", pc.field(2).name());
  EXPECT_EQ(8u, pc.field(2).offset());
  EXPECT_EQ(msgs::PointCloudPacked::Field::FLOAT32, pc.field(2).datatype());
  EXPECT_EQ(1u, pc.field(2).count());

  EXPECT_EQ("rgb", pc.field(3).name());
  EXPECT_EQ(12u, pc.field(3).offset());
  EXPECT_EQ(msgs::PointCloudPacked::Field::FLOAT32, pc.field(3).datatype());
  EXPECT_EQ(1u, pc.field(3).count());
  EXPECT_EQ("my_frame", pc.header().data(0).value(0));
  EXPECT_EQ("frame_id", pc.header().data(0).key());

  // Test data types
  msgs::InitPointCloudPacked(pc, "my_new_frame", false,
      {{"x", msgs::PointCloudPacked::Field::INT8},
       {"y", msgs::PointCloudPacked::Field::UINT8},
       {"z", msgs::PointCloudPacked::Field::INT16},
       {"r", msgs::PointCloudPacked::Field::UINT16},
       {"s", msgs::PointCloudPacked::Field::INT32},
       {"t", msgs::PointCloudPacked::Field::UINT32},
       {"u", msgs::PointCloudPacked::Field::FLOAT64}});

  EXPECT_EQ("frame_id", pc.header().data(0).key());
  EXPECT_EQ("my_new_frame", pc.header().data(0).value(0));

  EXPECT_EQ(0u, pc.field(0).offset());
  EXPECT_EQ(1u, pc.field(1).offset());
  EXPECT_EQ(2u, pc.field(2).offset());
  EXPECT_EQ(4u, pc.field(3).offset());
  EXPECT_EQ(6u, pc.field(4).offset());
  EXPECT_EQ(10u, pc.field(5).offset());
  EXPECT_EQ(14u, pc.field(6).offset());
  EXPECT_EQ(22u, pc.point_step());
}

/////////////////////////////////////////////////
TEST(UtilityTest, DiscoveryTypeString)
{
  EXPECT_EQ("UNINITIALIZED",
      msgs::ToString(msgs::Discovery::UNINITIALIZED));
  EXPECT_EQ("ADVERTISE",
      msgs::ToString(msgs::Discovery::ADVERTISE));
  EXPECT_EQ("SUBSCRIBE",
      msgs::ToString(msgs::Discovery::SUBSCRIBE));
  EXPECT_EQ("UNADVERTISE",
      msgs::ToString(msgs::Discovery::UNADVERTISE));
  EXPECT_EQ("HEARTBEAT",
      msgs::ToString(msgs::Discovery::HEARTBEAT));
  EXPECT_EQ("BYE",
      msgs::ToString(msgs::Discovery::BYE));
  EXPECT_EQ("NEW_CONNECTION",
      msgs::ToString(msgs::Discovery::NEW_CONNECTION));
  EXPECT_EQ("END_CONNECTION",
      msgs::ToString(msgs::Discovery::END_CONNECTION));
  EXPECT_EQ("SUBSCRIBERS_REQ",
      msgs::ToString(msgs::Discovery::SUBSCRIBERS_REQ));
  EXPECT_EQ("SUBSCRIBERS_REP",
      msgs::ToString(msgs::Discovery::SUBSCRIBERS_REP));

  // If any of the following fail, then make sure you have added then new
  // enum values to std::string ToString(const msgs::Discovery::Type &_t).
  // Then update the following tests to match the new enum values.
  EXPECT_EQ(msgs::Discovery::UNINITIALIZED,
      msgs::Discovery::Type_MIN);
  EXPECT_EQ(msgs::Discovery::SUBSCRIBERS_REP,
      msgs::Discovery::Type_MAX);
  EXPECT_EQ(10, msgs::Discovery::Type_ARRAYSIZE);
}
