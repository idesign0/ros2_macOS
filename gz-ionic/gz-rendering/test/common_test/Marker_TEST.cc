/* * Copyright (C) 2019 Open Source Robotics Foundation
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

#include "CommonRenderingTest.hh"

#include "gz/rendering/Marker.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;
using namespace std::chrono_literals;

class MarkerTest : public CommonRenderingTest 
{
};

/////////////////////////////////////////////////
TEST_F(MarkerTest, Marker)
{
  CHECK_UNSUPPORTED_ENGINE("optix");

  ScenePtr scene = engine->CreateScene("scene");

  MarkerPtr marker = scene->CreateMarker();
  ASSERT_NE(nullptr, marker);

  // initial values
  EXPECT_EQ(std::chrono::steady_clock::duration::zero().count(),
      marker->Lifetime().count());
  EXPECT_EQ(0, marker->Layer());
  EXPECT_EQ(MarkerType::MT_NONE, marker->Type());

  // lifetime
  std::chrono::steady_clock::duration duration{2ms};
  marker->SetLifetime(duration);
  EXPECT_EQ(duration.count(), marker->Lifetime().count());

  // layer
  marker->SetLayer(1);
  EXPECT_EQ(1, marker->Layer());
  marker->SetLayer(-2);
  EXPECT_EQ(-2, marker->Layer());

  // type
  marker->SetType(MarkerType::MT_CAPSULE);
  EXPECT_EQ(MarkerType::MT_CAPSULE, marker->Type());

  marker->SetType(MarkerType::MT_CYLINDER);
  EXPECT_EQ(MarkerType::MT_CYLINDER, marker->Type());

  marker->SetType(MarkerType::MT_NONE);
  EXPECT_EQ(MarkerType::MT_NONE, marker->Type());

  marker->SetType(MarkerType::MT_POINTS);
  EXPECT_EQ(MarkerType::MT_POINTS, marker->Type());

  marker->SetType(MarkerType::MT_LINE_STRIP);
  EXPECT_EQ(MarkerType::MT_LINE_STRIP, marker->Type());

  marker->SetType(MarkerType::MT_LINE_LIST);
  EXPECT_EQ(MarkerType::MT_LINE_LIST, marker->Type());

  marker->SetType(MarkerType::MT_TRIANGLE_STRIP);
  EXPECT_EQ(MarkerType::MT_TRIANGLE_STRIP, marker->Type());

  marker->SetType(MarkerType::MT_TRIANGLE_LIST);
  EXPECT_EQ(MarkerType::MT_TRIANGLE_LIST, marker->Type());

  marker->SetType(MarkerType::MT_TRIANGLE_FAN);
  EXPECT_EQ(MarkerType::MT_TRIANGLE_FAN, marker->Type());

  // exercise point api
  EXPECT_NO_THROW(marker->AddPoint(math::Vector3d(0, 1, 2),
      math::Color::White));
  EXPECT_NO_THROW(marker->SetPoint(0, math::Vector3d(3, 1, 2)));
  EXPECT_NO_THROW(marker->ClearPoints());

  EXPECT_DOUBLE_EQ(1.0, marker->Size());
  marker->SetSize(3.0);
  EXPECT_DOUBLE_EQ(3.0, marker->Size());

  // Clean up
  engine->DestroyScene(scene);
}
