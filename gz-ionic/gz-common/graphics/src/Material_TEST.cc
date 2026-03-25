/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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

#include "gz/common/Image.hh"
#include "gz/common/Material.hh"
#include "gz/common/Pbr.hh"

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;

class MaterialTest : public common::testing::AutoLogFixture { };

TEST_F(MaterialTest, Material)
{
  common::Material mat(math::Color(1.0f, 0.5f, 0.2f, 1.0f));
  EXPECT_TRUE(mat.Ambient() == math::Color(1.0f, 0.5f, 0.2f, 1.0f));
  EXPECT_TRUE(mat.Diffuse() == math::Color(1.0f, 0.5f, 0.2f, 1.0f));
  EXPECT_STREQ("gz_material_0", mat.Name().c_str());
  EXPECT_EQ(nullptr, mat.TextureData());

  auto texImg = std::make_shared<common::Image>();
  mat.SetTextureImage("texture_image", texImg);
  EXPECT_STREQ("texture_image", mat.TextureImage().c_str());
  EXPECT_EQ(texImg, mat.TextureData());

  mat.SetTextureImage("texture_image", "/path");
  std::string texturePath = common::joinPaths("/path", "..",
      "materials", "textures", "texture_image");
  EXPECT_STREQ(texturePath.c_str(), mat.TextureImage().c_str());

  texturePath = common::testing::TestFile("data", "box.dae");

  mat.SetTextureImage(texturePath, "bad_path");
  EXPECT_STREQ(texturePath.c_str(), mat.TextureImage().c_str());

  mat.SetAmbient(math::Color(0.1f, 0.2f, 0.3f, 0.4f));
  EXPECT_TRUE(mat.Ambient() == math::Color(0.1f, 0.2f, 0.3f, 0.4f));

  mat.SetDiffuse(math::Color(0.1f, 0.2f, 0.3f, 0.4f));
  EXPECT_TRUE(mat.Diffuse() == math::Color(0.1f, 0.2f, 0.3f, 0.4f));

  mat.SetSpecular(math::Color(0.1f, 0.2f, 0.3f, 0.4f));
  EXPECT_TRUE(mat.Specular() == math::Color(0.1f, 0.2f, 0.3f, 0.4f));

  mat.SetEmissive(math::Color(0.1f, 0.2f, 0.3f, 0.4f));
  EXPECT_TRUE(mat.Emissive() == math::Color(0.1f, 0.2f, 0.3f, 0.4f));

  mat.SetTransparency(0.2);
  EXPECT_DOUBLE_EQ(0.2, mat.Transparency());

  mat.SetAlphaFromTexture(true, 0.3, false);
  EXPECT_EQ(mat.TextureAlphaEnabled(), true);
  EXPECT_DOUBLE_EQ(mat.AlphaThreshold(), 0.3);
  EXPECT_EQ(mat.TwoSidedEnabled(), false);

  mat.SetRenderOrder(4.0);
  EXPECT_DOUBLE_EQ(4.0, mat.RenderOrder());

  mat.SetShininess(0.2);
  EXPECT_DOUBLE_EQ(0.2, mat.Shininess());

  mat.SetBlendFactors(.1, .5);
  double a, b;
  mat.BlendFactors(a, b);
  EXPECT_DOUBLE_EQ(.1, a);
  EXPECT_DOUBLE_EQ(0.5, b);

  mat.SetBlend(common::Material::MODULATE);
  EXPECT_EQ(common::Material::MODULATE, mat.Blend());

  mat.SetShade(common::Material::BLINN);
  EXPECT_EQ(common::Material::BLINN, mat.Shade());

  mat.SetPointSize(0.2);
  EXPECT_DOUBLE_EQ(0.2, mat.PointSize());

  mat.SetDepthWrite(false);
  EXPECT_FALSE(mat.DepthWrite());

  mat.SetLighting(true);
  EXPECT_TRUE(mat.Lighting());

  common::Pbr pbr;
  EXPECT_EQ(nullptr, mat.PbrMaterial());
  mat.SetPbrMaterial(pbr);
  EXPECT_NE(nullptr, mat.PbrMaterial());
  EXPECT_EQ(pbr, *mat.PbrMaterial());
}
