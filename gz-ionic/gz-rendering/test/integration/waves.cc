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

#include <string>
#include <memory>

#include "CommonRenderingTest.hh"

#include <gz/common/Image.hh>
#include <gz/common/MeshManager.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Image.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/ShaderParams.hh"

using namespace gz;
using namespace rendering;

class WavesTest: public CommonRenderingTest
{
  // Path to test media directory
  public: const std::string TEST_MEDIA_PATH =
          gz::common::joinPaths(std::string(PROJECT_SOURCE_PATH),
                "test", "media");
};

/////////////////////////////////////////////////
TEST_F(WavesTest, Waves)
{
  if (GraphicsAPI::METAL == this->engine->GraphicsAPI())
  {
    // \todo(iche033) Test fails on ogre2 with metal. To be investigated.
    return;
  }

  // add resources in build dir
  engine->AddResourcePath(
      common::joinPaths(std::string(PROJECT_BUILD_PATH), "src"));

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(0.3, 0.3, 0.3);

  scene->SetBackgroundColor(1.0, 0.0, 0.0);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  // create directional light
  DirectionalLightPtr light0 = scene->CreateDirectionalLight();
  light0->SetDirection(-0.5, 0.5, -1);
  light0->SetDiffuseColor(0.5, 0.5, 0.5);
  light0->SetSpecularColor(0.5, 0.5, 0.5);
  root->AddChild(light0);

  const std::string vertexShaderGLSLFile = "GerstnerWaves_vs.glsl";
  const std::string fragmentShaderGLSLFile = "GerstnerWaves_fs.glsl";

  const std::string vertexShaderGLSL330File = "GerstnerWaves_vs_330.glsl";
  const std::string fragmentShaderGLSL330File = "GerstnerWaves_fs_330.glsl";

  const std::string vertexShaderMetalFile = "GerstnerWaves_vs.metal";
  const std::string fragmentShaderMetalFile = "GerstnerWaves_fs.metal";

  std::string vertexShaderFile;
  std::string fragmentShaderFile;

  if (GraphicsAPI::METAL == this->engine->GraphicsAPI())
  {
    vertexShaderFile = vertexShaderMetalFile;
    fragmentShaderFile = fragmentShaderMetalFile;
  }
  else if (this->engine->Name() == "ogre")
  {
    vertexShaderFile = vertexShaderGLSLFile;
    fragmentShaderFile = fragmentShaderGLSLFile;
  }
  else
  {
    vertexShaderFile = vertexShaderGLSL330File;
    fragmentShaderFile = fragmentShaderGLSL330File;
  }

  // create shader materials
  // path to look for vertex and fragment shader parameters
  std::string vertexShaderPath = gz::common::joinPaths(
      TEST_MEDIA_PATH, "materials", "programs", vertexShaderFile);

  std::string fragmentShaderPath = gz::common::joinPaths(
      TEST_MEDIA_PATH, "materials", "programs", fragmentShaderFile);

  // create shader material
  gz::rendering::MaterialPtr shader = scene->CreateMaterial();
  shader->SetVertexShader(vertexShaderPath);
  shader->SetFragmentShader(fragmentShaderPath);

  // create waves visual
  VisualPtr waves = scene->CreateVisual("waves");
  MeshDescriptor descriptor;
  descriptor.meshName = common::joinPaths(
      TEST_MEDIA_PATH, "meshes", "mesh.dae");
  common::MeshManager *meshManager = common::MeshManager::Instance();
  descriptor.mesh = meshManager->Load(descriptor.meshName);
  ASSERT_NE(nullptr, descriptor.mesh);
  MeshPtr meshGeom = scene->CreateMesh(descriptor);
  waves->AddGeometry(meshGeom);
  waves->SetLocalPosition(3, 0, 0);
  waves->SetLocalScale(1, 1, 1);
  waves->SetMaterial(shader, false);

  // set vertex shader params
  ShaderParamsPtr vsParams = shader->VertexShaderParams();

  if (this->engine->Name() == "ogre2" || this->engine->Name() == "ogre")
  {
    // worldviewproj_matrix is a constant defined by ogre.
    // Here we add a line to add this constant to the params.
    // The specified value is ignored as it will be auto bound to the
    // correct type and value. See available constants:
    // https://github.com/OGRECave/ogre-next/blob/v2-2/OgreMain/src/OgreGpuProgramParams.cpp
    (*vsParams)["worldviewproj_matrix"] = 1;
  }

  (*vsParams)["Nwaves"] = 3;
  (*vsParams)["rescale"] = 0.5f;

  float bumpScale[2] = {25.0f, 25.0f};
  (*vsParams)["bumpScale"].InitializeBuffer(2);
  (*vsParams)["bumpScale"].UpdateBuffer(bumpScale);

  float bumpSpeed[2] = {0.01f, 0.01f};
  (*vsParams)["bumpSpeed"].InitializeBuffer(2);
  (*vsParams)["bumpSpeed"].UpdateBuffer(bumpSpeed);

  float amplitude = 3.0f;
  float amplitudeV[3] = {0.6f * amplitude, 0.4f * amplitude, 0.3f * amplitude};
  (*vsParams)["amplitude"].InitializeBuffer(3);
  (*vsParams)["amplitude"].UpdateBuffer(amplitudeV);

  float frequency = 0.028f;
  float wavenumberV[3] = {frequency, 3.2f * frequency, 1.8f * frequency};
  (*vsParams)["wavenumber"].InitializeBuffer(3);
  (*vsParams)["wavenumber"].UpdateBuffer(wavenumberV);

  float omegaV[3] = {0.5f, 1.7f, 1.0f};
  (*vsParams)["omega"].InitializeBuffer(3);
  (*vsParams)["omega"].UpdateBuffer(omegaV);

  float dir0[2] = {-1.0f, 0.0f};
  (*vsParams)["dir0"].InitializeBuffer(2);
  (*vsParams)["dir0"].UpdateBuffer(dir0);

  float dir1[2] = {-0.7f, 0.7f};
  (*vsParams)["dir1"].InitializeBuffer(2);
  (*vsParams)["dir1"].UpdateBuffer(dir1);

  float dir2[2] = {0.7f, 0.7f};
  (*vsParams)["dir2"].InitializeBuffer(2);
  (*vsParams)["dir2"].UpdateBuffer(dir2);

  float steepness = 1.0f;
  float steepnessV[3] = {steepness, 1.5f * steepness, 0.8f * steepness};
  (*vsParams)["steepness"].InitializeBuffer(3);
  (*vsParams)["steepness"].UpdateBuffer(steepnessV);

  float tau = 2.0f;
  (*vsParams)["tau"] = tau;

  // camera_position_object_space is a constant defined by ogre.
  (*vsParams)["camera_position_object_space"] = 1;

  (*vsParams)["t"] = 0.0f;

  // set fragment shader params
  ShaderParamsPtr fsParams = shader->FragmentShaderParams();

  float hdrMultiplier = 0.4f;
  (*fsParams)["hdrMultiplier"] = hdrMultiplier;

  float fresnelPower = 5.0f;
  (*fsParams)["fresnelPower"] = fresnelPower;

  float shallowColor[4] = {0.0f, 0.1f, 0.3f, 1.0f};
  (*fsParams)["shallowColor"].InitializeBuffer(4);
  (*fsParams)["shallowColor"].UpdateBuffer(shallowColor);

  float deepColor[4] = {0.0f, 0.05f, 0.2f, 1.0f};
  (*fsParams)["deepColor"].InitializeBuffer(4);
  (*fsParams)["deepColor"].UpdateBuffer(deepColor);

  std::string bumpMapPath = gz::common::joinPaths(TEST_MEDIA_PATH,
      "materials", "textures", "wave_normals.dds");
  (*fsParams)["bumpMap"].SetTexture(bumpMapPath);

  std::string cubeMapPath = gz::common::joinPaths(TEST_MEDIA_PATH,
      "materials", "textures", "skybox_lowres.dds");

  (*fsParams)["cubeMap"].SetTexture(cubeMapPath,
      ShaderParam::ParamType::PARAM_TEXTURE_CUBE, 1u);

  // create camera
  CameraPtr camera = scene->CreateCamera();
  ASSERT_NE(nullptr, camera);
  camera->SetLocalPosition(0, 0.0, 3.5);
  camera->SetImageWidth(640);
  camera->SetImageHeight(480);
  root->AddChild(camera);

  // capture original image with red background
  Image image = camera->CreateImage();
  camera->Capture(image);

  // Add waves to the scene
  root->AddChild(waves);

  // capture image with waves in the scene
  Image imageWaves = camera->CreateImage();
  camera->Capture(imageWaves);

  // Compare image pixels
  unsigned char *data = image.Data<unsigned char>();
  unsigned char *dataWaves = imageWaves.Data<unsigned char>();
  unsigned int height = camera->ImageHeight();
  unsigned int width = camera->ImageWidth();
  unsigned int channelCount = PixelUtil::ChannelCount(camera->ImageFormat());
  unsigned int step = width * channelCount;

  unsigned int rSum = 0u;
  unsigned int gSum = 0u;
  unsigned int bSum = 0u;

  unsigned int rWavesTopSum = 0u;
  unsigned int gWavesTopSum = 0u;
  unsigned int bWavesTopSum = 0u;

  unsigned int rWavesBottomSum = 0u;
  unsigned int gWavesBottomSum = 0u;
  unsigned int bWavesBottomSum = 0u;

  for (unsigned int i = 0; i < height; ++i)
  {
    for (unsigned int j = 0; j < step; j += channelCount)
    {
      unsigned int idx = i * step + j;
      rSum += data[idx];
      gSum += data[idx + 1];
      bSum += data[idx + 2];

      if (i < height / 2)
      {
        rWavesTopSum += dataWaves[idx];
        gWavesTopSum += dataWaves[idx + 1];
        bWavesTopSum += dataWaves[idx + 2];
      }
      else
      {
        rWavesBottomSum += dataWaves[idx];
        gWavesBottomSum += dataWaves[idx + 1];
        bWavesBottomSum += dataWaves[idx + 2];
      }
    }
  }

  // No waves - red background
  EXPECT_GT(rSum, 0u);
  EXPECT_EQ(0u, gSum);
  EXPECT_EQ(0u, bSum);

  // waves enabled - top half should be red background while
  // bottom half should be blue due to waves
  EXPECT_GT(rWavesTopSum, 0u);
  EXPECT_EQ(0u, gWavesTopSum);
  EXPECT_EQ(0u, bWavesTopSum);
  EXPECT_GT(rWavesBottomSum, 0u);
  EXPECT_GT(gWavesBottomSum, 0u);
  EXPECT_GT(bWavesBottomSum, 0u);
  EXPECT_GT(bWavesBottomSum, gWavesBottomSum);
  EXPECT_GT(bWavesBottomSum, rWavesBottomSum);

  // Clean up
  engine->DestroyScene(scene);
}
