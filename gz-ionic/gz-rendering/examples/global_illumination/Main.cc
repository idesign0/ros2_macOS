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

#include <iostream>
#include <vector>

#include <gz/common/Console.hh>
#include <gz/common/MeshManager.hh>
#include <gz/rendering.hh>

#include "GiConfig.hh"
#include "Sdl2Window.hh"

#include "example_config.hh"

#include <SDL.h>

using namespace gz;
using namespace rendering;

const std::string RESOURCE_PATH =
  common::joinPaths(std::string(PROJECT_BINARY_PATH), "media");

#if GI_METHOD == 1
gz::rendering::GlobalIlluminationVctPtr g_gi = nullptr;
#else
gz::rendering::GlobalIlluminationCiVctPtr g_gi = nullptr;
#endif

//////////////////////////////////////////////////
void buildScene(ScenePtr _scene)
{
  // initialize _scene
  _scene->SetBackgroundColor(0.2, 0.2, 0.2);
  VisualPtr root = _scene->RootVisual();

  // enable sky
  _scene->SetSkyEnabled(true);

  // create PBR material
  MaterialPtr matPBR = _scene->CreateMaterial();
  std::string textureMap = common::joinPaths(RESOURCE_PATH, "pump_albedo.png");
  std::string normalMap = common::joinPaths(RESOURCE_PATH, "pump_normal.png");
  std::string roughnessMap =
    common::joinPaths(RESOURCE_PATH, "pump_roughness.png");
  std::string metalnessMap =
    common::joinPaths(RESOURCE_PATH, "pump_metallic.png");
  std::string environmentMap =
    common::joinPaths(RESOURCE_PATH, "fort_point.dds");
  matPBR->SetTexture(textureMap);
  matPBR->SetNormalMap(normalMap);
  matPBR->SetRoughnessMap(roughnessMap);
  matPBR->SetMetalnessMap(metalnessMap);
  matPBR->SetMetalness(0.7);
  matPBR->SetRoughness(0.3);
  matPBR->SetEnvironmentMap(environmentMap);

  // create mesh for PBR
  MeshDescriptor descriptorPBR;
  descriptorPBR.meshName = common::joinPaths(RESOURCE_PATH, "pump.dae");
  common::MeshManager *meshManager = common::MeshManager::Instance();
  descriptorPBR.mesh = meshManager->Load(descriptorPBR.meshName);
  if (descriptorPBR.mesh)
  {
    VisualPtr meshPBR = _scene->CreateVisual("pump");
    meshPBR->SetLocalPosition(2, 0.0, -0.3);
    meshPBR->SetLocalRotation(0, 0, 0);
    MeshPtr meshPBRGeom = _scene->CreateMesh(descriptorPBR);
    meshPBRGeom->SetMaterial(matPBR);
    meshPBR->AddGeometry(meshPBRGeom);
    meshPBR->SetStatic(true);
    root->AddChild(meshPBR);
  }
  else
  {
    gzerr << "Failed load mesh: " << descriptorPBR.meshName << std::endl;
  }

  // create green material
  MaterialPtr green = _scene->CreateMaterial();
  green->SetDiffuse(0.0, 0.8, 0.0);
  green->SetSpecular(0.2, 0.4, 0.2);
  green->SetMetalness(0.5);
  green->SetRoughness(0.8);

  // create cylinder visual
  VisualPtr cylinder = _scene->CreateVisual("cylinder");
  cylinder->AddGeometry(_scene->CreateCylinder());
  cylinder->SetLocalPosition(3, -0.5, 0.3);
  cylinder->SetLocalScale(0.7, 0.7, 0.7);
  cylinder->SetMaterial(green);
  cylinder->SetStatic(true);
  root->AddChild(cylinder);

  // create duck material
  MaterialPtr duckMat = _scene->CreateMaterial();
  duckMat->SetTexture(common::joinPaths(RESOURCE_PATH, "duck.png"));
  duckMat->SetRoughness(0.3f);
  duckMat->SetMetalness(0.3f);

  // create a mesh
  VisualPtr mesh = _scene->CreateVisual("duck");
  mesh->SetLocalPosition(4, 0, 0);
  mesh->SetLocalRotation(1.5708, 0, 2.0);
  MeshDescriptor descriptor;
  descriptor.meshName = common::joinPaths(RESOURCE_PATH, "duck.dae");
  descriptor.mesh = meshManager->Load(descriptor.meshName);
  MeshPtr meshGeom = _scene->CreateMesh(descriptor);
  meshGeom->SetMaterial(duckMat);
  mesh->AddGeometry(meshGeom);
  mesh->SetStatic(true);
  root->AddChild(mesh);

  // create red material
  MaterialPtr red = _scene->CreateMaterial();
  red->SetDiffuse(0.8, 0.0, 0.0);
  red->SetSpecular(0.5, 0.2, 0.2);
  red->SetRoughness(0.2);
  red->SetMetalness(1.0);

  // create sphere visual
  VisualPtr sphere = _scene->CreateVisual("sphere");
  sphere->AddGeometry(_scene->CreateSphere());
  sphere->SetLocalPosition(2.5, 0, 0.3);
  sphere->SetLocalRotation(0, 0, 0);
  sphere->SetLocalScale(0.5, 0.5, 0.5);
  sphere->SetStatic(true);
  sphere->SetMaterial(red);
  root->AddChild(sphere);

  //! [create envmap]
  // create mirror material
  MaterialPtr mirrorMat = _scene->CreateMaterial();
  mirrorMat->SetDiffuse(1.0, 1.0, 1.0);
  mirrorMat->SetRoughness(0.1);
  mirrorMat->SetMetalness(0.9);
  std::string skyEnvironmentMap =
    common::joinPaths(RESOURCE_PATH, "skybox_lowres.dds");
  mirrorMat->SetEnvironmentMap(skyEnvironmentMap);
  //! [create envmap]

  // create box visual
  VisualPtr box = _scene->CreateVisual("box");
  box->AddGeometry(_scene->CreateBox());
  box->SetLocalPosition(3.0, 0.5, 0.3);
  box->SetLocalRotation(0, 0, 0);
  box->SetLocalScale(0.5, 0.5, 0.5);
  box->SetStatic(true);
  box->SetMaterial(mirrorMat);
  root->AddChild(box);

  // create backpack material
  MaterialPtr backpackMat = _scene->CreateMaterial();
  backpackMat->SetTexture(common::joinPaths(RESOURCE_PATH, "backpack.png"));
  backpackMat->SetRoughness(0.8f);
  backpackMat->SetMetalness(0.0f);

  // create a backpack
  // backpack mesh is mirrored and requires texture addressing mode to be
  // set to 'wrap', which is the default in ign-rendering-ogre2
  VisualPtr backpack = _scene->CreateVisual("backpack");
  backpack->SetLocalPosition(2.5, -1, 0);
  backpack->SetLocalRotation(0, 0, -1.57);
  MeshDescriptor backpackDesc;
  backpackDesc.meshName = common::joinPaths(RESOURCE_PATH, "backpack.dae");
  backpackDesc.mesh = meshManager->Load(backpackDesc.meshName);
  MeshPtr backpackMeshGeom = _scene->CreateMesh(backpackDesc);
  backpackMeshGeom->SetMaterial(backpackMat);
  backpack->AddGeometry(backpackMeshGeom);
  backpack->SetStatic(true);
  root->AddChild(backpack);

  // create white material
  MaterialPtr white = _scene->CreateMaterial();
  white->SetDiffuse(1.0, 1.0, 1.0);
  white->SetSpecular(1.0, 1.0, 1.0);

  // create plane visual
  VisualPtr plane = _scene->CreateVisual("plane");
  plane->AddGeometry(_scene->CreatePlane());
  plane->SetLocalScale(20, 20, 1);
  plane->SetLocalPosition(0, 0, -0.5);
  plane->SetStatic(true);
  plane->SetMaterial(white);
  root->AddChild(plane);

  // create directional light
  DirectionalLightPtr light0 = _scene->CreateDirectionalLight();
  light0->SetDirection(0.5, 0.5, -1);
  light0->SetDiffuseColor(0.8, 0.7, 0.6);
  light0->SetSpecularColor(0.3, 0.3, 0.3);
  light0->SetCastShadows(true);
  root->AddChild(light0);

  // create spot light
  SpotLightPtr light1 = _scene->CreateSpotLight();
  light1->SetDiffuseColor(0.8, 0.8, 0.3);
  light1->SetSpecularColor(0.2, 0.2, 0.2);
  light1->SetLocalPosition(0, 3, 3);
  light1->SetDirection(1, -1, -1);
  light1->SetAttenuationConstant(0.1);
  light1->SetAttenuationLinear(0.001);
  light1->SetAttenuationQuadratic(0.0001);
  light1->SetFalloff(0.8);
  light1->SetCastShadows(true);
  root->AddChild(light1);

  // create point light
  PointLightPtr light2 = _scene->CreatePointLight();
  light2->SetDiffuseColor(0.2, 0.4, 0.8);
  light2->SetSpecularColor(0.2, 0.2, 0.2);
  light2->SetLocalPosition(3, 0, 2);
  light2->SetAttenuationConstant(0.1);
  light2->SetAttenuationLinear(0.001);
  light2->SetAttenuationQuadratic(0.0001);
  light2->SetCastShadows(true);
  root->AddChild(light2);

  // create spot light that does not cast shadows
  SpotLightPtr light3 = _scene->CreateSpotLight();
  light3->SetDiffuseColor(0.3, 0.3, 0.3);
  light3->SetSpecularColor(0.2, 0.2, 0.2);
  light3->SetLocalPosition(0, -3, 3);
  light3->SetDirection(1, 1, -1);
  light3->SetAttenuationConstant(0.1);
  light3->SetAttenuationLinear(0.001);
  light3->SetAttenuationQuadratic(0.0001);
  light3->SetFalloff(0.8);
  light3->SetCastShadows(false);
  root->AddChild(light3);

  // create camera
  CameraPtr camera = _scene->CreateCamera("camera");
  camera->SetLocalPosition(0.0, 0.0, 0.0);
  camera->SetLocalRotation(0.0, 0.0, 0.0);
  camera->SetImageWidth(1600);
  camera->SetImageHeight(900);
  camera->SetAntiAliasing(2);
  camera->SetAspectRatio(1.333);
  camera->SetHFOV(GZ_PI / 2);
  root->AddChild(camera);

#if GI_METHOD == 1
  auto gi = _scene->CreateGlobalIlluminationVct();
  if (gi)
  {
    const uint32_t resolution[3]{ 128u, 128u, 32u };
    const uint32_t octantCount[3]{ 4, 4, 2 };
    gi->SetResolution(resolution);
    gi->SetAnisotropic(true);
    gi->SetHighQuality(false);
    gi->SetThinWallCounter(1.0f);
    gi->SetOctantCount(octantCount);
    gi->SetAnisotropic(false);
    gi->Build();
    _scene->SetActiveGlobalIllumination(gi);
  }
#elif GI_METHOD == 2
  auto gi = _scene->CreateGlobalIlluminationCiVct();

  if (gi)
  {
    gi->SetMaxCascades(3u);

    CiVctCascadePtr cascade = gi->AddCascade(nullptr);
    const uint32_t resolution[3]{ 128u, 128u, 128u };
    const uint32_t octantCount[3]{ 4, 4, 2 };
    cascade->SetAreaHalfSize(gz::math::Vector3d(5.0, 5.0, 5.0));
    cascade->SetResolution(resolution);
    cascade->SetCameraStepSize(gz::math::Vector3d(
      1.0, 1.0, 1.0));  // Will be overriden by autoCalculateStepSizes
    cascade->SetThinWallCounter(1.0f);
    cascade->SetOctantCount(octantCount);

    cascade = gi->AddCascade(cascade.get());
    cascade->SetAreaHalfSize(gz::math::Vector3d(10.0, 10.0, 10.0));

    cascade = gi->AddCascade(cascade.get());
    cascade->SetAreaHalfSize(gz::math::Vector3d(20.0, 20.0, 20.0));

    gi->AutoCalculateStepSizes(gz::math::Vector3d(3.0, 3.0, 3.0));

    gi->Bind(camera);
    gi->SetHighQuality(false);
    gi->Start(2u, true);
    gi->Build();
    _scene->SetActiveGlobalIllumination(gi);
  }
#endif
  g_gi = gi;
}

//////////////////////////////////////////////////
CameraPtr createCamera(const std::string &_engineName,
    const std::map<std::string, std::string>& _params)
{
  // create and populate scene
  RenderEngine *engine = rendering::engine(_engineName, _params);
  if (!engine)
  {
    std::cout << "Engine '" << _engineName << "' is not supported" << std::endl;
    return CameraPtr();
  }
  ScenePtr scene = engine->CreateScene("scene");
  buildScene(scene);

  // return camera sensor
  SensorPtr sensor = scene->SensorByName("camera");
  // get render pass system
  CameraPtr camera = std::dynamic_pointer_cast<Camera>(sensor);
  RenderPassSystemPtr rpSystem = engine->RenderPassSystem();
  if (rpSystem)
  {
    // add gaussian noise pass
    RenderPassPtr pass = rpSystem->Create<GaussianNoisePass>();
    GaussianNoisePassPtr noisePass =
      std::dynamic_pointer_cast<GaussianNoisePass>(pass);
    noisePass->SetMean(0.1);
    noisePass->SetStdDev(0.08);
    noisePass->SetEnabled(false);
    camera->AddRenderPass(noisePass);
  }

  return camera;
}

//////////////////////////////////////////////////
int main(int _argc, char **_argv)
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }

  std::string engineName("ogre2");
  if (_argc > 1)
  {
    engineName = _argv[1];
  }

  GraphicsAPI graphicsApi = GraphicsAPI::OPENGL;
  if (_argc > 2)
  {
    graphicsApi = GraphicsAPIUtils::Set(std::string(_argv[2]));
  }

  common::Console::SetVerbosity(4);
  std::vector<CameraPtr> cameras;

  std::map<std::string, std::string> params;
  if (engineName == "ogre2"
      && graphicsApi == GraphicsAPI::VULKAN)
  {
    params["vulkan"] = "1";
  }
  try
  {
    CameraPtr camera = createCamera(engineName, params);
    if (camera)
    {
      cameras.push_back(camera);
    }
  }
  catch (...)
  {
    std::cerr << "Error starting up: " << engineName << std::endl;
  }
  run(cameras);

  SDL_Quit();
  return 0;
}
