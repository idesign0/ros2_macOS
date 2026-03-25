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

#include <gtest/gtest.h>
#include <cmath>

#include "CommonRenderingTest.hh"

#include "gz/rendering/LensFlarePass.hh"
#include "gz/rendering/RenderPassSystem.hh"

using namespace gz;
using namespace rendering;

class LensFlarePassTest : public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(LensFlarePassTest, LensFlare)
{
  CHECK_SUPPORTED_ENGINE("ogre2");
  CHECK_RENDERPASS_SUPPORTED();

  // get the render pass system
  RenderPassSystemPtr rpSystem = engine->RenderPassSystem();
  RenderPassPtr pass = rpSystem->Create<LensFlarePass>();
  LensFlarePassPtr lensFlarePass =
    std::dynamic_pointer_cast<LensFlarePass>(pass);
  EXPECT_NE(nullptr, lensFlarePass);

  // verify initial values
  EXPECT_DOUBLE_EQ(1.0, lensFlarePass->Scale());
  EXPECT_EQ(10u, lensFlarePass->OcclusionSteps());
  EXPECT_EQ(math::Vector3d(1.0, 1.0, 1.0), lensFlarePass->Color());
  EXPECT_EQ(true, lensFlarePass->WideAngleCameraAfterStitching());

  // scale
  const double scale = 0.23;
  lensFlarePass->SetScale(scale);
  EXPECT_DOUBLE_EQ(scale, lensFlarePass->Scale());

  // occlusion steps
  const uint32_t occlusionSteps = 25u;
  lensFlarePass->SetOcclusionSteps(occlusionSteps);
  EXPECT_DOUBLE_EQ(occlusionSteps, lensFlarePass->OcclusionSteps());

  // color
  const math::Vector3d color = math::Vector3d(0.7, 0.4, 0.12);
  lensFlarePass->SetColor(color);
  EXPECT_EQ(color, lensFlarePass->Color());

  lensFlarePass->SetWideAngleCameraAfterStitching(false);
  EXPECT_EQ(false, lensFlarePass->WideAngleCameraAfterStitching());
  lensFlarePass->SetWideAngleCameraAfterStitching(true);
  EXPECT_EQ(true, lensFlarePass->WideAngleCameraAfterStitching());
}
