/*
 * Copyright (C) 2024 Open Source Robotics Foundation
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

#include <cmath>
#include <optional>
#include <sstream>

#include "gz/math/Angle.hh"
#include "gz/math/CoordinateVector3.hh"
#include "gz/math/Helpers.hh"
#include "gz/math/Vector3.hh"

using namespace gz;

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, Construction)
{
  // Empty constructor
  math::CoordinateVector3 vec;
  EXPECT_FALSE(vec.IsSpherical());
  ASSERT_TRUE(vec.IsMetric());
  ASSERT_TRUE(vec.X().has_value());
  ASSERT_TRUE(vec.Y().has_value());
  ASSERT_TRUE(vec.Z().has_value());
  EXPECT_FALSE(vec.Lat().has_value());
  EXPECT_FALSE(vec.Lon().has_value());
  EXPECT_EQ(*vec.X(), 0.0);
  EXPECT_EQ(*vec.Y(), 0.0);
  EXPECT_EQ(*vec.Z(), 0.0);

  // Metric constructor
  auto vec_m{math::CoordinateVector3::Metric(1, 2, 3)};
  EXPECT_FALSE(vec_m.IsSpherical());
  ASSERT_TRUE(vec_m.IsMetric());
  ASSERT_TRUE(vec_m.X().has_value());
  ASSERT_TRUE(vec_m.Y().has_value());
  ASSERT_TRUE(vec_m.Z().has_value());
  EXPECT_FALSE(vec_m.Lat().has_value());
  EXPECT_FALSE(vec_m.Lon().has_value());
  EXPECT_EQ(*vec_m.X(), 1.0);
  EXPECT_EQ(*vec_m.Y(), 2.0);
  EXPECT_EQ(*vec_m.Z(), 3.0);

  // Metric constructor from vector
  auto vec_m2{math::CoordinateVector3::Metric(math::Vector3d{1, 2, 3})};
  EXPECT_FALSE(vec_m2.IsSpherical());
  ASSERT_TRUE(vec_m2.IsMetric());
  ASSERT_TRUE(vec_m2.X().has_value());
  ASSERT_TRUE(vec_m2.Y().has_value());
  ASSERT_TRUE(vec_m2.Z().has_value());
  EXPECT_FALSE(vec_m2.Lat().has_value());
  EXPECT_FALSE(vec_m2.Lon().has_value());
  EXPECT_EQ(*vec_m2.X(), 1.0);
  EXPECT_EQ(*vec_m2.Y(), 2.0);
  EXPECT_EQ(*vec_m2.Z(), 3.0);

  // Spherical constructor
  auto vec_s{math::CoordinateVector3::Spherical(1, 2, 3)};
  EXPECT_FALSE(vec_s.IsMetric());
  ASSERT_TRUE(vec_s.IsSpherical());
  ASSERT_TRUE(vec_s.Lat().has_value());
  ASSERT_TRUE(vec_s.Lon().has_value());
  ASSERT_TRUE(vec_s.Z().has_value());
  EXPECT_FALSE(vec_s.X().has_value());
  EXPECT_FALSE(vec_s.Y().has_value());
  EXPECT_EQ(vec_s.Lat()->Radian(), 1.0);
  EXPECT_EQ(vec_s.Lon()->Radian(), 2.0);
  EXPECT_EQ(*vec_s.Z(), 3.0);

  // Copy constructor and equality
  math::CoordinateVector3 vec2(vec_m);
  EXPECT_EQ(vec2, vec_m);

  // Copy operator
  math::CoordinateVector3 vec3;
  vec3 = vec_m;
  EXPECT_EQ(vec3, vec_m);

  // Move constructor
  math::CoordinateVector3 vec4(std::move(vec_m));
  EXPECT_EQ(vec4, vec2);
  vec_m = vec4;
  EXPECT_EQ(vec_m, vec2);

  // Move operator
  math::CoordinateVector3 vec5;
  vec5 = std::move(vec2);
  EXPECT_EQ(vec5, vec3);
  vec2 = vec5;
  EXPECT_EQ(vec2, vec3);

  // Inequality
  math::CoordinateVector3 vec6;
  EXPECT_NE(vec6, vec3);
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, Setters)
{
  auto vec1{math::CoordinateVector3::Metric(0.1, 0.2, 0.4)};

  vec1.SetMetric(1.1, 2.2, 3.4);
  EXPECT_EQ(vec1, math::CoordinateVector3::Metric(1.1, 2.2, 3.4));

  vec1.SetMetric(math::Vector3d(-1.1, -2.2, -3.4));
  EXPECT_EQ(vec1, math::CoordinateVector3::Metric(-1.1, -2.2, -3.4));

  vec1.SetSpherical(1.1, 2.2, 3.4);
  EXPECT_EQ(vec1, math::CoordinateVector3::Spherical(1.1, 2.2, 3.4));
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, AsMetricVector)
{
  auto vec1{math::CoordinateVector3::Metric(0.1, 0.2, 0.4)};
  ASSERT_TRUE(vec1.AsMetricVector().has_value());
  EXPECT_EQ(*vec1.AsMetricVector(), math::Vector3d(0.1, 0.2, 0.4));

  auto vec2{math::CoordinateVector3::Spherical(0.1, 0.2, 0.4)};
  ASSERT_FALSE(vec2.AsMetricVector().has_value());
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, AddMetric)
{
  auto vec1{math::CoordinateVector3::Metric(0.1, 0.2, 0.4)};
  auto vec2{math::CoordinateVector3::Metric(1.1, 2.2, 3.4)};

  math::CoordinateVector3 vec3 = vec1;
  vec3 += vec2;

  EXPECT_EQ(vec1 + vec2, math::CoordinateVector3::Metric(1.2, 2.4, 3.8));
  EXPECT_EQ(vec3, math::CoordinateVector3::Metric(1.2, 2.4, 3.8));
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, AddSpherical)
{
  auto vec1{math::CoordinateVector3::Spherical(0.1, 0.2, 0.4)};
  auto vec2{math::CoordinateVector3::Spherical(1.1, 2.2, 3.4)};

  math::CoordinateVector3 vec3 = vec1;
  vec3 += vec2;

  EXPECT_EQ(vec1 + vec2, math::CoordinateVector3::Spherical(1.2, 2.4, 3.8));
  EXPECT_EQ(vec3, math::CoordinateVector3::Spherical(1.2, 2.4, 3.8));
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, AddMismatch)
{
  auto vec1{math::CoordinateVector3::Spherical(0.1, 0.2, 0.4)};
  auto vec2{math::CoordinateVector3::Metric(1.1, 2.2, 3.4)};

  math::CoordinateVector3 vec3 = vec1;
  vec3 += vec2;
  math::CoordinateVector3 vec4 = vec2;
  vec4 += vec1;

  math::CoordinateVector3 vec12 = vec1 + vec2;
  math::CoordinateVector3 vec21 = vec2 + vec1;

  EXPECT_FALSE(vec3.IsMetric());
  EXPECT_FALSE(vec12.IsMetric());
  EXPECT_FALSE(vec4.IsSpherical());
  EXPECT_FALSE(vec21.IsSpherical());
  ASSERT_TRUE(vec3.IsSpherical());
  ASSERT_TRUE(vec12.IsSpherical());
  ASSERT_TRUE(vec21.IsMetric());
  ASSERT_TRUE(vec4.IsMetric());

  EXPECT_TRUE(std::isnan(vec3.Lat()->Radian()));
  EXPECT_TRUE(std::isnan(vec12.Lat()->Radian()));
  EXPECT_TRUE(std::isnan(*vec21.X()));
  EXPECT_TRUE(std::isnan(*vec4.X()));

  EXPECT_TRUE(std::isnan(vec3.Lon()->Radian()));
  EXPECT_TRUE(std::isnan(vec12.Lon()->Radian()));
  EXPECT_TRUE(std::isnan(*vec21.Y()));
  EXPECT_TRUE(std::isnan(*vec4.Y()));

  EXPECT_TRUE(std::isnan(*vec3.Z()));
  EXPECT_TRUE(std::isnan(*vec12.Z()));
  EXPECT_TRUE(std::isnan(*vec21.Z()));
  EXPECT_TRUE(std::isnan(*vec4.Z()));
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, SubtractMetric)
{
  auto vec1{math::CoordinateVector3::Metric(0.1, 0.2, 0.4)};
  auto vec2{math::CoordinateVector3::Metric(1.1, 2.2, 3.4)};

  math::CoordinateVector3 vec3 = vec1;
  vec3 -= vec2;

  EXPECT_EQ(vec1 - vec2, math::CoordinateVector3::Metric(-1.0, -2.0, -3.0));
  EXPECT_EQ(vec3, math::CoordinateVector3::Metric(-1.0, -2.0, -3.0));
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, SubtractSpherical)
{
  auto vec1{math::CoordinateVector3::Spherical(0.1, 0.2, 0.4)};
  auto vec2{math::CoordinateVector3::Spherical(1.1, 2.2, 3.4)};

  math::CoordinateVector3 vec3 = vec1;
  vec3 -= vec2;

  EXPECT_EQ(vec1 - vec2, math::CoordinateVector3::Spherical(-1.0, -2.0, -3.0));
  EXPECT_EQ(vec3, math::CoordinateVector3::Spherical(-1.0, -2.0, -3.0));
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, SubtractMismatch)
{
  auto vec1{math::CoordinateVector3::Spherical(0.1, 0.2, 0.4)};
  auto vec2{math::CoordinateVector3::Metric(1.1, 2.2, 3.4)};

  math::CoordinateVector3 vec3 = vec1;
  vec3 -= vec2;
  math::CoordinateVector3 vec4 = vec2;
  vec4 -= vec1;

  math::CoordinateVector3 vec12 = vec1 - vec2;
  math::CoordinateVector3 vec21 = vec2 - vec1;

  EXPECT_FALSE(vec3.IsMetric());
  EXPECT_FALSE(vec12.IsMetric());
  EXPECT_FALSE(vec4.IsSpherical());
  EXPECT_FALSE(vec21.IsSpherical());
  ASSERT_TRUE(vec3.IsSpherical());
  ASSERT_TRUE(vec12.IsSpherical());
  ASSERT_TRUE(vec21.IsMetric());
  ASSERT_TRUE(vec4.IsMetric());

  EXPECT_TRUE(std::isnan(vec3.Lat()->Radian()));
  EXPECT_TRUE(std::isnan(vec12.Lat()->Radian()));
  EXPECT_TRUE(std::isnan(*vec21.X()));
  EXPECT_TRUE(std::isnan(*vec4.X()));

  EXPECT_TRUE(std::isnan(vec3.Lon()->Radian()));
  EXPECT_TRUE(std::isnan(vec12.Lon()->Radian()));
  EXPECT_TRUE(std::isnan(*vec21.Y()));
  EXPECT_TRUE(std::isnan(*vec4.Y()));

  EXPECT_TRUE(std::isnan(*vec3.Z()));
  EXPECT_TRUE(std::isnan(*vec12.Z()));
  EXPECT_TRUE(std::isnan(*vec21.Z()));
  EXPECT_TRUE(std::isnan(*vec4.Z()));
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, NotEqual)
{
  auto vec1{math::CoordinateVector3::Metric(0.1, 0.2, 0.3)};
  auto vec2{math::CoordinateVector3::Metric(0.2, 0.2, 0.3)};
  auto vec3{math::CoordinateVector3::Metric(0.1, 0.2, 0.3)};
  auto vec4{math::CoordinateVector3::Spherical(0.1, 0.2, 0.3)};
  auto vec5{math::CoordinateVector3::Spherical(0.2, 0.2, 0.3)};
  auto vec6{math::CoordinateVector3::Spherical(0.1, 0.2, 0.3)};

  EXPECT_TRUE(vec1 != vec2);
  EXPECT_FALSE(vec1 != vec3);

  EXPECT_TRUE(vec4 != vec5);
  EXPECT_FALSE(vec4 != vec6);

  EXPECT_TRUE(vec1 != vec4);
  EXPECT_TRUE(vec1 != vec5);
  EXPECT_TRUE(vec1 != vec6);
  EXPECT_TRUE(vec2 != vec4);
  EXPECT_TRUE(vec2 != vec5);
  EXPECT_TRUE(vec2 != vec6);
  EXPECT_TRUE(vec3 != vec4);
  EXPECT_TRUE(vec3 != vec5);
  EXPECT_TRUE(vec3 != vec6);
  EXPECT_TRUE(vec4 != vec1);
  EXPECT_TRUE(vec4 != vec2);
  EXPECT_TRUE(vec4 != vec3);
  EXPECT_TRUE(vec5 != vec1);
  EXPECT_TRUE(vec5 != vec2);
  EXPECT_TRUE(vec5 != vec3);
  EXPECT_TRUE(vec6 != vec1);
  EXPECT_TRUE(vec6 != vec2);
  EXPECT_TRUE(vec6 != vec3);
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, EqualToleranceMetric)
{
  const auto zero = math::CoordinateVector3();
  const auto one = math::CoordinateVector3::Metric(1, 1, 1);
  EXPECT_FALSE(zero.Equal(one, 1e-6, 1e-6));
  EXPECT_FALSE(zero.Equal(one, 1e-3, 1e-3));
  EXPECT_FALSE(zero.Equal(one, 1e-1, 1e-1));
  EXPECT_TRUE(zero.Equal(one, 1, 1));
  EXPECT_TRUE(zero.Equal(one, 1.1, 1.1));
  EXPECT_TRUE(zero.Equal(one, 1.1, 1e-6));
  EXPECT_FALSE(zero.Equal(one, 1e-6, 1.1));
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, EqualToleranceSpherical)
{
  const auto zero = math::CoordinateVector3::Spherical(0, 0, 0);
  const auto one = math::CoordinateVector3::Spherical(1, 1, 1);
  EXPECT_FALSE(zero.Equal(one, 1e-6, 1e-6));
  EXPECT_FALSE(zero.Equal(one, 1e-3, 1e-3));
  EXPECT_FALSE(zero.Equal(one, 1e-1, 1e-1));
  EXPECT_TRUE(zero.Equal(one, 1, 1));
  EXPECT_TRUE(zero.Equal(one, 1.1, 1.1));
  EXPECT_FALSE(zero.Equal(one, 1e-6, 1.1));
  EXPECT_FALSE(zero.Equal(one, 1.1, 1e-6));
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, EqualToleranceMismatch)
{
  const auto zero = math::CoordinateVector3::Spherical(0, 0, 0);
  const auto one = math::CoordinateVector3::Metric(1, 1, 1);
  EXPECT_FALSE(zero.Equal(one, 1e-6, 1e-6));
  EXPECT_FALSE(zero.Equal(one, 1e-3, 1e-3));
  EXPECT_FALSE(zero.Equal(one, 1e-1, 1e-1));
  EXPECT_FALSE(zero.Equal(one, 1, 1));
  EXPECT_FALSE(zero.Equal(one, 1.1, 1.1));
  EXPECT_FALSE(zero.Equal(one, 1e-6, 1.1));
  EXPECT_FALSE(zero.Equal(one, 1.1, 1e-6));
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, Finite)
{
  using namespace gz::math;

  EXPECT_TRUE(math::CoordinateVector3::Metric(0.1, 0.2, 0.3).IsFinite());
  EXPECT_TRUE(math::CoordinateVector3::Spherical(0.1, 0.2, 0.3).IsFinite());
  EXPECT_FALSE(
    math::CoordinateVector3::Metric(NAN_D, NAN_D, NAN_D).IsFinite());
  EXPECT_FALSE(
    math::CoordinateVector3::Spherical(NAN_D, NAN_D, NAN_D).IsFinite());
  EXPECT_FALSE(
    math::CoordinateVector3::Metric(INF_D, INF_D, INF_D).IsFinite());
  EXPECT_FALSE(
    math::CoordinateVector3::Spherical(INF_D, INF_D, INF_D).IsFinite());
  EXPECT_FALSE(math::CoordinateVector3::Metric(INF_D, 0, 0).IsFinite());
  EXPECT_FALSE(math::CoordinateVector3::Spherical(INF_D, 0, 0).IsFinite());
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, NaNMetric)
{
  using namespace gz::math;

  auto nanVec{math::CoordinateVector3::Metric(NAN_D, NAN_D, NAN_D)};
  EXPECT_FALSE(nanVec.IsFinite());
  ASSERT_TRUE(nanVec.X().has_value());
  ASSERT_TRUE(nanVec.Y().has_value());
  ASSERT_TRUE(nanVec.Z().has_value());
  EXPECT_FALSE(nanVec.Lat().has_value());
  EXPECT_FALSE(nanVec.Lon().has_value());
  EXPECT_TRUE(math::isnan(*nanVec.X()));
  EXPECT_TRUE(math::isnan(*nanVec.Y()));
  EXPECT_TRUE(math::isnan(*nanVec.Z()));
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, NaNSpherical)
{
  using namespace gz::math;

  auto nanVec{math::CoordinateVector3::Spherical(NAN_D, NAN_D, NAN_D)};
  EXPECT_FALSE(nanVec.IsFinite());
  ASSERT_TRUE(nanVec.Lat().has_value());
  ASSERT_TRUE(nanVec.Lon().has_value());
  ASSERT_TRUE(nanVec.Z().has_value());
  EXPECT_FALSE(nanVec.X().has_value());
  EXPECT_FALSE(nanVec.X().has_value());
  EXPECT_TRUE(math::isnan(nanVec.Lat()->Radian()));
  EXPECT_TRUE(math::isnan(nanVec.Lon()->Radian()));
  EXPECT_TRUE(math::isnan(*nanVec.Z()));
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, OperatorStreamOutMetric)
{
  auto v{math::CoordinateVector3::Metric(0.1234, 1.234, 2.3456)};
  std::ostringstream stream;
  stream << v;
  EXPECT_EQ(stream.str(), "0.1234 1.234 2.3456");

  stream.str("");
  stream << std::setprecision(2) << v;
  EXPECT_EQ(stream.str(), "0.12 1.2 2.3");

  stream.str("");
  stream << std::setprecision(3) << v;
  EXPECT_EQ(stream.str(), "0.123 1.23 2.35");

  stream.str("");
  stream << std::setprecision(1) << std::fixed << v;
  EXPECT_EQ(stream.str(), "0.1 1.2 2.3");
}

/////////////////////////////////////////////////
TEST(CoordinateVector3Test, OperatorStreamOutSpherical)
{
  auto v{math::CoordinateVector3::Spherical(0.1234, 1.234, 2.3456)};
  std::ostringstream stream;
  stream << v;
  EXPECT_EQ(stream.str(), "7.0703° 70.703° 2.3456");

  stream.str("");
  stream << std::setprecision(2) << v;
  EXPECT_EQ(stream.str(), "7.1° 71° 2.3");

  stream.str("");
  stream << std::setprecision(3) << v;
  EXPECT_EQ(stream.str(), "7.07° 70.7° 2.35");

  stream.str("");
  stream << std::setprecision(1) << std::fixed << v;
  EXPECT_EQ(stream.str(), "7.1° 70.7° 2.3");
}
