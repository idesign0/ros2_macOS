/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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

#include "gz/rendering/GaussianNoisePass.hh"
#include "gz/rendering/RenderPassSystem.hh"

using namespace gz;
using namespace rendering;

class RenderPassSystemTest : public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(RenderPassSystemTest, RenderPassSystem)
{
  CHECK_RENDERPASS_SUPPORTED();

  // get the render pass system
  RenderPassSystemPtr rpSystem = engine->RenderPassSystem();

  // test creating abstract render pass
  RenderPassPtr nullPass = rpSystem->Create<RenderPass>();
  EXPECT_EQ(nullptr, nullPass);

  // test passing invalid class type
  RenderPassPtr invalidPass = rpSystem->Create<RenderEngine>();
  EXPECT_EQ(nullptr, invalidPass);

  // create valid gaussian noise pass
  RenderPassPtr pass = rpSystem->Create<GaussianNoisePass>();
  EXPECT_NE(nullptr, pass);
  GaussianNoisePassPtr noisePass =
      std::dynamic_pointer_cast<GaussianNoisePass>(pass);
  EXPECT_NE(nullptr, noisePass);
}
