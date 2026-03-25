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

#include "CommonRenderingTest.hh"

#include "gz/rendering/RenderingIface.hh"
#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderEngineManager.hh"

using namespace gz;
using namespace rendering;

class RenderEngineManagerTest : public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(RenderEngineManagerTest, RenderEngineManager)
{
  // Tested missing APIs
  EXPECT_FALSE(RenderEngineManager::Instance()->HasEngine("invalid"));
  EXPECT_EQ(nullptr, RenderEngineManager::Instance()->Engine("invalid"));
  EXPECT_FALSE(RenderEngineManager::Instance()->UnloadEngine("invalid"));

  EXPECT_FALSE(RenderEngineManager::Instance()->UnloadEngineAt(100));

  RenderEngineManager::Instance()->RegisterEngine("invalid", nullptr);

  RenderEngineManager::Instance()->RegisterEngine("ogre2", engine);

  std::list<std::string> paths = {"path1", "path2"};
  RenderEngineManager::Instance()->SetPluginPaths(paths);
}
