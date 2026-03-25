/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include "gz/rendering/Camera.hh"
#include "gz/rendering/GaussianNoisePass.hh"
#include "gz/rendering/RenderPassSystem.hh"
#include "gz/rendering/RenderTarget.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class RenderTargetTest : public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(RenderTargetTest, RenderTexture)
{
  ScenePtr scene = engine->CreateScene("scene");

  RenderTexturePtr renderTexture = scene->CreateRenderTexture();

  // default properties
  EXPECT_EQ(scene->BackgroundColor(), renderTexture->BackgroundColor());
  // test basic properties
  EXPECT_EQ(0u, renderTexture->GLId());
  renderTexture->SetFormat(PF_R8G8B8);
  renderTexture->SetWidth(800u);
  renderTexture->SetHeight(600u);

  EXPECT_EQ(PF_R8G8B8, renderTexture->Format());
  EXPECT_EQ(800u, renderTexture->Width());
  EXPECT_EQ(600u, renderTexture->Height());

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(RenderTargetTest, RenderWindow)
{
  CHECK_SUPPORTED_ENGINE("ogre");

  ScenePtr scene = engine->CreateScene("scene");

  CameraPtr camera =  scene->CreateCamera("camera");

  RenderWindowPtr renderWindow = camera->CreateRenderWindow();

  renderWindow->SetWidth(320);
  renderWindow->SetHeight(240);
  renderWindow->SetHandle("windowId");
  renderWindow->SetDevicePixelRatio(1.0);

  EXPECT_EQ(320u, renderWindow->Width());
  EXPECT_EQ(240u, renderWindow->Height());
  EXPECT_EQ("windowId", renderWindow->Handle());
  EXPECT_DOUBLE_EQ(1.0, renderWindow->DevicePixelRatio());
  EXPECT_EQ(math::Color::Black, renderWindow->BackgroundColor());

  // change render window properties and verify
  renderWindow->SetWidth(640);
  renderWindow->SetHeight(480);
  scene->SetBackgroundColor(math::Color::Red);

  EXPECT_EQ(640u, renderWindow->Width());
  EXPECT_EQ(480u, renderWindow->Height());
  EXPECT_EQ(math::Color::Red, renderWindow->BackgroundColor());

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(RenderTargetTest, AddRemoveRenderPass)
{
  CHECK_RENDERPASS_SUPPORTED();
  CHECK_SUPPORTED_ENGINE("ogre");

  ScenePtr scene = engine->CreateScene("scene");

  // create a render texture and verify no render pass exists
  RenderTexturePtr renderTexture = scene->CreateRenderTexture();
  EXPECT_EQ(0u, renderTexture->RenderPassCount());

  // get the render pass system
  RenderPassSystemPtr rpSystem = engine->RenderPassSystem();
  RenderPassPtr pass1 = rpSystem->Create<GaussianNoisePass>();
  EXPECT_NE(nullptr, pass1);

  // test adding a render pass
  renderTexture->AddRenderPass(pass1);
  EXPECT_EQ(1u, renderTexture->RenderPassCount());
  EXPECT_EQ(pass1, renderTexture->RenderPassByIndex(0u));

  // test adding another render pass
  RenderPassPtr pass2 = rpSystem->Create<GaussianNoisePass>();
  EXPECT_NE(nullptr, pass2);
  renderTexture->AddRenderPass(pass2);
  EXPECT_EQ(2u, renderTexture->RenderPassCount());
  EXPECT_EQ(pass1, renderTexture->RenderPassByIndex(0u));
  EXPECT_EQ(pass2, renderTexture->RenderPassByIndex(1u));

  // test removing render pass
  renderTexture->RemoveRenderPass(pass1);
  EXPECT_EQ(1u, renderTexture->RenderPassCount());
  EXPECT_EQ(pass2, renderTexture->RenderPassByIndex(0u));

  // Clean up
  engine->DestroyScene(scene);
}
