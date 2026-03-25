/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#include <gz/math/TimeVaryingVolumetricGridLookupField.hh>
#include <gtest/gtest.h>
using namespace gz;
using namespace math;
/////////////////////////////////////////////////
TEST(TimeVaryingVolumetricLookupFieldTest, TestConstruction)
{
  std::vector<Vector3d> cloud;
  cloud.emplace_back(0, 0, 0);
  cloud.emplace_back(0, 0, 1);
  cloud.emplace_back(0, 1, 0);
  cloud.emplace_back(0, 1, 1);
  cloud.emplace_back(1, 0, 0);
  cloud.emplace_back(1, 0, 1);
  cloud.emplace_back(1, 1, 0);
  cloud.emplace_back(1, 1, 1);

  std::vector<double> valuesTime0{0, 0, 0, 0, 0, 0, 0, 0};
  std::vector<double> valuesTime1{1, 1, 1, 1, 1, 1, 1, 1};

  VolumetricGridLookupField<double> scalarIndex(cloud);
  TimeVaryingVolumetricGridLookupField<
    double, double, InMemorySession<double, double>> timeVaryingField;

  timeVaryingField.AddVolumetricGridField(0, scalarIndex);
  timeVaryingField.AddVolumetricGridField(1, scalarIndex);

  {
    // Get data at T=0
    auto session = timeVaryingField.CreateSession();

    // Check session validity
    ASSERT_TRUE(timeVaryingField.IsValid(session));

    auto points = timeVaryingField.LookUp(session, Vector3d{0.5, 0.5, 0.5});
    ASSERT_EQ(points.size(), 2);
    ASSERT_EQ(points[0].time, 0);
    ASSERT_EQ(points[1].time, 1);
    auto res = timeVaryingField.EstimateQuadrilinear<double>(
      session,
      points,
      valuesTime0,
      valuesTime1,
      Vector3d{0.5, 0.5, 0.5}, -1);
    ASSERT_EQ(res, 0);

    // Step session
    auto next_session = timeVaryingField.StepTo(session, 0.5);

    ASSERT_TRUE(next_session.has_value());
    ASSERT_EQ(next_session->time, 0.5);

    points = timeVaryingField.LookUp(
      next_session.value(), Vector3d{0.5, 0.5, 0.5});
    ASSERT_EQ(points.size(), 2);
    ASSERT_EQ(points[0].time, 0);
    ASSERT_EQ(points[1].time, 1);

    res = timeVaryingField.EstimateQuadrilinear<double>(
      next_session.value(),
      points,
      valuesTime0,
      valuesTime1,
      Vector3d{0.5, 0.5, 0.5}, -1);
    ASSERT_EQ(res, 0.5);

    // Step session
    next_session = timeVaryingField.StepTo(next_session.value(), 1);
    ASSERT_TRUE(next_session.has_value());
    ASSERT_EQ(next_session->time, 1);

    points = timeVaryingField.LookUp(next_session.value(),
      Vector3d{0.5, 0.5, 0.5});
    ASSERT_EQ(points[0].time, 1);
    ASSERT_EQ(points.size(), 1);

    res = timeVaryingField.EstimateQuadrilinear<double>(
      next_session.value(),
      points,
      valuesTime0,
      valuesTime1,
      Vector3d{0.5, 0.5, 0.5}, -1);
    ASSERT_EQ(res, 1);

    // Out of time steps
    next_session = timeVaryingField.StepTo(next_session.value(), 2);
    ASSERT_FALSE(next_session.has_value());

    // Create invalid session
    auto invalid_session = timeVaryingField.CreateSession(500);

    // Check session validity
    ASSERT_FALSE(timeVaryingField.IsValid(invalid_session));

    points = timeVaryingField.LookUp(
      invalid_session, Vector3d{0.5, 0.5, 0.5});
    ASSERT_EQ(points.size(), 0);
    auto result = timeVaryingField.EstimateQuadrilinear<double>(
      invalid_session,
      points,
      valuesTime0,
      valuesTime1,
      Vector3d{0.5, 0.5, 0.5}, -1);
    ASSERT_FALSE(result.has_value());

    // Try stepping invalid session should return nullopt
    ASSERT_FALSE(timeVaryingField.StepTo(invalid_session, 8000).has_value());

    // No query points
    auto valid_session = timeVaryingField.CreateSession(0.5);
    res = timeVaryingField.EstimateQuadrilinear<double>(
      valid_session,
      points,
      valuesTime0,
      valuesTime1,
      Vector3d{0.5, 0.5, 0.5}, -1);
    ASSERT_FALSE(res.has_value());

    // Only upper half has valid query points, estimate using upper half
    valid_session = timeVaryingField.CreateSession();
    next_session = timeVaryingField.StepTo(valid_session, 0.5);
    ASSERT_TRUE(next_session.has_value());
    valid_session = next_session.value();
    points = timeVaryingField.LookUp(
      valid_session, Vector3d{0.5, 0.5, 0.5});
    ASSERT_EQ(points.size(), 2);
    points[0].timeSlice.clear();
    res = timeVaryingField.EstimateQuadrilinear<double>(
      valid_session,
      points,
      valuesTime0,
      valuesTime1,
      Vector3d{0.5, 0.5, 0.5}, -1);
    ASSERT_EQ(res, 1);

    // Only lower half has valid query points, estimate using upper half
    points = timeVaryingField.LookUp(
      valid_session, Vector3d{0.5, 0.5, 0.5});
    ASSERT_EQ(points.size(), 2);
    points[1].timeSlice.clear();
    res = timeVaryingField.EstimateQuadrilinear<double>(
      valid_session,
      points,
      valuesTime0,
      valuesTime1,
      Vector3d{0.5, 0.5, 0.5}, -1);
    ASSERT_EQ(res, 0);

    // Both have no data
    points = timeVaryingField.LookUp(
      valid_session, Vector3d{0.5, 0.5, 0.5});
    ASSERT_EQ(points.size(), 2);
    points[0].timeSlice.clear();
    points[1].timeSlice.clear();
    res = timeVaryingField.EstimateQuadrilinear<double>(
      valid_session,
      points,
      valuesTime0,
      valuesTime1,
      Vector3d{0.5, 0.5, 0.5}, -1);
    ASSERT_FALSE(res.has_value());

    // Out of spatial range.
    points = timeVaryingField.LookUp(
      valid_session, Vector3d{0.5, 0.5, 0.5});
    res = timeVaryingField.EstimateQuadrilinear<double>(
      valid_session,
      points,
      valuesTime0,
      valuesTime1,
      Vector3d{2.5, 2.5, 2.5}, -1);
    ASSERT_FALSE(res.has_value());

    // Good case.
    points = timeVaryingField.LookUp(
      valid_session, Vector3d{0.5, 0.5, 0.5});
    res = timeVaryingField.EstimateQuadrilinear<double>(
      valid_session,
      points,
      valuesTime0,
      valuesTime1,
      Vector3d{0.5, 0.5, 0.5}, -1);
    ASSERT_EQ(res, 0.5);
  }
}
