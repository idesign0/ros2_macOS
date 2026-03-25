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

#if defined(__APPLE__)
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#elif not defined(_WIN32)
  #include <GL/glew.h>
  #include <GL/gl.h>
  #include <GL/glut.h>
#endif

#include <iostream>
#include <vector>

#include <gz/common/Console.hh>
#include <gz/rendering.hh>

#include "example_config.hh"
#include "GlutWindow.hh"

using namespace gz;
using namespace rendering;

const std::string RESOURCE_PATH =
    common::joinPaths(std::string(PROJECT_BINARY_PATH), "media");

//////////////////////////////////////////////////
void buildScene(ScenePtr _scene)
{
  // initialize _scene
  _scene->SetAmbientLight(0.3, 0.3, 0.3);
  VisualPtr root = _scene->RootVisual();

  // create directional light
  DirectionalLightPtr light0 = _scene->CreateDirectionalLight();
  light0->SetDirection(-0.5, 0.5, -1);
  light0->SetDiffuseColor(0.5, 0.5, 0.5);
  light0->SetSpecularColor(0.5, 0.5, 0.5);
  root->AddChild(light0);

  // create projector
  std::string texture = common::joinPaths(RESOURCE_PATH,
        "stereo_projection_pattern_high_res_red.png");

  ProjectorPtr projector = _scene->CreateProjector("projector");
  projector->SetLocalPosition(-4, 0, 4);
  projector->SetNearClipPlane(1.0);
  projector->SetFarClipPlane(10.0);
  projector->SetLocalRotation(0, 0.6, 0);
  projector->SetTexture(texture);
  projector->SetVisibilityFlags(0x01);
  root->AddChild(projector);

  // create blue material
  MaterialPtr blue = _scene->CreateMaterial();
  blue->SetAmbient(0.0, 0.0, 0.5);
  blue->SetDiffuse(0.0, 0.0, 0.7);
  blue->SetSpecular(0.5, 0.5, 0.5);

  // create visual representing the projector
  VisualPtr box = _scene->CreateVisual();
  box->AddGeometry(_scene->CreateBox());
  box->SetLocalScale(0.1, 0.1, 0.1);
  box->SetMaterial(blue);
  projector->AddChild(box);

  // create white material
  MaterialPtr white = _scene->CreateMaterial();
  white->SetAmbient(0.5, 0.5, 0.5);
  white->SetDiffuse(0.8, 0.8, 0.8);

  // create plane visual
  VisualPtr plane = _scene->CreateVisual();
  plane->AddGeometry(_scene->CreatePlane());
  plane->SetLocalScale(10, 10, 1);
  plane->SetMaterial(white);
  root->AddChild(plane);

  // create plane2 visual
  VisualPtr plane2 = _scene->CreateVisual();
  plane2->AddGeometry(_scene->CreatePlane());
  plane2->SetLocalScale(10, 10, 1);
  plane2->SetLocalPosition(5, 0, 5);
  plane2->SetLocalRotation(math::Quaternion(math::Vector3d(0, -GZ_PI/2, 0)));
  plane2->SetMaterial(white);
  root->AddChild(plane2);

  // create plane3 visual
  VisualPtr plane3 = _scene->CreateVisual();
  plane3->AddGeometry(_scene->CreatePlane());
  plane3->SetLocalScale(10, 10, 1);
  plane3->SetLocalPosition(0, -5, 5);
  plane3->SetLocalRotation(math::Quaternion(math::Vector3d(-GZ_PI/2, 0, 0)));
  plane3->SetMaterial(white);
  root->AddChild(plane3);

  // create plane4 visual
  VisualPtr plane4 = _scene->CreateVisual();
  plane4->AddGeometry(_scene->CreatePlane());
  plane4->SetLocalScale(10, 10, 1);
  plane4->SetLocalPosition(0, 5, 5);
  plane4->SetLocalRotation(math::Quaternion(math::Vector3d(GZ_PI/2, 0, 0)));
  plane4->SetMaterial(white);
  root->AddChild(plane4);

  // create green material
  MaterialPtr green = _scene->CreateMaterial();
  green->SetAmbient(0.0, 0.5, 0.0);
  green->SetDiffuse(0.0, 0.7, 0.0);
  green->SetSpecular(0.5, 0.5, 0.5);

  // create sphere visual
  VisualPtr sphere = _scene->CreateVisual();
  sphere->AddGeometry(_scene->CreateSphere());
  sphere->SetLocalPosition(3, 0, 1);
  sphere->SetMaterial(green);
  root->AddChild(sphere);

  // create camera
  CameraPtr camera = _scene->CreateCamera("camera");
  camera->SetLocalPosition(-5, 0.0, 5.0);
  camera->SetLocalRotation(0.0, GZ_PI / 5.0, 0.0);
  camera->SetImageWidth(800);
  camera->SetImageHeight(600);
  camera->SetAspectRatio(1.333);
  camera->SetHFOV(GZ_PI / 2);
  camera->SetVisibilityMask(0x01);

  root->AddChild(camera);
}

//////////////////////////////////////////////////
CameraPtr createCamera(const std::string &_engineName,
    const std::map<std::string, std::string>& _params)
{
  // create and populate scene
  RenderEngine *engine = rendering::engine(_engineName, _params);
  if (!engine)
  {
    std::cout << "Engine '" << _engineName
              << "' is not supported" << std::endl;
    return CameraPtr();
  }
  ScenePtr scene = engine->CreateScene("scene");
  buildScene(scene);

  // return camera sensor
  SensorPtr sensor = scene->SensorByName("camera");
  return std::dynamic_pointer_cast<Camera>(sensor);
}

//////////////////////////////////////////////////
int main(int _argc, char** _argv)
{
  glutInit(&_argc, _argv);

  // Expose engine name to command line because we can't instantiate both
  // ogre and ogre2 at the same time
  std::string ogreEngineName("ogre");
  if (_argc > 1)
  {
    ogreEngineName = _argv[1];
  }

  GraphicsAPI graphicsApi = defaultGraphicsAPI();
  if (_argc > 2)
  {
    graphicsApi = GraphicsAPIUtils::Set(std::string(_argv[2]));
  }

  common::Console::SetVerbosity(4);
  std::vector<std::string> engineNames;
  std::vector<CameraPtr> cameras;

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

      CameraPtr camera = createCamera(engineName, params);
      if (camera)
        cameras.push_back(camera);
    }
    catch (...)
    {
      std::cerr << "Error starting up: " << engineName << std::endl;
    }
  }
  run(cameras);

  return 0;
}
