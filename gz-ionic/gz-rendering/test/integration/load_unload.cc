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
#include <gz/utils/ExtraTestMacros.hh>

#include "CommonRenderingTest.hh"

class LoadUnloadTest : public testing::Test
{
  /// \brief A thread that loads and unloads the render engine
  public: void RenderThread()
  {
    gz::common::Console::SetVerbosity(4);

    auto [envEngine, envBackend, envHeadless] = GetTestParams();

    if (envEngine.empty())
    {
      GTEST_SKIP() << kEngineToTestEnv << " environment not set";
    }

    auto engineParams = GetEngineParams(envEngine, envBackend, envHeadless);
    gz::rendering::RenderEngine *engine =
        gz::rendering::engine(envEngine, engineParams);
    if (!engine)
    {
      GTEST_SKIP() << "Engine '" << envEngine << "' could not be loaded"
                   << std::endl;
    }

    gz::rendering::unloadEngine(envEngine);
  }
};

/////////////////////////////////////////////////
TEST_F(LoadUnloadTest, GZ_UTILS_TEST_DISABLED_ON_MAC(Thread))
{
  // verify that we can load and unload the render engine in a thread
  std::thread renderThread = std::thread(&LoadUnloadTest::RenderThread, this);
  EXPECT_TRUE(renderThread.joinable());
  EXPECT_NO_THROW(renderThread.join());
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}


