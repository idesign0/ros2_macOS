/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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
#include <string>

#include "CommonRenderingTest.hh"

#include <gz/math/Color.hh>

#include "gz/rendering/Projector.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

/// \brief The test fixture.
class ProjectorTest : public CommonRenderingTest
{
  /// \brief A directory under test/ with some textures.
  protected: const std::string TEST_MEDIA_PATH =
      common::joinPaths(std::string(PROJECT_SOURCE_PATH),
      "test", "media", "materials", "textures");
};

/////////////////////////////////////////////////
TEST_F(ProjectorTest, Projector)
{
  ScenePtr scene = engine->CreateScene("scene");
  EXPECT_NE(nullptr, scene);

  // Projector and can only be accessed by the scene extension API
  // in gz-rendering7
  if (!scene->Extension())
    return;

  // Create a projector
  ProjectorPtr projector = scene->CreateProjector();

  // check default properties
  EXPECT_LT(0U, projector->NearClipPlane());
  EXPECT_LT(0U, projector->FarClipPlane());
  EXPECT_LT(0U, projector->HFOV().Radian());
  EXPECT_EQ(GZ_VISIBILITY_ALL, projector->VisibilityFlags());
  EXPECT_TRUE(projector->Texture().empty());
  EXPECT_FALSE(projector->IsEnabled());

  // test APIs
  double nearClip = 1.1;
  double farClip = 15.5;
  math::Angle hfov(2.3);
  uint32_t visibilityFlags = 0x03;
  std::string texture = common::joinPaths(TEST_MEDIA_PATH,
      "blue_texture.png");

  projector->SetNearClipPlane(nearClip);
  EXPECT_DOUBLE_EQ(nearClip, projector->NearClipPlane());

  projector->SetFarClipPlane(farClip);
  EXPECT_DOUBLE_EQ(farClip, projector->FarClipPlane());

  projector->SetHFOV(hfov);
  EXPECT_EQ(hfov, projector->HFOV());

  projector->SetVisibilityFlags(visibilityFlags);
  EXPECT_EQ(visibilityFlags, projector->VisibilityFlags());

  projector->SetTexture(texture);
  EXPECT_EQ(texture, projector->Texture());

  projector->SetEnabled(true);
  EXPECT_TRUE(projector->IsEnabled());

  engine->DestroyScene(scene);
}
