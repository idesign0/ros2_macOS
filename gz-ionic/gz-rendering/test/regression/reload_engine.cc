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

#include <gz/rendering/Scene.hh>
#include <gz/rendering/Visual.hh>
#include <gz/rendering/BoundingBoxCamera.hh>
#include <gz/rendering/Camera.hh>
#include <gz/rendering/DepthCamera.hh>
#include <gz/rendering/GpuRays.hh>
#include <gz/rendering/SegmentationCamera.hh>
#include <gz/rendering/ThermalCamera.hh>
#include <gz/rendering/WideAngleCamera.hh>

#include <gz/utils/ExtraTestMacros.hh>

constexpr auto kNumRetries = 3;

/// \brief Test fixture for reloading engines.
/// Since the CommonRenderingTest loads an engine by default,
/// we are doing a custom implementation here.
class ReloadEngineTest: public ::testing::Test
{
  /// \brief Set up the test fixture
  public: void SetUp() override
  {
    gz::common::Console::SetVerbosity(4);
    auto [envEngine, envBackend, envHeadless] = GetTestParams();
    if (envEngine.empty())
    {
      GTEST_SKIP() << kEngineToTestEnv << " environment not set";
    }

    this->engineToTest = envEngine;
    this->engineParams = GetEngineParams(envEngine, envBackend, envHeadless);
  }

  /// \brief Load the configured engine and run a series of rendering commands
  /// \param[in] _exec Function to execute on loaded engine
  public: void Run(std::function<void(gz::rendering::RenderEngine*)> _exec)
  {
    for (size_t ii = 0; ii < kNumRetries; ++ii)
    {
      auto engine = gz::rendering::engine(this->engineToTest,
                                          this->engineParams);
      ASSERT_NE(nullptr, engine);
      _exec(engine);
      ASSERT_TRUE(gz::rendering::unloadEngine(this->engineToTest));
    }
  }

  /// \brief Engine under test
  protected: std::string engineToTest;

  /// \brief Parameters for spawning rendering engine
  protected: std::map<std::string, std::string> engineParams;
};

/////////////////////////////////////////////////
TEST_F(ReloadEngineTest, Empty)
{
  // Noop test
  this->Run([](auto){});
}

/////////////////////////////////////////////////
TEST_F(ReloadEngineTest, Scene)
{
  this->Run([](auto engine){
    auto scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, scene);
    engine->DestroyScene(scene);
  });
}

/////////////////////////////////////////////////
TEST_F(ReloadEngineTest, BoundingBoxCamera)
{
  CHECK_SUPPORTED_ENGINE("ogre2");

  this->Run([](auto engine){
    auto scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, scene);
    auto root = scene->RootVisual();
    ASSERT_NE(nullptr, root);

    auto camera = scene->CreateBoundingBoxCamera("camera");
    ASSERT_NE(nullptr, camera);
    camera->SetImageWidth(500);
    camera->SetImageHeight(500);
    root->AddChild(camera);

    camera->Update();
    engine->DestroyScene(scene);
  });
}

/////////////////////////////////////////////////
TEST_F(ReloadEngineTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(Camera))
{
  this->Run([](auto engine){
    auto scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, scene);
    auto root = scene->RootVisual();
    ASSERT_NE(nullptr, root);

    auto camera = scene->CreateCamera("camera");
    ASSERT_NE(nullptr, camera);
    camera->SetImageWidth(500);
    camera->SetImageHeight(500);
    root->AddChild(camera);

    camera->Update();
    engine->DestroyScene(scene);
  });
}

/////////////////////////////////////////////////
TEST_F(ReloadEngineTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(DepthCamera))
{
  this->Run([](auto engine){
    auto scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, scene);
    auto root = scene->RootVisual();
    ASSERT_NE(nullptr, root);

    auto camera = scene->CreateDepthCamera("camera");
    ASSERT_NE(nullptr, camera);
    camera->SetImageWidth(500);
    camera->SetImageHeight(500);
    root->AddChild(camera);

    camera->Update();
    engine->DestroyScene(scene);
  });
}

/////////////////////////////////////////////////
TEST_F(ReloadEngineTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(GpuRays))
{
  this->Run([](auto engine){
    auto scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, scene);
    auto root = scene->RootVisual();
    ASSERT_NE(nullptr, root);

    auto gpuRays = scene->CreateGpuRays("gpu_rays");
    ASSERT_NE(nullptr, gpuRays);
    gpuRays->SetAngleMin(-1.0);
    gpuRays->SetAngleMax(1.0);
    gpuRays->SetRayCount(1000);
    gpuRays->SetVerticalRayCount(1);
    root->AddChild(gpuRays);

    gpuRays->Update();
    engine->DestroyScene(scene);
  });
}

/////////////////////////////////////////////////
TEST_F(ReloadEngineTest, SegmentationCamera)
{
  CHECK_SUPPORTED_ENGINE("ogre2");

  this->Run([](auto engine){
    auto scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, scene);
    auto root = scene->RootVisual();
    ASSERT_NE(nullptr, root);

    auto camera = scene->CreateSegmentationCamera("camera");
    ASSERT_NE(nullptr, camera);
    camera->SetImageWidth(500);
    camera->SetImageHeight(500);
    root->AddChild(camera);

    camera->Update();
    engine->DestroyScene(scene);
  });
}

/////////////////////////////////////////////////
TEST_F(ReloadEngineTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(ThermalCamera))
{
  CHECK_SUPPORTED_ENGINE("ogre", "ogre2");

  this->Run([](auto engine){
    auto scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, scene);
    auto root = scene->RootVisual();
    ASSERT_NE(nullptr, root);

    auto camera = scene->CreateThermalCamera("camera");
    ASSERT_NE(nullptr, camera);
    camera->SetImageWidth(500);
    camera->SetImageHeight(500);
    root->AddChild(camera);

    camera->Update();
    engine->DestroyScene(scene);
  });
}

/////////////////////////////////////////////////
TEST_F(ReloadEngineTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(WideAngleCamera))
{
  CHECK_SUPPORTED_ENGINE("ogre");

  this->Run([](auto engine){
    auto scene = engine->CreateScene("scene");
    ASSERT_NE(nullptr, scene);
    auto root = scene->RootVisual();
    ASSERT_NE(nullptr, root);

    auto camera = scene->CreateWideAngleCamera("camera");
    ASSERT_NE(nullptr, camera);
    camera->SetImageWidth(500);
    camera->SetImageHeight(500);
    root->AddChild(camera);

    camera->Update();
    engine->DestroyScene(scene);
  });
}
