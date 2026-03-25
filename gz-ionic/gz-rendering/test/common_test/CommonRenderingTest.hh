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

#ifndef COMMON_RENDERING_TEST_HH_
#define COMMON_RENDERING_TEST_HH_

#include <gtest/gtest.h>

#include <string>
#include <unordered_set>

#include <gz/common/Console.hh>

#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/RenderEngine.hh>
#include <gz/utils/Environment.hh>

constexpr const char * kEngineToTestEnv = "GZ_ENGINE_TO_TEST";
constexpr const char * kEngineBackend = "GZ_ENGINE_BACKEND";
constexpr const char * kEngineHeadless = "GZ_ENGINE_HEADLESS";

static std::tuple<std::string, std::string, std::string> GetTestParams()
{
  std::string engine = "";
  std::string backend = "";
  std::string headless = "";

  if (gz::utils::env(kEngineToTestEnv, engine))
  {
    gzdbg << "Read GZ_ENGINE_TO_TEST=" << engine << std::endl;
    if (gz::utils::env(kEngineBackend, backend))
    {
      gzdbg << "Read GZ_ENGINE_BACKEND=" << backend << std::endl;
    }

    if (gz::utils::env(kEngineHeadless, headless))
    {
      gzdbg << "Read GZ_ENGINE_HEADLESS=" << headless << std::endl;
    }
  }
  return {engine, backend, headless};
}

static std::map<std::string, std::string>
GetEngineParams(const std::string &_engine, const std::string &_backend, const std::string &_headless)
{
  std::map<std::string, std::string> engineParams;
  if (_engine == "ogre2" && _backend == "vulkan")
  {
    gzdbg << "Using OGRE2-VULKAN backend to test" << std::endl;
    engineParams["vulkan"] = "1";
  }
  else if (_engine == "ogre2" && _backend == "metal")
  {
    gzdbg << "Using OGRE2-METAL backend to test" << std::endl;
    engineParams["metal"] = "1";
  }
  if (!_headless.empty())
  {
    engineParams["headless"] = "1";
  }
  return engineParams;
}

/// \brief Common test fixture for all rendering tests
/// This allows for the engine, backend, and headless parameters
/// to be controlled via environment variables
class CommonRenderingTest: public testing::Test
{
  /// \brief Set up the test case
  public: void SetUp() override
  {
    gz::common::Console::SetVerbosity(4);

    auto [envEngine, envBackend, envHeadless] = GetTestParams();

    if (envEngine.empty())
    {
      GTEST_SKIP() << kEngineToTestEnv << " environment not set";
    }

    auto engineParams = GetEngineParams(envEngine, envBackend, envHeadless);
    this->engineToTest = envEngine;
    engine = gz::rendering::engine(this->engineToTest, engineParams);
    if (!engine)
    {
      GTEST_SKIP() << "Engine '" << this->engineToTest << "' could not be loaded" << std::endl;
    }
  }

  /// \brief Tear down the test case
  public: void TearDown() override
  {
    if(engine)
    {
      ASSERT_TRUE(gz::rendering::unloadEngine(this->engineToTest));
    }
  }

  /// \brief String name of the engine to test
  public: std::string engineToTest;

  /// \brief Pointer to the rendering engine to test
  public: gz::rendering::RenderEngine *engine = nullptr;
};


/// \brief Check that the current engine being tested is supported.
/// If the engine is not in the set of passed arguments, the test is skipped
/// Example:
/// Skip test if engine is not ogre or ogre2
/// CHECK_SUPPORTED_ENGINE("ogre", "ogre2");
#define CHECK_SUPPORTED_ENGINE(...) \
if(std::unordered_set<std::string>({__VA_ARGS__}).count(this->engineToTest) == 0) \
  GTEST_SKIP() << "Engine '" << this->engineToTest << "' unsupported";

/// \brief Check that the current engine being tested is unsupported
/// If the engine is in the set of passed arguments, the test is skipped
/// Example:
/// Skip test if engine is ogre2
/// CHECK_UNSUPPORTED_ENGINE("ogre");
#define CHECK_UNSUPPORTED_ENGINE(...) \
if(std::unordered_set<std::string>({__VA_ARGS__}).count(this->engineToTest) != 0) \
  GTEST_SKIP() << "Engine '" << this->engineToTest << "' unsupported";

/// \brief Check that renderpass is supported by the engine under test
/// Skip the test if the engine doesn't support renderpass
#define CHECK_RENDERPASS_SUPPORTED() \
{ \
  gz::rendering::RenderPassSystemPtr rpSystem = this->engine->RenderPassSystem(); \
  if (!rpSystem) \
  { \
    GTEST_SKIP() << "Engine '" << this->engineToTest << "' does not support RenderPassSystem"; \
  } \
} \

#endif  // COMMON_RENDERING_TEST_HH_
