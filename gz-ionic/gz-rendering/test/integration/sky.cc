/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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

#include <gz/common/Image.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Image.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/WideAngleCamera.hh"

#include <gz/utils/ExtraTestMacros.hh>

using namespace gz;
using namespace rendering;

class SkyTest: public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(SkyTest, Sky)
{
  CHECK_SUPPORTED_ENGINE("ogre2");

  // add resources in build dir
  engine->AddResourcePath(
      common::joinPaths(std::string(PROJECT_BUILD_PATH), "src"));

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(0.3, 0.3, 0.3);

  scene->SetBackgroundColor(1.0, 0.0, 0.0);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  // create  camera
  CameraPtr camera = scene->CreateCamera();
  ASSERT_NE(nullptr, camera);
  camera->SetImageWidth(100);
  camera->SetImageHeight(100);
  // look up into the sky
  camera->SetLocalRotation(math::Quaterniond(0, -GZ_PI/2.0, 0));
  root->AddChild(camera);

  // capture original image with red background
  Image image = camera->CreateImage();
  camera->Capture(image);

  // Enable sky
  scene->SetSkyEnabled(true);

  // capture image with sky enabled
  Image imageSky = camera->CreateImage();
  camera->Capture(imageSky);

  // Compare image pixels
  unsigned char *data = image.Data<unsigned char>();
  unsigned char *dataSky = imageSky.Data<unsigned char>();
  unsigned int height = camera->ImageHeight();
  unsigned int width = camera->ImageWidth();
  unsigned int channelCount = PixelUtil::ChannelCount(camera->ImageFormat());
  unsigned int step = width * channelCount;

  unsigned int rSum = 0u;
  unsigned int gSum = 0u;
  unsigned int bSum = 0u;
  unsigned int rSkySum = 0u;
  unsigned int gSkySum = 0u;
  unsigned int bSkySum = 0u;

  for (unsigned int i = 0; i < height; ++i)
  {
    for (unsigned int j = 0; j < step; j += channelCount)
    {
      unsigned int idx = i * step + j;
      rSum += data[idx];
      gSum += data[idx + 1];
      bSum += data[idx + 2];

      rSkySum += dataSky[idx];
      gSkySum += dataSky[idx + 1];
      bSkySum += dataSky[idx + 2];
    }
  }

  // sky disabled - red background
  EXPECT_GT(rSum, 0u);
  EXPECT_EQ(0u, gSum);
  EXPECT_EQ(0u, bSum);

  // sky enabled - blue should be the dominant color
  EXPECT_GT(rSkySum, 0u);
  EXPECT_GT(gSkySum, 0u);
  EXPECT_GT(bSkySum, 0u);
  EXPECT_GT(bSkySum, gSkySum);
  EXPECT_GT(bSkySum, rSkySum);

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(SkyTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(WideAngleCamera))
{
  CHECK_SUPPORTED_ENGINE("ogre2");

  // add resources in build dir
  engine->AddResourcePath(
      common::joinPaths(std::string(PROJECT_BUILD_PATH), "src"));

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(0.3, 0.3, 0.3);

  scene->SetBackgroundColor(1.0, 0.0, 0.0);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  // create  camera
  auto camera = scene->CreateWideAngleCamera("WideAngleCamera");
  ASSERT_NE(nullptr, camera);

  CameraLens lens;
  lens.SetCustomMappingFunction(1.05, 4.0, AFT_TAN, 1.0, 0.0);
  lens.SetType(MFT_CUSTOM);
  lens.SetCutOffAngle(GZ_PI);

  camera->SetLens(lens);
  camera->SetHFOV(2.6);
  camera->SetImageWidth(100);
  camera->SetImageHeight(100);
  camera->SetAspectRatio(1.333);
  camera->SetLocalPosition(0.0, 0.0, 0.0);

  // look up into the sky
  camera->SetLocalRotation(math::Quaterniond(0, -GZ_PI/2.0, 0));
  root->AddChild(camera);

  // capture original image with red background
  Image image = camera->CreateImage();
  camera->Capture(image);

  // Enable sky
  scene->SetSkyEnabled(true);

  // capture image with sky enabled
  Image imageSky = camera->CreateImage();
  camera->Capture(imageSky);

  // Compare image pixels
  unsigned char *data = image.Data<unsigned char>();
  unsigned char *dataSky = imageSky.Data<unsigned char>();
  unsigned int height = camera->ImageHeight();
  unsigned int width = camera->ImageWidth();
  unsigned int channelCount = PixelUtil::ChannelCount(camera->ImageFormat());
  unsigned int step = width * channelCount;

  unsigned int rSum = 0u;
  unsigned int gSum = 0u;
  unsigned int bSum = 0u;
  unsigned int rSkySum = 0u;
  unsigned int gSkySum = 0u;
  unsigned int bSkySum = 0u;

  for (unsigned int i = 0; i < height; ++i)
  {
    for (unsigned int j = 0; j < step; j += channelCount)
    {
      unsigned int idx = i * step + j;
      rSum += data[idx];
      gSum += data[idx + 1];
      bSum += data[idx + 2];

      rSkySum += dataSky[idx];
      gSkySum += dataSky[idx + 1];
      bSkySum += dataSky[idx + 2];
    }
  }

  // sky disabled - red background
  EXPECT_GT(rSum, 0u);
  EXPECT_EQ(0u, gSum);
  EXPECT_EQ(0u, bSum);

  // sky enabled - blue should be the dominant color
  EXPECT_GT(rSkySum, 0u);
  EXPECT_GT(gSkySum, 0u);
  EXPECT_GT(bSkySum, 0u);
  EXPECT_GT(bSkySum, gSkySum);
  EXPECT_GT(bSkySum, rSkySum);

  // Clean up
  engine->DestroyScene(scene);
}
