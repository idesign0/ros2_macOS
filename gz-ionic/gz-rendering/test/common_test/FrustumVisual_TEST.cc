/*
 * Copyright (C) 2025 Open Source Robotics Foundation
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

#include "gz/rendering/FrustumVisual.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

/// \brief The test fixture.
class FrustumVisualTest : public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(FrustumVisualTest, FrustumVisual)
{
  CHECK_SUPPORTED_ENGINE("ogre2");

  ScenePtr scene = engine->CreateScene("scene");
  EXPECT_NE(nullptr, scene);

  // FrustumVisual and can only be accessed by the scene extension API
  // in gz-rendering9
  // \todo(iche033) Remove this in gz-rendering10
  if (!scene->Extension())
    return;

  // Create a frustum visual
  // \todo(iche033) uncomment and use official API in gz-rendering10
  // FrustumVisualPtr frustumVisual = scene->CreateFrustumVisual();
  FrustumVisualPtr frustumVisual = std::dynamic_pointer_cast<FrustumVisual>(
      scene->Extension()->CreateExt("frustum_visual"));

  // Check default properties
  EXPECT_DOUBLE_EQ(0, frustumVisual->NearClipPlane());
  EXPECT_DOUBLE_EQ(1.0, frustumVisual->FarClipPlane());
  EXPECT_EQ(math::Angle(GZ_DTOR(45)), frustumVisual->HFOV().Radian());
  EXPECT_DOUBLE_EQ(1.0, frustumVisual->AspectRatio());
  auto emptyPlane = math::Planed();
  EXPECT_DOUBLE_EQ(emptyPlane.Offset(),
                   frustumVisual->Plane(FRUSTUM_PLANE_NEAR).Offset());
  EXPECT_EQ(emptyPlane.Normal(),
            frustumVisual->Plane(FRUSTUM_PLANE_NEAR).Normal());
  EXPECT_DOUBLE_EQ(emptyPlane.Offset(),
                   frustumVisual->Plane(FRUSTUM_PLANE_FAR).Offset());
  EXPECT_EQ(emptyPlane.Normal(),
            frustumVisual->Plane(FRUSTUM_PLANE_FAR).Normal());
  EXPECT_DOUBLE_EQ(emptyPlane.Offset(),
                   frustumVisual->Plane(FRUSTUM_PLANE_LEFT).Offset());
  EXPECT_EQ(emptyPlane.Normal(),
            frustumVisual->Plane(FRUSTUM_PLANE_LEFT).Normal());
  EXPECT_DOUBLE_EQ(emptyPlane.Offset(),
                   frustumVisual->Plane(FRUSTUM_PLANE_RIGHT).Offset());
  EXPECT_EQ(emptyPlane.Normal(),
            frustumVisual->Plane(FRUSTUM_PLANE_RIGHT).Normal());
  EXPECT_DOUBLE_EQ(emptyPlane.Offset(),
                   frustumVisual->Plane(FRUSTUM_PLANE_TOP).Offset());
  EXPECT_EQ(emptyPlane.Normal(),
            frustumVisual->Plane(FRUSTUM_PLANE_TOP).Normal());
  EXPECT_DOUBLE_EQ(emptyPlane.Offset(),
                   frustumVisual->Plane(FRUSTUM_PLANE_BOTTOM).Offset());
  EXPECT_EQ(emptyPlane.Normal(),
            frustumVisual->Plane(FRUSTUM_PLANE_BOTTOM).Normal());

  // Test APIs
  double nearClip = 1.1;
  double farClip = 15.5;
  math::Angle hfov(1.06);
  double aspect = 1.3333;

  frustumVisual->SetNearClipPlane(nearClip);
  EXPECT_DOUBLE_EQ(nearClip, frustumVisual->NearClipPlane());

  frustumVisual->SetFarClipPlane(farClip);
  EXPECT_DOUBLE_EQ(farClip, frustumVisual->FarClipPlane());

  frustumVisual->SetHFOV(hfov);
  EXPECT_EQ(hfov, frustumVisual->HFOV());

  frustumVisual->SetAspectRatio(aspect);
  EXPECT_EQ(aspect, frustumVisual->AspectRatio());

  frustumVisual->Update();

  EXPECT_DOUBLE_EQ(nearClip,
      std::fabs(frustumVisual->Plane(FRUSTUM_PLANE_NEAR).Offset()));
  EXPECT_EQ(math::Vector3d::UnitX,
      frustumVisual->Plane(FRUSTUM_PLANE_NEAR).Normal());
  EXPECT_DOUBLE_EQ(farClip,
      std::fabs(frustumVisual->Plane(FRUSTUM_PLANE_FAR).Offset()));
  EXPECT_EQ(-math::Vector3d::UnitX,
      frustumVisual->Plane(FRUSTUM_PLANE_FAR).Normal());
  EXPECT_NEAR(0.0, frustumVisual->Plane(FRUSTUM_PLANE_LEFT).Offset(), 1e-6);
  EXPECT_GT(0.0, frustumVisual->Plane(FRUSTUM_PLANE_LEFT).Normal().Y());
  EXPECT_NEAR(0.0, frustumVisual->Plane(FRUSTUM_PLANE_RIGHT).Offset(), 1e-6);
  EXPECT_LT(0.0, frustumVisual->Plane(FRUSTUM_PLANE_RIGHT).Normal().Y());
  EXPECT_NEAR(0.0, frustumVisual->Plane(FRUSTUM_PLANE_TOP).Offset(), 1e-6);
  EXPECT_GT(0.0, frustumVisual->Plane(FRUSTUM_PLANE_TOP).Normal().Z());
  EXPECT_NEAR(0.0, frustumVisual->Plane(FRUSTUM_PLANE_BOTTOM).Offset(), 1e-6);
  EXPECT_LT(0.0, frustumVisual->Plane(FRUSTUM_PLANE_BOTTOM).Normal().Z());

  engine->DestroyScene(scene);
}
