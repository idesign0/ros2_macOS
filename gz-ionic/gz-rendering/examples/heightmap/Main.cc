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

#if defined(__APPLE__)
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#elif not defined(_WIN32)
  #include <GL/glew.h>
  #include <GL/gl.h>
  #include <GL/glut.h>
#else
  #include <GL/glut.h>
#endif

#include <iostream>
#include <vector>

#include <gz/common/Console.hh>
#include <gz/common/geospatial/Dem.hh>
#include <gz/common/geospatial/HeightmapData.hh>
#include <gz/common/geospatial/ImageHeightmap.hh>
#include <gz/rendering.hh>

#include "example_config.hh"
#include "GlutWindow.hh"

using namespace gz;
using namespace rendering;

#if not defined(_WIN32)
const std::string RESOURCE_PATH =
    common::joinPaths(std::string(PROJECT_BINARY_PATH), "media");
#else
const std::string RESOURCE_PATH =
    common::joinPaths(std::string(PROJECT_BINARY_PATH), "..", "media");
#endif

//////////////////////////////////////////////////
void createImageHeightmaps(const ScenePtr _scene, VisualPtr _root)
{
  //! [create an image heightmap]
    auto data = std::make_shared<common::ImageHeightmap>();
    data->Load(common::joinPaths(RESOURCE_PATH, "heightmap_bowl.png"));

    HeightmapDescriptor desc;
    desc.SetName("example_bowl");
    desc.SetData(data);
    desc.SetSize({17, 17, 10});
    desc.SetSampling(2u);
    desc.SetUseTerrainPaging(false);

    HeightmapTexture textureA;
    textureA.SetSize(1.0);
    textureA.SetDiffuse("../media/dirt_diffusespecular.png");
    textureA.SetNormal("../media/flat_normal.png");
    desc.AddTexture(textureA);

    HeightmapBlend blendA;
    blendA.SetMinHeight(2.0);
    blendA.SetFadeDistance(5.0);
    desc.AddBlend(blendA);

    HeightmapTexture textureB;
    textureB.SetSize(1.0);
    textureB.SetDiffuse("../media/grass_diffusespecular.png");
    textureB.SetNormal("../media/flat_normal.png");
    desc.AddTexture(textureB);

    HeightmapBlend blendB;
    blendB.SetMinHeight(4.0);
    blendB.SetFadeDistance(5.0);
    desc.AddBlend(blendB);

    HeightmapTexture textureC;
    textureC.SetSize(1.0);
    textureC.SetDiffuse("../media/fungus_diffusespecular.png");
    textureC.SetNormal("../media/flat_normal.png");
    desc.AddTexture(textureC);

    auto heightmapGeom = _scene->CreateHeightmap(desc);

    auto vis = _scene->CreateVisual();
    vis->AddGeometry(heightmapGeom);
    _root->AddChild(vis);
  //! [create an image heightmap]

  //! [create another image heightmap]
    auto data2 = std::make_shared<common::ImageHeightmap>();
    data2->Load(common::joinPaths(RESOURCE_PATH, "city_terrain.jpg"));

    HeightmapDescriptor desc2;
    desc2.SetName("example_city");
    desc2.SetData(data2);
    desc2.SetSize({26, 26, 20});
    desc2.SetSampling(2u);
    desc2.SetUseTerrainPaging(true);

    HeightmapTexture textureA2;
    textureA2.SetSize(1.0);
    textureA2.SetDiffuse("../media/fungus_diffusespecular.png");
    textureA2.SetNormal("../media/flat_normal.png");
    desc2.AddTexture(textureA2);

    HeightmapBlend blendA2;
    blendA2.SetMinHeight(2.0);
    blendA2.SetFadeDistance(5.0);
    desc2.AddBlend(blendA2);

    HeightmapTexture textureB2;
    textureB2.SetSize(1.0);
    textureB2.SetDiffuse("../media/grass_diffusespecular.png");
    textureB2.SetNormal("../media/flat_normal.png");
    desc2.AddTexture(textureB2);

    HeightmapBlend blendB2;
    blendB2.SetMinHeight(8.0);
    blendB2.SetFadeDistance(5.0);
    desc2.AddBlend(blendB2);

    HeightmapTexture textureC2;
    textureC2.SetSize(1.0);
    textureC2.SetDiffuse("../media/dirt_diffusespecular.png");
    textureC2.SetNormal("../media/flat_normal.png");
    desc2.AddTexture(textureC2);
    desc2.SetPosition({30, 10, 0});
    auto heightmapGeom2 = _scene->CreateHeightmap(desc2);

    auto vis2 = _scene->CreateVisual();
    vis2->AddGeometry(heightmapGeom2);
    _root->AddChild(vis2);
  //! [create another image heightmap]
}

//////////////////////////////////////////////////
void createDemHeightmaps(const ScenePtr _scene, VisualPtr _root)
{
  //! [create a dem heightmap]
    auto data = std::make_shared<common::Dem>();
    data->Load(common::joinPaths(RESOURCE_PATH, "volcano.tif"));

    HeightmapDescriptor desc;
    desc.SetName("example_volcano");
    desc.SetData(data);
    desc.SetSize({20, 20, 18});
    desc.SetSampling(2u);
    desc.SetUseTerrainPaging(true);

    HeightmapTexture textureA;
    textureA.SetSize(1.0);
    textureA.SetDiffuse("../media/dirt_diffusespecular.png");
    textureA.SetNormal("../media/flat_normal.png");
    desc.AddTexture(textureA);

    HeightmapBlend blendA;
    blendA.SetMinHeight(2.0);
    blendA.SetFadeDistance(5.0);
    desc.AddBlend(blendA);

    HeightmapTexture textureB;
    textureB.SetSize(1.0);
    textureB.SetDiffuse("../media/grass_diffusespecular.png");
    textureB.SetNormal("../media/flat_normal.png");
    desc.AddTexture(textureB);

    HeightmapBlend blendB;
    blendB.SetMinHeight(4.0);
    blendB.SetFadeDistance(5.0);
    desc.AddBlend(blendB);

    HeightmapTexture textureC;
    textureC.SetSize(1.0);
    textureC.SetDiffuse("../media/fungus_diffusespecular.png");
    textureC.SetNormal("../media/flat_normal.png");
    desc.AddTexture(textureC);
    desc.SetPosition({30, 0, 0});

    auto heightmapGeom = _scene->CreateHeightmap(desc);

    auto vis = _scene->CreateVisual();
    vis->AddGeometry(heightmapGeom);
    _root->AddChild(vis);
  //! [create a dem heightmap]

  //! [create another dem heightmap]
    auto data2 = std::make_shared<common::Dem>();
    data2->Load(common::joinPaths(RESOURCE_PATH, "moon.tif"));

    HeightmapDescriptor desc2;
    desc2.SetName("example_moon");
    desc2.SetData(data2);
    desc2.SetSize({20, 20, 6.85});
    desc2.SetSampling(2u);
    desc2.SetUseTerrainPaging(false);

    HeightmapTexture textureA2;
    textureA2.SetSize(20.0);
    textureA2.SetDiffuse("../media/moon_diffuse.png");
    textureA2.SetNormal("../media/moon_normal.png");
    desc2.AddTexture(textureA2);
    desc2.SetPosition({0, 0, std::abs(data2->MinElevation())});
    auto heightmapGeom2 = _scene->CreateHeightmap(desc2);

    auto vis2 = _scene->CreateVisual();
    vis2->AddGeometry(heightmapGeom2);
    _root->AddChild(vis2);
  //! [create another dem heightmap]
}

//////////////////////////////////////////////////
void buildScene(ScenePtr _scene, bool _buildDemScene)
{
  // initialize _scene
  _scene->SetAmbientLight(0.3, 0.3, 0.3);
  _scene->SetBackgroundColor(0.3, 0.3, 0.3);
  VisualPtr root = _scene->RootVisual();

  // create directional light
  DirectionalLightPtr light0 = _scene->CreateDirectionalLight();
  light0->SetDirection(0.5, 0.5, -1);
  light0->SetDiffuseColor(0.8, 0.8, 0.8);
  light0->SetSpecularColor(0.5, 0.5, 0.5);
  root->AddChild(light0);

  // create point light
  PointLightPtr light1 = _scene->CreatePointLight();
  light1->SetDiffuseColor(0.8, 0.8, 0.8);
  light1->SetSpecularColor(0.2, 0.2, 0.2);
  light1->SetLocalPosition(30, -3, 6);
  light1->SetAttenuationConstant(0.1);
  light1->SetAttenuationLinear(0.001);
  light1->SetAttenuationQuadratic(0.0001);
  light1->SetAttenuationRange(80);
  light1->SetCastShadows(true);
  root->AddChild(light1);

  if (_buildDemScene)
    createDemHeightmaps(_scene, root);
  else
    createImageHeightmaps(_scene, root);

  // create gray material
  MaterialPtr gray = _scene->CreateMaterial();
  gray->SetAmbient(0.7, 0.7, 0.7);
  gray->SetDiffuse(0.7, 0.7, 0.7);
  gray->SetSpecular(0.7, 0.7, 0.7);

//! [create grid visual]
  VisualPtr grid = _scene->CreateVisual();
  GridPtr gridGeom = _scene->CreateGrid();
  gridGeom->SetCellCount(20);
  gridGeom->SetCellLength(1);
  gridGeom->SetVerticalCellCount(0);
  grid->AddGeometry(gridGeom);
  grid->SetLocalPosition(3, 0, 0.0);
  grid->SetMaterial(gray);
  root->AddChild(grid);
//! [create grid visual]

//! [create camera]
  CameraPtr camera = _scene->CreateCamera("camera");
  camera->SetLocalPosition(1.441, 25.787, 17.801);
  camera->SetLocalRotation(0.0, 0.588, -1.125);
  camera->SetImageWidth(1600);
  camera->SetImageHeight(900);
  camera->SetAntiAliasing(2);
  camera->SetAspectRatio(1.333);
  camera->SetHFOV(GZ_PI / 2);
  root->AddChild(camera);
//! [create camera]
}

//////////////////////////////////////////////////
CameraPtr createCamera(const std::string &_engineName,
    const std::map<std::string, std::string>& _params,
    bool _buildDemScene)
{
  // create and populate scene
  RenderEngine *engine = rendering::engine(_engineName, _params);
  if (!engine)
  {
    gzwarn << "Engine '" << _engineName
              << "' is not supported" << std::endl;
    return CameraPtr();
  }
  ScenePtr scene = engine->CreateScene("scene");
  buildScene(scene, _buildDemScene);

  // return camera sensor
  SensorPtr sensor = scene->SensorByName("camera");
  return std::dynamic_pointer_cast<Camera>(sensor);
}

//////////////////////////////////////////////////
int main(int _argc, char** _argv)
{
  glutInit(&_argc, _argv);

  common::Console::SetVerbosity(4);
  std::vector<std::string> engineNames;
  std::vector<CameraPtr> cameras;

  int buildDemScene = 0;
  for (int i = 1; i < _argc; ++i)
  {
    if (std::string(_argv[i]) == "--dem")
    {
      buildDemScene = i;
      break;
    }
  }

  // Expose engine name to command line because we can't instantiate both
  // ogre and ogre2 at the same time
  std::string ogreEngineName("ogre2");
  if (_argc > 1 && buildDemScene != 1)
  {
    ogreEngineName = _argv[1];
  }

  GraphicsAPI graphicsApi = defaultGraphicsAPI();
  if (_argc > 2 && buildDemScene != 2)
  {
    graphicsApi = GraphicsAPIUtils::Set(std::string(_argv[2]));
  }

  engineNames.push_back(ogreEngineName);

  for (const auto &engineName : engineNames)
  {
    try
    {
      std::map<std::string, std::string> params;
      if (engineName == "ogre2"
          && graphicsApi == GraphicsAPI::METAL)
      {
        params["metal"] = "1";
      }

      CameraPtr camera = createCamera(engineName, params, buildDemScene);
      if (camera)
      {
        cameras.push_back(camera);
      }
    }
    catch (...)
    {
      std::cerr << "Error starting up: " << engineName << std::endl;
    }
  }
  run(cameras);
  return 0;
}
