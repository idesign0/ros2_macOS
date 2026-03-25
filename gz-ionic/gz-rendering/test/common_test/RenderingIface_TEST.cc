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

#include <gz/common/Console.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/RenderEngine.hh"
#include "gz/rendering/RenderingIface.hh"

using namespace gz;
using namespace rendering;

#if GZ_RENDERING_HAVE_OGRE
static bool kHaveOgre = true;
#else
static bool kHaveOgre = false;
#endif

#if GZ_RENDERING_HAVE_OGRE2
static bool kHaveOgre2 = true;
#else
static bool kHaveOgre2 = false;
#endif

#if GZ_RENDERING_HAVE_OPTIX
static bool kHaveOptix = true;
#else
static bool kHaveOptix = false;
#endif


/////////////////////////////////////////////////
// get the number of default engines available
unsigned int defaultEnginesForTest()
{
  unsigned int count = 0u;
  count += kHaveOgre;
  count += kHaveOgre2;
  count += kHaveOptix;
  return count;
}

/////////////////////////////////////////////////
TEST(RenderingIfaceTest, HasEngine)
{
  unsigned int count = defaultEnginesForTest();
  EXPECT_EQ(count, engineCount());

  EXPECT_EQ(kHaveOgre, rendering::hasEngine("ogre"));
  EXPECT_EQ(kHaveOgre2, rendering::hasEngine("ogre2"));
  EXPECT_EQ(kHaveOptix, rendering::hasEngine("optix"));
}

/////////////////////////////////////////////////
TEST(RenderingIfaceTest, GetEngine)
{
  common::Console::SetVerbosity(4);
  auto [envEngine, envBackend, envHeadless] = GetTestParams();
  unsigned int count = defaultEnginesForTest();
  EXPECT_EQ(count, engineCount());

  EXPECT_TRUE(loadedEngines().empty());
  EXPECT_FALSE(isEngineLoaded("ogre"));
  EXPECT_FALSE(isEngineLoaded("ogre2"));
  EXPECT_FALSE(isEngineLoaded("optix"));
  EXPECT_FALSE(isEngineLoaded("no_such_engine"));
  EXPECT_EQ(nullptr, sceneFromFirstRenderEngine());

  RenderEngine *eng = engine(envEngine, std::map<std::string, std::string>());
  ASSERT_NE(nullptr, eng);
  auto engineName = eng->Name();
  ASSERT_FALSE(engineName.empty());

  EXPECT_TRUE(isEngineLoaded(engineName));
  EXPECT_TRUE(hasEngine(engineName));
  EXPECT_EQ(eng, engine(engineName));

  {
    auto scene = eng->CreateScene("scene");
    EXPECT_NE(nullptr, scene);
    EXPECT_EQ(scene, sceneFromFirstRenderEngine());
    ASSERT_EQ(1u, loadedEngines().size());
    EXPECT_EQ(eng->Name(), loadedEngines()[0]);
  }

  rendering::unloadEngine(engineName);
  EXPECT_FALSE(isEngineLoaded(engineName));

  EXPECT_TRUE(loadedEngines().empty());
  EXPECT_EQ(nullptr, sceneFromFirstRenderEngine());

  // non-existent engine
  EXPECT_EQ(nullptr, engine("no_such_engine"));
  EXPECT_EQ(nullptr, engine(1000000));
}

/////////////////////////////////////////////////
TEST(RenderingIfaceTest, RegisterEngine)
{
  common::Console::SetVerbosity(4);

  unsigned int count = defaultEnginesForTest();

  if (count == 0)
    return;

  RenderEngine *eng = nullptr;
  std::string engineToTest = "";
  if (gz::utils::env(kEngineToTestEnv, engineToTest))
    eng = engine(engineToTest);
  else
    eng = engine(0u);

  ASSERT_NE(nullptr, eng);
  auto engineName = eng->Name();
  ASSERT_FALSE(engineName.empty());

  EXPECT_TRUE(hasEngine(engineName));
  EXPECT_NO_THROW(unregisterEngine(engineName));
  EXPECT_FALSE(hasEngine(engineName));

  // register engine back with a different name
  EXPECT_NO_THROW(registerEngine("my_new_engine", eng));
  EXPECT_TRUE(hasEngine("my_new_engine"));

  // unregister engine by name
  EXPECT_NO_THROW(unregisterEngine("my_new_engine"));
  EXPECT_FALSE(hasEngine("my_new_engine"));

  // unregister non-existent engine
  EXPECT_NO_THROW(unregisterEngine("no_such_engine"));
  EXPECT_NO_THROW(unregisterEngine(1000000u));
  EXPECT_NO_THROW(unregisterEngine(nullptr));
}
