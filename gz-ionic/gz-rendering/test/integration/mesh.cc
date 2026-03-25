/*
 * Copyright (C) 2024 Open Source Robotics Foundation
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
#include <gz/common/Mesh.hh>
#include <gz/common/MeshManager.hh>
#include <gz/common/SubMesh.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Image.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class MeshTest: public CommonRenderingTest
{
};

/////////////////////////////////////////////////
TEST_F(MeshTest, NormalMapWithoutTexCoord)
{
#if defined _WIN32
  // https://github.com/gazebosim/gz-rendering/issues/1117
  CHECK_UNSUPPORTED_ENGINE("ogre");
#elif defined __linux__
  // This test fails in ogre on Noble Github Actions.
  // Need to investigate further.
  std::string githubAction;
  if (common::env("GITHUB_ACTIONS", githubAction))
  {
    CHECK_UNSUPPORTED_ENGINE("ogre");
  }
#endif

  // Create a mesh with 2 submeshes - one with red texture and the other with
  // green texture. Add texcoords to the red submesh but not the green submesh.
  // Verify that we can set normal map to the red submesh and the two
  // submeshes should be rendered correctly

  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);
  scene->SetAmbientLight(1.0, 1.0, 1.0);
  scene->SetBackgroundColor(0.0, 0.0, 1.0);

  VisualPtr root = scene->RootVisual();
  ASSERT_NE(nullptr, root);

  // create directional light
  DirectionalLightPtr light0 = scene->CreateDirectionalLight();
  light0->SetDirection(0.0, 0.0, -1);
  light0->SetDiffuseColor(1.0, 1.0, 1.0);
  light0->SetSpecularColor(1.0, 1.0, 1.0);
  root->AddChild(light0);

  common::Mesh mesh;
  common::SubMesh subMesh0;
  subMesh0.SetName("submesh0");
  subMesh0.AddVertex(math::Vector3d(0, 0, 0));
  subMesh0.AddVertex(math::Vector3d(1, 0, 0));
  subMesh0.AddVertex(math::Vector3d(1, 1, 0));
  subMesh0.AddNormal(math::Vector3d(0, 0, 1));
  subMesh0.AddNormal(math::Vector3d(0, 0, 1));
  subMesh0.AddNormal(math::Vector3d(0, 0, 1));
  subMesh0.AddIndex(0);
  subMesh0.AddIndex(1);
  subMesh0.AddIndex(2);
  subMesh0.AddTexCoordBySet(math::Vector2d(0, 0), 0);
  subMesh0.AddTexCoordBySet(math::Vector2d(0, 1), 0);
  subMesh0.AddTexCoordBySet(math::Vector2d(0, 0), 0);
  common::MaterialPtr material0;
  material0 = std::make_shared<common::Material>();
  std::string textureMapName0 = "red_diffuse_map";
  auto textureMapData0 = std::make_shared<common::Image>();
  auto red = std::make_unique<unsigned char[]>(3);
  red.get()[0] = 255u;
  red.get()[1] = 0u;
  red.get()[2] = 0u;
  textureMapData0->SetFromData(red.get(), 1, 1, common::Image::RGB_INT8);
  material0->SetTextureImage(textureMapName0, textureMapData0);

  common::Pbr pbr;
  std::string normalMapName = "normal_map";
  auto normalMapData = std::make_shared<common::Image>();
  auto normal = std::make_unique<unsigned char[]>(3);
  normal.get()[0] = 127u;
  normal.get()[1] = 127u;
  normal.get()[2] = 255u;
  normalMapData->SetFromData(normal.get(), 1, 1, common::Image::RGB_INT8);

  pbr.SetNormalMap(normalMapName, common::NormalMapSpace::TANGENT,
      normalMapData);
  material0->SetPbrMaterial(pbr);
  mesh.AddMaterial(material0);
  subMesh0.SetMaterialIndex(0u);

  common::SubMesh subMesh1;
  subMesh1.SetName("submesh1");
  subMesh1.AddVertex(math::Vector3d(0, 0, 0));
  subMesh1.AddVertex(math::Vector3d(1, 1, 0));
  subMesh1.AddVertex(math::Vector3d(0, 1, 0));
  subMesh1.AddNormal(math::Vector3d(0, 0, 1));
  subMesh1.AddNormal(math::Vector3d(0, 0, 1));
  subMesh1.AddNormal(math::Vector3d(0, 0, 1));
  subMesh1.AddIndex(0);
  subMesh1.AddIndex(1);
  subMesh1.AddIndex(2);

  common::MaterialPtr material1;
  material1 = std::make_shared<common::Material>();
  std::string textureMapName1 = "green_diffuse_map";
  auto textureMapData1 = std::make_shared<common::Image>();
  auto green = std::make_unique<unsigned char[]>(3);
  green.get()[0] = 0u;
  green.get()[1] = 255u;
  green.get()[2] = 0u;
  textureMapData1->SetFromData(green.get(), 1, 1, common::Image::RGB_INT8);
  material1->SetTextureImage(textureMapName1, textureMapData1);
  mesh.AddMaterial(material1);
  subMesh1.SetMaterialIndex(1u);

  mesh.AddSubMesh(subMesh0);
  mesh.AddSubMesh(subMesh1);

  MeshDescriptor descriptor;
  descriptor.meshName = "test_mesh";
  descriptor.mesh = &mesh;
  MeshPtr meshGeom = scene->CreateMesh(descriptor);

  VisualPtr visual = scene->CreateVisual("visual");
  visual->AddGeometry(meshGeom);
  root->AddChild(visual);

  // create camera
  CameraPtr camera = scene->CreateCamera();
  ASSERT_NE(nullptr, camera);
  camera->SetLocalPosition(0.5, 0.5, 0.5);
  camera->SetLocalRotation(0, 1.57, 0);
  camera->SetImageWidth(32);
  camera->SetImageHeight(32);
  root->AddChild(camera);

  Image image = camera->CreateImage();
  camera->Capture(image);

  unsigned int height = camera->ImageHeight();
  unsigned int width = camera->ImageWidth();
  unsigned int channelCount = PixelUtil::ChannelCount(camera->ImageFormat());
  unsigned int step = width * channelCount;
  unsigned char *data = image.Data<unsigned char>();
  for (unsigned int i = 0; i < height; ++i)
  {
    for (unsigned int j = 0; j < step; j += channelCount)
    {
      unsigned int idx = i * step + j;
      unsigned int r = data[idx];
      unsigned int g = data[idx + 1];
      unsigned int b = data[idx + 2];

      // color should be a shade of red (submesh0) or green (submesh1)
      EXPECT_TRUE(r > 0u || g > 0u);
      EXPECT_EQ(b, 0u);
    }
  }

  // Clean up
  engine->DestroyScene(scene);
}
