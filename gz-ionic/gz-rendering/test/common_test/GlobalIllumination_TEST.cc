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
#include <string>

#include "CommonRenderingTest.hh"

#include "gz/rendering/Camera.hh"
#include "gz/rendering/GlobalIlluminationVct.hh"
#include "gz/rendering/GlobalIlluminationCiVct.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class GlobalIlluminationTest : public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(GlobalIlluminationTest, GlobalIlluminationVct)
{
#ifdef __APPLE__
  GTEST_SKIP() << "Unsupported on apple.";
#endif

  // \todo(anyone) test fails on github action but pass on other
  // ubuntu jenkins CI. Need to investigate further.
  // Github action sets the MESA_GL_VERSION_OVERRIDE variable
  // so check for this variable and disable test if it is set.
#ifdef __linux__
  std::string value;
  bool result = common::env("MESA_GL_VERSION_OVERRIDE", value, true);
  if (result && value == "3.3")
  {
    GTEST_SKIP() << "Test is run on machine with software rendering or mesa "
                 << "driver. Skipping test. " << std::endl;
  }
#endif


  CHECK_SUPPORTED_ENGINE("ogre2");

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  // create directional light
  DirectionalLightPtr light = scene->CreateDirectionalLight();
  ASSERT_NE(nullptr, light);
  light->SetDirection(0.5, 0.5, -1);
  light->SetDiffuseColor(0.8, 0.7, 0.6);
  light->SetSpecularColor(0.3, 0.3, 0.3);
  light->SetCastShadows(true);
  root->AddChild(light);

  auto gi = scene->CreateGlobalIlluminationVct();
  const uint32_t resolution[3]{ 128u, 128u, 32u };
  const uint32_t octantCount[3]{ 4u, 4u, 2u };
  gi->SetResolution(resolution);
  gi->SetOctantCount(octantCount);
  gi->SetAnisotropic(true);
  gi->SetHighQuality(false);
  gi->SetConserveMemory(true);
  gi->SetThinWallCounter(1.0f);
  gi->Build();

  EXPECT_EQ(128u, gi->Resolution()[0]);
  EXPECT_EQ(128u, gi->Resolution()[1]);
  EXPECT_EQ(32u, gi->Resolution()[2]);
  EXPECT_EQ(4u, gi->OctantCount()[0]);
  EXPECT_EQ(4u, gi->OctantCount()[1]);
  EXPECT_EQ(2u, gi->OctantCount()[2]);
  EXPECT_TRUE(gi->Anisotropic());
  EXPECT_FALSE(gi->HighQuality());
  EXPECT_TRUE(gi->ConserveMemory());
  EXPECT_FLOAT_EQ(1.0f, gi->ThinWallCounter());

  EXPECT_FALSE(gi->Enabled());
  scene->SetActiveGlobalIllumination(gi);
  EXPECT_TRUE(gi->Enabled());

  EXPECT_EQ(GlobalIlluminationVct::DVM_None, gi->DebugVisualization());
  gi->SetDebugVisualization(GlobalIlluminationVct::DVM_Albedo);
  EXPECT_EQ(GlobalIlluminationVct::DVM_Albedo, gi->DebugVisualization());
  gi->SetDebugVisualization(GlobalIlluminationVct::DVM_Normal);
  EXPECT_EQ(GlobalIlluminationVct::DVM_Normal, gi->DebugVisualization());
  gi->SetDebugVisualization(GlobalIlluminationVct::DVM_Emissive);
  EXPECT_EQ(GlobalIlluminationVct::DVM_Emissive, gi->DebugVisualization());
  gi->SetDebugVisualization(GlobalIlluminationVct::DVM_Lighting);
  EXPECT_EQ(GlobalIlluminationVct::DVM_Lighting, gi->DebugVisualization());

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(GlobalIlluminationTest, GlobalIlluminationCiVct)
{
#ifdef __APPLE__
  GTEST_SKIP() << "Unsupported on apple.";
#endif

  // \todo(anyone) test fails on github action but pass on other
  // ubuntu jenkins CI. Need to investigate further.
  // Github action sets the MESA_GL_VERSION_OVERRIDE variable
  // so check for this variable and disable test if it is set.
#ifdef __linux__
  std::string value;
  bool result = common::env("MESA_GL_VERSION_OVERRIDE", value, true);
  if (result && value == "3.3")
  {
    GTEST_SKIP() << "Test is run on machine with software rendering or mesa "
                 << "driver. Skipping test. " << std::endl;
  }
#endif

  CHECK_SUPPORTED_ENGINE("ogre2");

  if (engine->GraphicsAPI() != GraphicsAPI::VULKAN)
  {
    GTEST_SKIP() << "Global Illumination CI VCT method currently only works "
                 << "with vulkan backend" << std::endl;
  }

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  // create camera
  CameraPtr camera = scene->CreateCamera("camera");
  ASSERT_NE(nullptr, camera);

  camera->SetLocalPosition(0.0, 0.0, 0.0);
  camera->SetLocalRotation(0.0, 0.0, 0.0);
  camera->SetImageWidth(1600);
  camera->SetImageHeight(900);
  camera->SetAntiAliasing(2);
  camera->SetAspectRatio(1.333);
  camera->SetHFOV(GZ_PI / 2);
  root->AddChild(camera);

  // create directional light
  DirectionalLightPtr light = scene->CreateDirectionalLight();
  ASSERT_NE(nullptr, light);
  light->SetDirection(0.5, 0.5, -1);
  light->SetDiffuseColor(0.8, 0.7, 0.6);
  light->SetSpecularColor(0.3, 0.3, 0.3);
  light->SetCastShadows(true);
  root->AddChild(light);

  auto gi = scene->CreateGlobalIlluminationCiVct();
  EXPECT_NE(nullptr, gi);

  gi->SetMaxCascades(3u);

  CiVctCascadePtr cascade = gi->AddCascade(nullptr);
  const uint32_t resolution[3]{ 128u, 128u, 128u };
  const uint32_t octantCount[3]{ 4u, 4u, 2u };

  math::Vector3d areaHalfSize(5.0, 5.0, 5.0);
  math::Vector3d cameraStepSize(1.0, 1.0, 1.0);

  cascade->SetResolution(resolution);
  cascade->SetOctantCount(octantCount);
  cascade->SetAreaHalfSize(areaHalfSize);
  cascade->SetCameraStepSize(cameraStepSize);
  cascade->SetThinWallCounter(1.0f);
  cascade->SetCorrectAreaLightShadows(true);

  auto cascade2 = gi->AddCascade(cascade.get());
  math::Vector3d areaHalfSize2(10.0, 10.0, 10.0);
  cascade2->SetAreaHalfSize(areaHalfSize2);

  auto cascade3 = gi->AddCascade(cascade2.get());
  math::Vector3d areaHalfSize3(20.0, 20.0, 20.0);
  cascade3->SetAreaHalfSize(areaHalfSize3);

  // check cascades
  EXPECT_EQ(128u, cascade->Resolution()[0]);
  EXPECT_EQ(128u, cascade->Resolution()[1]);
  EXPECT_EQ(128u, cascade->Resolution()[2]);
  EXPECT_EQ(4u, cascade->OctantCount()[0]);
  EXPECT_EQ(4u, cascade->OctantCount()[1]);
  EXPECT_EQ(2u, cascade->OctantCount()[2]);
  EXPECT_EQ(areaHalfSize, cascade->AreaHalfSize());
  EXPECT_EQ(cameraStepSize, cascade->CameraStepSize());
  EXPECT_FLOAT_EQ(1.0f, cascade->ThinWallCounter());
  EXPECT_TRUE(cascade->CorrectAreaLightShadows());

  EXPECT_EQ(128u, cascade2->Resolution()[0]);
  EXPECT_EQ(128u, cascade2->Resolution()[1]);
  EXPECT_EQ(128u, cascade2->Resolution()[2]);
  EXPECT_EQ(4u, cascade2->OctantCount()[0]);
  EXPECT_EQ(4u, cascade2->OctantCount()[1]);
  EXPECT_EQ(2u, cascade2->OctantCount()[2]);
  EXPECT_EQ(areaHalfSize2, cascade2->AreaHalfSize());
  EXPECT_EQ(cameraStepSize, cascade2->CameraStepSize());
  EXPECT_FLOAT_EQ(1.0f, cascade2->ThinWallCounter());
  EXPECT_TRUE(cascade2->CorrectAreaLightShadows());

  EXPECT_EQ(128u, cascade3->Resolution()[0]);
  EXPECT_EQ(128u, cascade3->Resolution()[1]);
  EXPECT_EQ(128u, cascade3->Resolution()[2]);
  EXPECT_EQ(4u, cascade3->OctantCount()[0]);
  EXPECT_EQ(4u, cascade3->OctantCount()[1]);
  EXPECT_EQ(2u, cascade3->OctantCount()[2]);
  EXPECT_EQ(areaHalfSize3, cascade3->AreaHalfSize());
  EXPECT_EQ(cameraStepSize, cascade3->CameraStepSize());
  EXPECT_FLOAT_EQ(1.0f, cascade3->ThinWallCounter());
  EXPECT_TRUE(cascade3->CorrectAreaLightShadows());

  gi->PopCascade();
  cascade3 = gi->AddCascade(cascade2.get());
  cascade3->SetAreaHalfSize(areaHalfSize3);

  EXPECT_EQ(128u, cascade3->Resolution()[0]);
  EXPECT_EQ(128u, cascade3->Resolution()[1]);
  EXPECT_EQ(128u, cascade3->Resolution()[2]);
  EXPECT_EQ(4u, cascade3->OctantCount()[0]);
  EXPECT_EQ(4u, cascade3->OctantCount()[1]);
  EXPECT_EQ(2u, cascade3->OctantCount()[2]);
  EXPECT_EQ(areaHalfSize3, cascade3->AreaHalfSize());
  EXPECT_EQ(cameraStepSize, cascade3->CameraStepSize());
  EXPECT_FLOAT_EQ(1.0f, cascade3->ThinWallCounter());
  EXPECT_TRUE(cascade3->CorrectAreaLightShadows());

  gi->AutoCalculateStepSizes(gz::math::Vector3d(3.0, 3.0, 3.0));
  gi->SetConsistentCascadeSteps(true);
  gi->SetHighQuality(false);

  gi->Bind(camera);
  gi->Start(2u, true);
  gi->Build();

  EXPECT_FALSE(gi->Enabled());
  scene->SetActiveGlobalIllumination(gi);
  EXPECT_TRUE(gi->Enabled());

  EXPECT_TRUE(gi->Started());
  EXPECT_FALSE(gi->HighQuality());
  EXPECT_TRUE(gi->ConsistentCascadeSteps());
  EXPECT_TRUE(gi->Anisotropic());
  EXPECT_EQ(2u, gi->BounceCount());

  gi->NewSettings(3u, false);
  EXPECT_FALSE(gi->Anisotropic());
  EXPECT_EQ(3u, gi->BounceCount());

  EXPECT_EQ(GlobalIlluminationCiVct::DVM_None, gi->DebugVisualization());
  gi->SetDebugVisualization(GlobalIlluminationCiVct::DVM_Albedo);
  EXPECT_EQ(GlobalIlluminationCiVct::DVM_Albedo, gi->DebugVisualization());
  gi->SetDebugVisualization(GlobalIlluminationCiVct::DVM_Normal);
  EXPECT_EQ(GlobalIlluminationCiVct::DVM_Normal, gi->DebugVisualization());
  gi->SetDebugVisualization(GlobalIlluminationCiVct::DVM_Emissive);
  EXPECT_EQ(GlobalIlluminationCiVct::DVM_Emissive, gi->DebugVisualization());
  gi->SetDebugVisualization(GlobalIlluminationCiVct::DVM_Lighting);
  EXPECT_EQ(GlobalIlluminationCiVct::DVM_Lighting, gi->DebugVisualization());

  // Clean up
  engine->DestroyScene(scene);
}
