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

#include "gz/rendering/Camera.hh"
#include "gz/rendering/DepthCamera.hh"
#include "gz/rendering/DistortionPass.hh"
#include "gz/rendering/GaussianNoisePass.hh"
#include "gz/rendering/Image.hh"
#include "gz/rendering/LensFlarePass.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/RenderPassSystem.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/WideAngleCamera.hh"

#include <gz/utils/ExtraTestMacros.hh>

constexpr double DOUBLE_TOL = 1e-6;
unsigned int g_pointCloudCounter = 0;

/////////////////////////////////////////////////
void OnNewRgbPointCloud(float *_scanDest, const float *_scan,
                  unsigned int _width, unsigned int _height,
                  unsigned int _channels,
                  const std::string &/*_format*/)
{
  EXPECT_EQ(10u, _width);
  EXPECT_EQ(10u, _height);
  EXPECT_EQ(4u, _channels);

  const auto size = _width * _height * _channels;
  memcpy(_scanDest, _scan, size * sizeof(float));
  g_pointCloudCounter++;
}
using namespace gz;
using namespace rendering;

/////////////////////////////////////////////////
class RenderPassTest: public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(RenderPassTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(GaussianNoise))
{
  CHECK_RENDERPASS_SUPPORTED();

  // get the render pass system
  RenderPassSystemPtr rpSystem = this->engine->RenderPassSystem();
  // add resources in build dir
  engine->AddResourcePath(
      common::joinPaths(std::string(PROJECT_BUILD_PATH), "src"));

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(0.3, 0.3, 0.3);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  // create  camera
  CameraPtr camera = scene->CreateCamera();
  ASSERT_NE(nullptr, camera);
  camera->SetImageWidth(100);
  camera->SetImageHeight(100);
  root->AddChild(camera);

  // create directional light
  DirectionalLightPtr light = scene->CreateDirectionalLight();
  light->SetDirection(0.0, 0.0, -1);
  light->SetDiffuseColor(0.5, 0.5, 0.5);
  light->SetSpecularColor(0.5, 0.5, 0.5);
  root->AddChild(light);

  // create green material
  MaterialPtr green = scene->CreateMaterial();
  green->SetDiffuse(0.0, 0.7, 0.0);
  green->SetSpecular(0.5, 0.5, 0.5);

  // create box
  VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetLocalPosition(1.0, 0.0, 0.5);
  box->SetMaterial(green);
  root->AddChild(box);

  // capture original image with box (no noise)
  Image image = camera->CreateImage();
  camera->Capture(image);

  // add Gaussian noise render pass to camera
  double noiseMean = 0.1;
  double noiseStdDev = 0.01;

  // add gaussian noise pass
  RenderPassPtr pass = rpSystem->Create<GaussianNoisePass>();
  GaussianNoisePassPtr noisePass =
      std::dynamic_pointer_cast<GaussianNoisePass>(pass);
  noisePass->SetMean(noiseMean);
  noisePass->SetStdDev(noiseStdDev);
  camera->AddRenderPass(noisePass);

  // capture image with noise
  Image imageNoise = camera->CreateImage();
  camera->Capture(imageNoise);

  // Compare image pixels
  unsigned int diffMax = 0;
  unsigned int diffSum = 0;
  unsigned char *data = image.Data<unsigned char>();
  unsigned char *dataNoise = imageNoise.Data<unsigned char>();
  unsigned int height = camera->ImageHeight();
  unsigned int width = camera->ImageWidth();
  unsigned int channelCount = PixelUtil::ChannelCount(camera->ImageFormat());
  unsigned int step = width * channelCount;
  for (unsigned int i = 0; i < height; ++i)
  {
    for (unsigned int j = 0; j < step; j += channelCount)
    {
      unsigned int idx = i * step + j;
      for (unsigned int k = 0; k < channelCount; ++k)
      {
        int p = data[idx + k];
        int p2 = dataNoise[idx + k];

        unsigned int absDiff = std::abs(p - p2);
        if (absDiff > diffMax)
          diffMax = absDiff;
        diffSum += absDiff;
      }
    }
  }
  unsigned int diffAvg = diffSum / (width * height * channelCount);

  // We expect that there will be some non-zero difference between the two
  // images.
  EXPECT_NE(diffSum, 0u);
  // We expect that the average difference will be well within 3-sigma.
  EXPECT_NEAR(diffAvg/255., noiseMean, 3*noiseStdDev);

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(RenderPassTest, GZ_UTILS_TEST_DISABLED_ON_MAC(DepthGaussianNoise))
{
  CHECK_RENDERPASS_SUPPORTED();
  CHECK_SUPPORTED_ENGINE("ogre2");

  int imgWidth = 10;
  int imgHeight = 10;

  double aspectRatio_ = imgWidth/imgHeight;

  double unitBoxSize = 1.0;
  math::Vector3d boxPosition(1.8, 0.0, 0.0);

  gz::rendering::ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  // red background
  scene->SetBackgroundColor(1.0, 0.0, 0.0);

  // Create an scene with a box in it
  scene->SetAmbientLight(1.0, 1.0, 1.0);
  VisualPtr root = scene->RootVisual();

  // create blue material
  MaterialPtr blue = scene->CreateMaterial();
  blue->SetAmbient(0.0, 0.0, 1.0);
  blue->SetDiffuse(0.0, 0.0, 1.0);
  blue->SetSpecular(0.0, 0.0, 1.0);

  // create box visual
  VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetOrigin(0.0, 0.0, 0.0);
  box->SetLocalPosition(boxPosition);
  box->SetLocalRotation(0, 0, 0);
  box->SetLocalScale(unitBoxSize, unitBoxSize, unitBoxSize);
  box->SetMaterial(blue);
  root->AddChild(box);
  {
    double farDist = 10.0;
    double nearDist = 0.15;
    double hfov_ = 1.05;
    // Create depth camera
    auto depthCamera = scene->CreateDepthCamera("DepthCamera");
    ASSERT_NE(depthCamera, nullptr);

    math::Pose3d testPose(math::Vector3d(0, 0, 0),
        math::Quaterniond::Identity);
    depthCamera->SetLocalPose(testPose);

    // Configure depth camera
    depthCamera->SetImageWidth(imgWidth);
    EXPECT_EQ(depthCamera->ImageWidth(),
      static_cast<unsigned int>(imgWidth));
    depthCamera->SetImageHeight(imgHeight);
    EXPECT_EQ(depthCamera->ImageHeight(),
      static_cast<unsigned int>(imgHeight));
    depthCamera->SetFarClipPlane(farDist);
    EXPECT_NEAR(depthCamera->FarClipPlane(), farDist, DOUBLE_TOL);
    depthCamera->SetNearClipPlane(nearDist);
    EXPECT_NEAR(depthCamera->NearClipPlane(), nearDist, DOUBLE_TOL);
    depthCamera->SetAspectRatio(aspectRatio_);
    EXPECT_NEAR(depthCamera->AspectRatio(), aspectRatio_, DOUBLE_TOL);
    depthCamera->SetHFOV(hfov_);
    EXPECT_NEAR(depthCamera->HFOV().Radian(), hfov_, DOUBLE_TOL);

    depthCamera->CreateDepthTexture();
    scene->RootVisual()->AddChild(depthCamera);

    // Add Gaussian noise
    double noiseMean = 0.1;
    double noiseStdDev = 0.01;

    // add gaussian noise pass
    RenderPassSystemPtr rpSystem = engine->RenderPassSystem();
    RenderPassPtr pass = rpSystem->Create<GaussianNoisePass>();
    GaussianNoisePassPtr noisePass =
        std::dynamic_pointer_cast<GaussianNoisePass>(pass);
    noisePass->SetMean(noiseMean);
    noisePass->SetStdDev(noiseStdDev);
    depthCamera->AddRenderPass(noisePass);

    // rgb point cloud data callback
    unsigned int pointCloudChannelCount = 4u;
    float *pointCloudData = new float[
        imgHeight * imgWidth * pointCloudChannelCount];
    common::ConnectionPtr connection =
      depthCamera->ConnectNewRgbPointCloud(
          std::bind(&::OnNewRgbPointCloud, pointCloudData,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
            std::placeholders::_4, std::placeholders::_5));

    // update and verify we get new data
    g_pointCloudCounter = 0u;
    depthCamera->Update();
    EXPECT_EQ(1u, g_pointCloudCounter);

    // compute mid, left, and right indices to be used later for retrieving data
    // from point cloud image

    // point cloud image indices
    int midWidth = static_cast<int>(depthCamera->ImageWidth() * 0.5);
    int midHeight = static_cast<int>(depthCamera->ImageHeight() * 0.5);
    double expectedRangeAtMidPoint = boxPosition.X() - unitBoxSize * 0.5;

    int pcMid = midHeight * depthCamera->ImageWidth() * pointCloudChannelCount
        + (midWidth-1) * pointCloudChannelCount;
    int pcLeft = midHeight * depthCamera->ImageWidth() * pointCloudChannelCount;
    int pcRight = (midHeight+1)
        * (depthCamera->ImageWidth() * pointCloudChannelCount)
        - pointCloudChannelCount;

    float maxVal = math::INF_D;

    // values should be well within 4-sigma
    float noiseTol = 4.0*noiseStdDev;
    unsigned int colorNoiseTol = static_cast<unsigned int>(255.0*noiseTol);

    // Verify Point Cloud XYZ values
    {
      // check mid point
      float mx = pointCloudData[pcMid];
      EXPECT_NEAR(expectedRangeAtMidPoint + noiseMean, mx, noiseTol);

      // check left and right points
      float lx = pointCloudData[pcLeft];
      float ly = pointCloudData[pcLeft + 1];
      float lz = pointCloudData[pcLeft + 2];
      EXPECT_FLOAT_EQ(maxVal, lx);
      EXPECT_FLOAT_EQ(maxVal, ly);
      EXPECT_FLOAT_EQ(maxVal, lz);

      float rx = pointCloudData[pcRight];
      float ry = pointCloudData[pcRight + 1];
      float rz = pointCloudData[pcRight + 2];
      EXPECT_FLOAT_EQ(maxVal, rx);
      EXPECT_FLOAT_EQ(maxVal, ry);
      EXPECT_FLOAT_EQ(maxVal, rz);

      // all points on the box should have similar z position
      float mz = pointCloudData[pcMid + 2];
      float midLeftZ = pointCloudData[pcMid + 2 - pointCloudChannelCount];
      float midRightZ = pointCloudData[pcMid + 2 + pointCloudChannelCount];
      // 2 noisy values should be within 2 * 4 sigma
      EXPECT_NEAR(mz, midLeftZ, 2*noiseTol);
      EXPECT_NEAR(mz, midRightZ, 2*noiseTol);

      // Verify Point Cloud RGB values
      // The mid point should be blue
      float mc = pointCloudData[pcMid + 3];
      uint32_t *mrgba = reinterpret_cast<uint32_t *>(&mc);
      unsigned int mr = *mrgba >> 24 & 0xFF;
      unsigned int mg = *mrgba >> 16 & 0xFF;
      unsigned int mb = *mrgba >> 8 & 0xFF;
      unsigned int ma = *mrgba >> 0 & 0xFF;
      EXPECT_NEAR(0u, mr, colorNoiseTol);
      EXPECT_NEAR(0u, mg, colorNoiseTol);
      EXPECT_GT(mb, 0u);

      // Far left and right points should be red (background color)
      float lc = pointCloudData[pcLeft + 3];
      uint32_t *lrgba = reinterpret_cast<uint32_t *>(&lc);
      unsigned int lr = *lrgba >> 24 & 0xFF;
      unsigned int lg = *lrgba >> 16 & 0xFF;
      unsigned int lb = *lrgba >> 8 & 0xFF;
      unsigned int la = *lrgba >> 0 & 0xFF;

      EXPECT_NEAR(255u, lr, colorNoiseTol);
      EXPECT_NEAR(0u, lg, colorNoiseTol);
      EXPECT_NEAR(0u, lb, colorNoiseTol);

      float rc = pointCloudData[pcRight + 3];
      uint32_t *rrgba = reinterpret_cast<uint32_t *>(&rc);
      unsigned int rr = *rrgba >> 24 & 0xFF;
      unsigned int rg = *rrgba >> 16 & 0xFF;
      unsigned int rb = *rrgba >> 8 & 0xFF;
      unsigned int ra = *rrgba >> 0 & 0xFF;

      EXPECT_NEAR(255u, rr, colorNoiseTol);
      EXPECT_NEAR(0u, rg, colorNoiseTol);
      EXPECT_NEAR(0u, rb, colorNoiseTol);

      // Note: internal texture format used is RGB with no alpha channel
      // We observed the values can be either 255 or 0 but graphics card
      // drivers are free to fill it with any value they want.
      // This should be fixed in ogre 2.2 in gz-rendering6 which forbids
      // the use of RGB format.
      // see https://github.com/gazebosim/gz-rendering/issues/315
      EXPECT_TRUE(255u == ma || 0u == ma);
      EXPECT_TRUE(255u == la || 0u == la);
      EXPECT_TRUE(255u == ra || 0u == ra);
    }

    // Clean up
    connection.reset();
    if (pointCloudData)
      delete [] pointCloudData;
  }

  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(RenderPassTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(Distortion))
{
  CHECK_RENDERPASS_SUPPORTED();
  // Distortion isn't supported in ogre2
  CHECK_UNSUPPORTED_ENGINE("ogre2");

  // add resources in build dir
  this->engine->AddResourcePath(
      common::joinPaths(std::string(PROJECT_BUILD_PATH), "src"));

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(0.3, 0.3, 0.3);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  unsigned int width = 320;
  unsigned int height = 240;

  // create  camera
  CameraPtr camera = scene->CreateCamera();
  ASSERT_NE(nullptr, camera);
  camera->SetImageWidth(width);
  camera->SetImageHeight(height);
  root->AddChild(camera);

  // create directional light
  DirectionalLightPtr light = scene->CreateDirectionalLight();
  light->SetDirection(0.0, 0.0, -1);
  light->SetDiffuseColor(0.5, 0.5, 0.5);
  light->SetSpecularColor(0.5, 0.5, 0.5);
  root->AddChild(light);

  // create white material
  MaterialPtr white = scene->CreateMaterial();
  white->SetAmbient(0.5, 0.5, 0.5);
  white->SetDiffuse(0.8, 0.8, 0.8);
  white->SetReceiveShadows(true);
  white->SetReflectivity(0);

  // create plane
  VisualPtr plane = scene->CreateVisual();
  plane->AddGeometry(scene->CreatePlane());
  plane->SetLocalScale(5, 8, 1);
  plane->SetLocalPosition(3, 0, -0.5);
  plane->SetMaterial(white);
  root->AddChild(plane);

  // capture original image with box (no distortion)
  Image image = camera->CreateImage();
  Image imageBarrel = camera->CreateImage();
  Image imagePincushion = camera->CreateImage();
  camera->Capture(image);

  RenderPassSystemPtr rpSystem = engine->RenderPassSystem();
  // add barrel distortion pass
  {
    RenderPassPtr pass = rpSystem->Create<DistortionPass>();
    DistortionPassPtr distortionPass =
        std::dynamic_pointer_cast<DistortionPass>(pass);
    distortionPass->SetK1(-0.1349);
    distortionPass->SetK2(-0.51868);
    distortionPass->SetK3(-0.001);
    camera->AddRenderPass(distortionPass);
    camera->Capture(imageBarrel);
    camera->RemoveRenderPass(distortionPass);
  }
  // add pincushion distortion pass
  {
    RenderPassPtr pass = rpSystem->Create<DistortionPass>();
    DistortionPassPtr distortionPass =
        std::dynamic_pointer_cast<DistortionPass>(pass);
    distortionPass->SetK1(0.1349);
    distortionPass->SetK2(0.51868);
    distortionPass->SetK3(0.001);
    camera->AddRenderPass(distortionPass);
    camera->Capture(imagePincushion);
    camera->RemoveRenderPass(distortionPass);
  }

  unsigned char* imageData = static_cast<unsigned char*>(image.Data());
  unsigned char* imageBarrelData =
      static_cast<unsigned char*>(imageBarrel.Data());
  unsigned char* imagePincushionData =
      static_cast<unsigned char*>(imagePincushion.Data());

  unsigned int colorSum = 0;
  unsigned int colorSum2 = 0;
  unsigned int colorSum3 = 0;
  for (unsigned int y = 0; y < height; ++y)
  {
    for (unsigned int x = 0; x < width*3; x+=3)
    {
      unsigned int r = imageData[(y*width*3) + x];
      unsigned int g = imageData[(y*width*3) + x + 1];
      unsigned int b = imageData[(y*width*3) + x + 2];
      colorSum += r + g + b;
      unsigned int r3 = imageBarrelData[(y*width*3) + x];
      unsigned int g3 = imageBarrelData[(y*width*3) + x + 1];
      unsigned int b3 = imageBarrelData[(y*width*3) + x + 2];
      colorSum2 += r3 + g3 + b3;
      unsigned int r4 = imagePincushionData[(y*width*3) + x];
      unsigned int g4 = imagePincushionData[(y*width*3) + x + 1];
      unsigned int b4 = imagePincushionData[(y*width*3) + x + 2];
      colorSum3 += r4 + g4 + b4;
    }
  }

  // Compare colors. Barrel distorted image should have more darker pixels than
  // the original as the ground plane has been warped to occupy more of the
  // image. The same should be true for pincushion distortion, because the
  // ground plane is still distorted to be larger - just different parts
  // of the image are distorted.
  EXPECT_GT(colorSum, colorSum2);
  EXPECT_GT(colorSum, colorSum3);

  // Clean up
  engine->DestroyScene(scene);
}

static void TestLensFlare(gz::rendering::RenderEngine *_engine,
                          bool _wideAngleCamera)
{
  // get the render pass system
  RenderPassSystemPtr rpSystem = _engine->RenderPassSystem();
  // add resources in build dir
  _engine->AddResourcePath(
    common::joinPaths(std::string(PROJECT_BUILD_PATH), "src"));

  ScenePtr scene = _engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(0.3, 0.3, 0.3);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  // create  camera
  CameraPtr camera = scene->CreateWideAngleCamera();
  if (!_wideAngleCamera)
  {
    camera = scene->CreateCamera();
  }
  else
  {
    camera = scene->CreateWideAngleCamera();
  }
  ASSERT_NE(nullptr, camera);
  camera->SetImageWidth(100);
  camera->SetImageHeight(100);
  camera->SetLocalPosition(5.0, 3.0, 0.7);
  root->AddChild(camera);

  // create directional light
  DirectionalLightPtr light = scene->CreateDirectionalLight();
  light->SetDirection(-0.9, -0.3, -0.2);
  light->SetDiffuseColor(0.5, 0.5, 0.5);
  light->SetSpecularColor(0.5, 0.5, 0.5);
  root->AddChild(light);

  // create green material
  MaterialPtr green = scene->CreateMaterial();
  green->SetDiffuse(0.0, 0.7, 0.0);
  green->SetSpecular(0.5, 0.5, 0.5);

  // create box
  VisualPtr box = scene->CreateVisual();
  box->AddGeometry(scene->CreateBox());
  box->SetLocalPosition(9.0, 3.0, 0.7);
  box->SetMaterial(green);
  root->AddChild(box);

  //
  // TEST 0: No LensFlare (never added) vs LensFlare
  //

  // capture original image with box (no lens flare)
  Image refImage = camera->CreateImage();
  camera->Capture(refImage);

  // add Lens Flare render pass to camera

  // add lens flare pass
  RenderPassPtr pass = rpSystem->Create<LensFlarePass>();
  LensFlarePassPtr lensFlarePass =
    std::dynamic_pointer_cast<LensFlarePass>(pass);
  lensFlarePass->Init(scene);
  lensFlarePass->SetLight(light);
  camera->AddRenderPass(lensFlarePass);

  // capture image lens flare
  Image imageLensFlared = camera->CreateImage();
  camera->Capture(imageLensFlared);

  {
    // Compare image pixels
    unsigned char *refData = refImage.Data<unsigned char>();
    unsigned char *dataLensFlared = imageLensFlared.Data<unsigned char>();
    unsigned int height = camera->ImageHeight();
    unsigned int width = camera->ImageWidth();
    unsigned int channelCount = PixelUtil::ChannelCount(camera->ImageFormat());
    unsigned int step = width * channelCount;
    for (unsigned int i = 0; i < height; ++i)
    {
      for (unsigned int j = 0; j < step; j += channelCount)
      {
        unsigned int idx = i * step + j;
        for (unsigned int k = 0; k < channelCount; ++k)
        {
          // We expect every single pixel to be brighter than reference
          EXPECT_GT(dataLensFlared[idx + k], refData[idx + k]);
        }
      }
    }
  }

  //
  // TEST 1: No LensFlare (never added) vs No LensFlare (disabled)
  //

  // Disable image lens flare and try again. It should be equal to ref
  lensFlarePass->SetEnabled(false);
  camera->Capture(imageLensFlared);
  {
    // Compare image pixels
    unsigned char *refData = refImage.Data<unsigned char>();
    unsigned char *dataLensFlared = imageLensFlared.Data<unsigned char>();
    unsigned int height = camera->ImageHeight();
    unsigned int width = camera->ImageWidth();
    unsigned int channelCount = PixelUtil::ChannelCount(camera->ImageFormat());
    unsigned int step = width * channelCount;
    for (unsigned int i = 0; i < height; ++i)
    {
      for (unsigned int j = 0; j < step; j += channelCount)
      {
        unsigned int idx = i * step + j;
        for (unsigned int k = 0; k < channelCount; ++k)
        {
          // We expect every single pixel to be equal to reference
          // due to the effect being disabled (despite being added)
          EXPECT_EQ(dataLensFlared[idx + k], refData[idx + k]);
        }
      }
    }
  }

  //
  // TEST 2: No LensFlare (light from behind, disabled) vs
  //         No LensFlare (light from behind, enabled)
  // We need a new reference.
  //
  light->SetDirection(0.9, 0.3, -0.2);  // BEHIND CAMERA

  lensFlarePass->SetEnabled(false);
  camera->Capture(refImage);

  lensFlarePass->SetEnabled(true);
  camera->Capture(imageLensFlared);

  {
    // Compare image pixels
    unsigned char *refData = refImage.Data<unsigned char>();
    unsigned char *dataLensFlared = imageLensFlared.Data<unsigned char>();
    unsigned int height = camera->ImageHeight();
    unsigned int width = camera->ImageWidth();
    unsigned int channelCount = PixelUtil::ChannelCount(camera->ImageFormat());
    unsigned int step = width * channelCount;
    for (unsigned int i = 0; i < height; ++i)
    {
      for (unsigned int j = 0; j < step; j += channelCount)
      {
        unsigned int idx = i * step + j;
        for (unsigned int k = 0; k < channelCount; ++k)
        {
          // We expect every single pixel to be equal to reference
          // due to lens flare coming from behind us
          EXPECT_EQ(dataLensFlared[idx + k], refData[idx + k]);
        }
      }
    }
  }

  //
  // TEST 3: No LensFlare (disabled) vs
  //         No LensFlare (enabled, but 100% occluded)
  // We need a new reference.
  //
  light->SetDirection(-0.9, -0.1, -0.1);  // FULL OCCLUSION

  lensFlarePass->SetEnabled(false);
  camera->Capture(refImage);

  lensFlarePass->SetEnabled(true);
  camera->Capture(imageLensFlared);

  {
    // Compare image pixels
    unsigned char *refData = refImage.Data<unsigned char>();
    unsigned char *dataLensFlared = imageLensFlared.Data<unsigned char>();
    unsigned int height = camera->ImageHeight();
    unsigned int width = camera->ImageWidth();
    unsigned int channelCount = PixelUtil::ChannelCount(camera->ImageFormat());
    unsigned int step = width * channelCount;
    for (unsigned int i = 0; i < height; ++i)
    {
      for (unsigned int j = 0; j < step; j += channelCount)
      {
        unsigned int idx = i * step + j;
        for (unsigned int k = 0; k < channelCount; ++k)
        {
          // We expect every single pixel to be equal to reference
          // due to lens flare being 100% occluded
          EXPECT_EQ(dataLensFlared[idx + k], refData[idx + k]);
        }
      }
    }
  }

  //
  // TEST 4: No LensFlare (disabled) vs
  //         LensFlare (occluded; but occlusion disabled)
  // Reference MUST be the same as previous test.
  //
  const uint32_t oldOcclusionSteps = lensFlarePass->OcclusionSteps();
  lensFlarePass->SetEnabled(true);
  lensFlarePass->SetOcclusionSteps(0u);
  camera->Capture(imageLensFlared);
  // Restore setting
  lensFlarePass->SetOcclusionSteps(oldOcclusionSteps);

  {
    // Compare image pixels
    unsigned char *refData = refImage.Data<unsigned char>();
    unsigned char *dataLensFlared = imageLensFlared.Data<unsigned char>();
    unsigned int height = camera->ImageHeight();
    unsigned int width = camera->ImageWidth();
    unsigned int channelCount = PixelUtil::ChannelCount(camera->ImageFormat());
    unsigned int step = width * channelCount;
    for (unsigned int i = 0; i < height; ++i)
    {
      for (unsigned int j = 0; j < step; j += channelCount)
      {
        unsigned int idx = i * step + j;
        for (unsigned int k = 0; k < channelCount; ++k)
        {
          // We expect every single pixel to be brighter to reference
          // due to lens flare being occluded BUT occlusion is disabled
          EXPECT_GT(dataLensFlared[idx + k], refData[idx + k]);
        }
      }
    }
  }

  //
  // TEST 5: No LensFlare (disabled) vs
  //         LensFlare (partially occluded) vs
  //         LensFlare (partially occluded, occlusion disabled)
  // We need a new reference.
  //
  light->SetDirection(-0.9, -0.1, -0.13);  // PARTIAL OCCLUSSION

  lensFlarePass->SetEnabled(false);
  camera->Capture(refImage);

  lensFlarePass->SetEnabled(true);
  camera->Capture(imageLensFlared);

  Image imageLensNoOcclusion = camera->CreateImage();
  lensFlarePass->SetOcclusionSteps(0u);
  camera->Capture(imageLensNoOcclusion);
  // Restore setting
  lensFlarePass->SetOcclusionSteps(oldOcclusionSteps);

  {
    int uncomparablePixelCount = 0;

    // Compare image pixels
    unsigned char *refData = refImage.Data<unsigned char>();
    unsigned char *dataLensFlarePartialOcclusion =
      imageLensFlared.Data<unsigned char>();
    unsigned char *dataLensFlareNoOcclusion =
      imageLensNoOcclusion.Data<unsigned char>();
    unsigned int height = camera->ImageHeight();
    unsigned int width = camera->ImageWidth();
    unsigned int channelCount = PixelUtil::ChannelCount(camera->ImageFormat());
    unsigned int step = width * channelCount;
    for (unsigned int i = 0; i < height; ++i)
    {
      for (unsigned int j = 0; j < step; j += channelCount)
      {
        unsigned int idx = i * step + j;
        for (unsigned int k = 0; k < channelCount; ++k)
        {
          // We expect every single pixel to be brighter to reference
          // due to lens flare, despite being partially occluded
          EXPECT_GT(dataLensFlarePartialOcclusion[idx + k], refData[idx + k]);
          EXPECT_GT(dataLensFlareNoOcclusion[idx + k], refData[idx + k]);

          if (dataLensFlareNoOcclusion[idx + k] == 255u &&
              dataLensFlarePartialOcclusion[idx + k] == 255u)
          {
            // Nothing to do here. They are as bright as possible.
            // We can't compare them.
            ++uncomparablePixelCount;
          }
          else
          {
            EXPECT_GT(dataLensFlareNoOcclusion[idx + k],
                      dataLensFlarePartialOcclusion[idx + k]);
          }
        }
      }
    }

    // If a significant number of pixels between Partial & No occlusion are
    // incomparable, then this test is meaningless and needs tweaking.
    //
    // The threshold value is arbitrary and chosen empirically
    // If this value is exceded consider just raising it.
    if (!_wideAngleCamera)
    {
      EXPECT_LE(uncomparablePixelCount, 5);
    }
    else
    {
      if (_engine->Name() == "ogre")
      {
        EXPECT_LE(uncomparablePixelCount, 51);
      }
      else
      {
        EXPECT_LE(uncomparablePixelCount, 40);
      }
    }

    // Same test as before, but this time as a percentage of total pixels.
    //
    // While the threshold is again arbitrary; if this value is exceded,
    // rather than raising the threshold, we should consider the test
    // flawed and change it (i.e. the reference is too bright)
    EXPECT_LE(uncomparablePixelCount,
              (camera->ImageWidth() * camera->ImageHeight() * 1u) / 100u);
  }

  //
  // TEST 6: LensFlare (no occlusion), ensure coordinate convention is correct
  // The lens flare must come from top left corner.
  //
  // This test ensures there are no discrepancies between API backends
  // (e.g. different engines, OpenGL vs Vulkan vs Metal, etc)
  //
  // There is no reference.
  //
  light->SetDirection(-1.0, -0.25, -0.25);
  lensFlarePass->SetEnabled(true);
  box->SetVisible(false);
  camera->Capture(imageLensFlared);
  box->SetVisible(true);

  {
    int brightness[2][2] = { { 0, 0 }, { 0, 0 } };

    // Accumulate brightness in each quadrant
    unsigned char *dataLensFlared = imageLensFlared.Data<unsigned char>();
    unsigned int height = camera->ImageHeight();
    unsigned int width = camera->ImageWidth();
    unsigned int channelCount = PixelUtil::ChannelCount(camera->ImageFormat());
    unsigned int step = width * channelCount;
    for (unsigned int i = 0; i < height; ++i)
    {
      for (unsigned int j = 0; j < step; j += channelCount)
      {
        unsigned int idx = i * step + j;
        for (unsigned int k = 0; k < channelCount; ++k)
        {
          // We expect every single pixel to be brighter to reference
          // due to lens flare being occluded BUT occlusion is disabled
          brightness[(j * 2u) / step][(i * 2u) / height] +=
            dataLensFlared[idx + k];
        }
      }
    }

    // We expect quadrant 0, 0 to be much brigther than all of the others
    // due to visibly housing the source of the lens flare (the sun)
    EXPECT_GT(brightness[0][0], brightness[0][1]);
    EXPECT_GT(brightness[0][0], brightness[1][0]);
    EXPECT_GT(brightness[0][0], brightness[1][1]);

    // Because the lens flare is in diagonal from top left going towards
    // bottom right, we expect quadrant 1, 1 to be brigther than the rest
    // except 0, 0.
    EXPECT_GT(brightness[1][1], brightness[0][1]);
    EXPECT_GT(brightness[1][1], brightness[1][0]);
  }

  // Clean up
  _engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(RenderPassTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(LensFlarePass))
{
  CHECK_UNSUPPORTED_ENGINE("optix");
  CHECK_RENDERPASS_SUPPORTED();

  TestLensFlare(this->engine, false);
}

/////////////////////////////////////////////////
TEST_F(RenderPassTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(LensFlareWideAnglePass))
{
  CHECK_UNSUPPORTED_ENGINE("optix");
  CHECK_RENDERPASS_SUPPORTED();

  TestLensFlare(this->engine, true);
}
