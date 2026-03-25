/*
 * Copyright (C) 2025 Open Source Robotics Foundation
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

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include <gz/common/Console.hh>

#include <gz/plugin/Loader.hh>
#include <gz/plugin/RegisterStatic.hh>

#include <gz/rendering/base/BaseRenderEngine.hh>
#include <gz/rendering/base/BaseScene.hh>
#include <gz/rendering/base/BaseStorage.hh>
#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/RenderEnginePlugin.hh>

using namespace gz;
using namespace rendering;

namespace {
// Dummy scene class used in the render engine below.
class DummyScene final: public BaseScene
{};

// Dummy singleton render engine used in the render engine plugin below.
class DummyRenderEngine final: public BaseRenderEngine
{
public: typedef BaseSceneStore<DummyScene> DummySceneStore;
public: typedef std::shared_ptr<DummySceneStore> DummySceneStorePtr;

public: static DummyRenderEngine* Instance();

private: DummyRenderEngine();

public: std::string Name() const override {return "test_engine";}

protected: bool LoadImpl(const std::map<std::string,
    std::string> &_params) override {return true;}

protected: bool InitImpl() override {return true;}

protected: ScenePtr CreateSceneImpl(unsigned int _id,
        const std::string &_name) override {return nullptr;}

protected: SceneStorePtr Scenes() const override {return this->sceneStore; }

private: DummySceneStorePtr sceneStore;
};

DummyRenderEngine* DummyRenderEngine::Instance()
{
  static DummyRenderEngine engine;
  return &engine;
}

DummyRenderEngine::DummyRenderEngine()
: sceneStore(std::make_shared<DummySceneStore>())
{}

// Dummy render engine plugin used in the test below. Note that the plugin is
// registered with the static plugin registry at the bottom of this file.
class DummyRenderEnginePlugin final : public RenderEnginePlugin
{
public: DummyRenderEnginePlugin();

public: std::string Name() const override {return engine->Name();}

public: RenderEngine *Engine() const override {return engine;}

private: DummyRenderEngine* engine;
};

DummyRenderEnginePlugin::DummyRenderEnginePlugin()
: engine(DummyRenderEngine::Instance())
{}

// Test that the dummy render engine plugin can be loaded from the static plugin
// registry.
TEST(LoadStaticRenderEnginePlugin, LoadUnloadWorks)
{
  plugin::Loader pluginLoader;
  const std::string pluginName =
      pluginLoader.LookupPlugin("DummyRenderEnginePlugin");
  EXPECT_FALSE(pluginName.empty());

  const std::string engineFilename = "static://DummyRenderEnginePlugin";
  RenderEngine *engine = rendering::engine(engineFilename);
  EXPECT_NE(nullptr, engine);

  EXPECT_EQ(DummyRenderEngine::Instance(), engine);

  EXPECT_TRUE(rendering::unloadEngine(engineFilename));
}

// Register the plugin with the static registry
GZ_ADD_STATIC_PLUGIN(DummyRenderEnginePlugin, RenderEnginePlugin)
GZ_ADD_STATIC_PLUGIN_ALIAS(DummyRenderEnginePlugin, "DummyRenderEnginePlugin")

}  // namespace
