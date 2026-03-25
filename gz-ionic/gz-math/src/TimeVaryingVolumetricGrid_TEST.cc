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
#include <gz/math/TimeVaryingVolumetricGrid.hh>
#include <gtest/gtest.h>
using namespace gz;
using namespace math;
/////////////////////////////////////////////////
TEST(TimeVaryingVolumetricGridTest, TestConstruction)
{
  InMemoryTimeVaryingVolumetricGridFactory<double, double> gridFactory;

  for (double t = 0; t <= 1; t+=0.2)
  {
    for (double x = 0; x < 1; x+=0.5)
    {
      for (double y = 0; y < 1; y+=0.5)
      {
        for (double z = 0; z < 1; z+=0.5)
        {
          gridFactory.AddPoint(t, Vector3d{x, y, z}, t);
        }
      }
    }
  }

  auto grid = gridFactory.Build();
  auto session = grid.CreateSession();

  // Check validity
  ASSERT_TRUE(grid.IsValid(session));

  // Check stepping
  auto val = grid.LookUp(session, Vector3d{0.5, 0.5, 0.5});
  ASSERT_TRUE(val.has_value());
  ASSERT_EQ(val.value(), 0);

  // Handle new sessions
  auto new_sess = grid.StepTo(session, 0.5);
  ASSERT_TRUE(new_sess.has_value());
  val = grid.LookUp(new_sess.value(), Vector3d{0.5, 0.5, 0.5});
  ASSERT_TRUE(val.has_value());
  ASSERT_EQ(val.value(), 0.5);

  // Check boundary case
  new_sess = grid.StepTo(new_sess.value(), 1);
  ASSERT_TRUE(new_sess.has_value());
  val = grid.LookUp(new_sess.value(), Vector3d{0.5, 0.5, 0.5});
  ASSERT_TRUE(val.has_value());
  ASSERT_EQ(val.value(), 1);

  // Check out of bounds case
  val = grid.LookUp(new_sess.value(), Vector3d{2.5, 2.5, 2.5});
  ASSERT_FALSE(val.has_value());
}

/////////////////////////////////////////////////
TEST(TimeVaryingVolumetricGridTest, TestBounds)
{
  InMemoryTimeVaryingVolumetricGridFactory<double, double> gridFactory;

  for (double t = 0; t <= 1; t+=0.2)
  {
    for (double x = -1; x <= 1; x+=0.5)
    {
      for (double y = -1; y <= 1; y+=0.5)
      {
        for (double z = -1; z <= 1; z+=0.5)
        {
          gridFactory.AddPoint(t, Vector3d{x, y, z}, t);
        }
      }
    }
  }

  auto grid = gridFactory.Build();
  auto session = grid.CreateSession();

  auto [min, max] = grid.Bounds(session);

  ASSERT_NEAR(min.X(), -1.0, 1e-6);
  ASSERT_NEAR(min.Y(), -1.0, 1e-6);
  ASSERT_NEAR(min.Z(), -1.0, 1e-6);

  ASSERT_NEAR(max.X(), 1.0, 1e-6);
  ASSERT_NEAR(max.Y(), 1.0, 1e-6);
  ASSERT_NEAR(max.Z(), 1.0, 1e-6);
}

/////////////////////////////////////////////////
TEST(TimeVaryingVolumetricGridTest, TestEmptyGrid)
{
  InMemoryTimeVaryingVolumetricGridFactory<double, double> gridFactory;

  auto grid = gridFactory.Build();
  auto session = grid.CreateSession();

  auto [min, max] = grid.Bounds(session);

  ASSERT_NEAR(min.X(), 0, 1e-6);
  ASSERT_NEAR(min.Y(), 0, 1e-6);
  ASSERT_NEAR(min.Z(), 0, 1e-6);

  ASSERT_NEAR(max.X(), 0, 1e-6);
  ASSERT_NEAR(max.Y(), 0, 1e-6);
  ASSERT_NEAR(max.Z(), 0, 1e-6);

  auto invalid_session = grid.CreateSession(500);
  // Check validity
  ASSERT_FALSE(grid.IsValid(invalid_session));
}
