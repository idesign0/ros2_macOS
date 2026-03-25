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
#include <string>

#include "CommonRenderingTest.hh"

#include <gz/common/Material.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/Material.hh"
#include "gz/rendering/ShaderType.hh"
#include "gz/rendering/Scene.hh"

#include <gz/utils/ExtraTestMacros.hh>

using namespace gz;
using namespace rendering;

class MaterialTest : public CommonRenderingTest
{
  public: const std::string TEST_MEDIA_PATH =
        common::joinPaths(std::string(PROJECT_SOURCE_PATH),
        "test", "media", "materials", "textures");
};

/////////////////////////////////////////////////
TEST_F(MaterialTest, MaterialProperties)
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  MaterialPtr material = scene->CreateMaterial();
  ASSERT_NE(nullptr, material);

  material = scene->CreateMaterial("unique");
  ASSERT_NE(nullptr, scene);
  EXPECT_TRUE(scene->MaterialRegistered("unique"));

  // ambient
  math::Color ambient(0.5f, 0.2f, 0.4f, 1.0f);
  material->SetAmbient(ambient);
  EXPECT_EQ(ambient, material->Ambient());

  ambient.Set(0.55f, 0.22f, 0.44f, 1.0f);
  material->SetAmbient(ambient.R(), ambient.G(), ambient.B(), ambient.A());
  EXPECT_EQ(ambient, material->Ambient());

  // diffuse
  math::Color diffuse(0.1f, 0.9f, 0.3f, 1.0f);
  material->SetDiffuse(diffuse);
  EXPECT_EQ(diffuse, material->Diffuse());

  diffuse.Set(0.11f, 0.99f, 0.33f, 1.0f);
  material->SetDiffuse(diffuse.R(), diffuse.G(), diffuse.B(), diffuse.A());
  EXPECT_EQ(diffuse, material->Diffuse());

  // specular
  math::Color specular(0.8f, 0.7f, 0.0f, 1.0f);
  material->SetSpecular(specular);
  EXPECT_EQ(specular, material->Specular());

  specular.Set(0.88f, 0.77f, 0.66f, 1.0f);
  material->SetSpecular(specular.R(), specular.G(), specular.B(), specular.A());
  EXPECT_EQ(specular, material->Specular());

  // emissive
  math::Color emissive(0.6f, 0.4f, 0.2f, 1.0f);
  material->SetEmissive(emissive);
  EXPECT_EQ(emissive, material->Emissive());

  emissive.Set(0.66f, 0.44f, 0.22f, 1.0f);
  material->SetEmissive(emissive.R(), emissive.G(), emissive.B(), emissive.A());
  EXPECT_EQ(emissive, material->Emissive());

  // shininess
  double shininess = 0.8;
  material->SetShininess(shininess);
  EXPECT_DOUBLE_EQ(shininess, material->Shininess());

  // transparency
  double transparency = 0.3;
  material->SetTransparency(transparency);
  EXPECT_DOUBLE_EQ(transparency, material->Transparency());

  // alpha from texture
  bool alphaFromTexture = true;
  double alphaThreshold = 0.9;
  bool twoSidedEnabled = false;
  material->SetAlphaFromTexture(alphaFromTexture, alphaThreshold,
      twoSidedEnabled);
  EXPECT_EQ(material->TextureAlphaEnabled(), alphaFromTexture);
  EXPECT_DOUBLE_EQ(material->AlphaThreshold(), alphaThreshold);
  EXPECT_EQ(material->TwoSidedEnabled(), twoSidedEnabled);

  // reflectivity
  double reflectivity = 0.5;
  material->SetReflectivity(reflectivity);
  EXPECT_DOUBLE_EQ(reflectivity, material->Reflectivity());

  // cast shadows
  bool castShadows = false;
  material->SetCastShadows(castShadows);
  EXPECT_EQ(castShadows, material->CastShadows());

  // receive shadows
  bool receiveShadows = false;
  material->SetReceiveShadows(receiveShadows);
  EXPECT_EQ(receiveShadows, material->ReceiveShadows());

  // reflection
  bool reflectionEnabled = false;
  material->SetReflectionEnabled(reflectionEnabled);
  EXPECT_EQ(reflectionEnabled, material->ReflectionEnabled());

  // lighting
  bool lightingEnabled = false;
  material->SetLightingEnabled(lightingEnabled);
  EXPECT_EQ(lightingEnabled, material->LightingEnabled());

  // depth check
  bool depthCheckEnabled = false;
  material->SetDepthCheckEnabled(depthCheckEnabled);
  EXPECT_EQ(depthCheckEnabled, material->DepthCheckEnabled());

  // depth write
  bool depthWriteEnabled = false;
  material->SetDepthWriteEnabled(depthWriteEnabled);
  EXPECT_EQ(depthWriteEnabled, material->DepthWriteEnabled());

  // texture
  std::string textureName =
      common::joinPaths(TEST_MEDIA_PATH, "texture.png");
  material->SetTexture(textureName);
  EXPECT_EQ(textureName, material->Texture());
  EXPECT_TRUE(material->HasTexture());

  material->ClearTexture();
  EXPECT_FALSE(material->HasTexture());

  // Set the texture from binary data
  auto textureImg = std::make_shared<gz::common::Image>();
  textureImg->Load(textureName);
  ASSERT_NE(textureImg, nullptr);
  ASSERT_TRUE(textureImg->Valid());
  material->SetTexture(textureName, textureImg);
  EXPECT_EQ(textureName, material->Texture());
  EXPECT_EQ(material->TextureData(), textureImg);

  material->ClearTexture();
  EXPECT_FALSE(material->HasTexture());
  EXPECT_EQ(material->TextureData(), nullptr);

  std::string noSuchTextureName = "no_such_texture.png";
  material->SetTexture(noSuchTextureName, nullptr);
  EXPECT_EQ(noSuchTextureName, material->Texture());
  EXPECT_TRUE(material->HasTexture());

  // normal map
  std::string normalMapName = textureName;
  material->SetNormalMap(normalMapName);
  EXPECT_EQ(normalMapName, material->NormalMap());
  EXPECT_TRUE(material->HasNormalMap());

  material->ClearNormalMap();
  EXPECT_FALSE(material->HasNormalMap());

  // Set the normal map from binary data
  material->SetNormalMap(textureName, textureImg);
  EXPECT_EQ(textureName, material->NormalMap());
  EXPECT_EQ(material->NormalMapData(), textureImg);

  material->ClearNormalMap();
  EXPECT_FALSE(material->HasNormalMap());
  EXPECT_EQ(material->NormalMapData(), nullptr);

  std::string noSuchNormalMapName = "no_such_normal.png";
  material->SetNormalMap(noSuchNormalMapName);
  EXPECT_EQ(noSuchNormalMapName, material->NormalMap());
  EXPECT_TRUE(material->HasNormalMap());

  if (material->Type() == MaterialType::MT_PBS)
  {
    // metalness map
    std::string metalnessMapName = textureName;
    material->SetMetalnessMap(metalnessMapName);
    EXPECT_EQ(metalnessMapName, material->MetalnessMap());
    EXPECT_TRUE(material->HasMetalnessMap());

    material->ClearMetalnessMap();
    EXPECT_FALSE(material->HasMetalnessMap());

    // Set the metalness map from binary data
    material->SetMetalnessMap(textureName, textureImg);
    EXPECT_EQ(textureName, material->MetalnessMap());
    EXPECT_EQ(material->MetalnessMapData(), textureImg);

    material->ClearMetalnessMap();
    EXPECT_FALSE(material->HasMetalnessMap());
    EXPECT_EQ(material->MetalnessMapData(), nullptr);

    std::string noSuchMetalnessMapName = "no_such_metalness.png";
    material->SetMetalnessMap(noSuchMetalnessMapName);
    EXPECT_EQ(noSuchMetalnessMapName, material->MetalnessMap());
    EXPECT_TRUE(material->HasMetalnessMap());

    // roughness map
    std::string roughnessMapName = textureName;
    material->SetRoughnessMap(roughnessMapName);
    EXPECT_EQ(roughnessMapName, material->RoughnessMap());
    EXPECT_TRUE(material->HasRoughnessMap());

    material->ClearRoughnessMap();
    EXPECT_FALSE(material->HasRoughnessMap());

    // Set the roughness map from binary data
    material->SetRoughnessMap(textureName, textureImg);
    EXPECT_EQ(textureName, material->RoughnessMap());
    EXPECT_EQ(material->RoughnessMapData(), textureImg);

    material->ClearRoughnessMap();
    EXPECT_FALSE(material->HasRoughnessMap());
    EXPECT_EQ(material->RoughnessMapData(), nullptr);

    std::string noSuchRoughnessMapName = "no_such_roughness.png";
    material->SetRoughnessMap(noSuchRoughnessMapName);
    EXPECT_EQ(noSuchRoughnessMapName, material->RoughnessMap());
    EXPECT_TRUE(material->HasRoughnessMap());

    // environment map
    std::string environmentMapName = textureName;
    material->SetEnvironmentMap(environmentMapName);
    EXPECT_EQ(environmentMapName, material->EnvironmentMap());
    EXPECT_TRUE(material->HasEnvironmentMap());

    material->ClearEnvironmentMap();
    EXPECT_FALSE(material->HasEnvironmentMap());

    // Set the environment map from binary data
    material->SetEnvironmentMap(environmentMapName, textureImg);
    EXPECT_EQ(environmentMapName, material->EnvironmentMap());
    EXPECT_EQ(material->EnvironmentMapData(), textureImg);

    material->ClearEnvironmentMap();
    EXPECT_FALSE(material->HasEnvironmentMap());
    EXPECT_EQ(material->EnvironmentMapData(), nullptr);

    std::string noSuchEnvironmentMapName = "no_such_environment.png";
    material->SetEnvironmentMap(noSuchEnvironmentMapName);
    EXPECT_EQ(noSuchEnvironmentMapName, material->EnvironmentMap());
    EXPECT_TRUE(material->HasEnvironmentMap());

    // emissive map
    std::string emissiveMapName = textureName;
    material->SetEmissiveMap(emissiveMapName);
    EXPECT_EQ(emissiveMapName, material->EmissiveMap());
    EXPECT_TRUE(material->HasEmissiveMap());

    material->ClearEmissiveMap();
    EXPECT_FALSE(material->HasEmissiveMap());

    // Set the emissive map from binary data
    material->SetEmissiveMap(emissiveMapName, textureImg);
    EXPECT_EQ(emissiveMapName, material->EmissiveMap());
    EXPECT_EQ(material->EmissiveMapData(), textureImg);

    material->ClearEmissiveMap();
    EXPECT_FALSE(material->HasEmissiveMap());
    EXPECT_EQ(material->EmissiveMapData(), nullptr);

    std::string noSuchEmissiveMapName = "no_such_emissive.png";
    material->SetEmissiveMap(noSuchEmissiveMapName, nullptr);
    EXPECT_EQ(noSuchEmissiveMapName, material->EmissiveMap());
    EXPECT_TRUE(material->HasEmissiveMap());

    // light map
    std::string lightMapName = textureName;
    material->SetLightMap(lightMapName, 1u);
    EXPECT_EQ(lightMapName, material->LightMap());
    EXPECT_EQ(1u, material->LightMapTexCoordSet());
    EXPECT_TRUE(material->HasLightMap());

    material->ClearLightMap();
    EXPECT_FALSE(material->HasLightMap());

    // Set the light map from binary data
    material->SetLightMap(lightMapName, textureImg, 1);
    EXPECT_EQ(1u, material->LightMapTexCoordSet());
    EXPECT_EQ(lightMapName, material->LightMap());
    EXPECT_EQ(material->LightMapData(), textureImg);

    material->ClearLightMap();
    EXPECT_FALSE(material->HasLightMap());
    EXPECT_EQ(material->LightMapData(), nullptr);

    std::string noSuchLightMapName = "no_such_light.png";
    material->SetLightMap(noSuchLightMapName, nullptr);
    EXPECT_EQ(noSuchLightMapName, material->LightMap());
    EXPECT_TRUE(material->HasLightMap());

    // roughness
    float roughness = 0.3f;
    material->SetRoughness(roughness);
    EXPECT_FLOAT_EQ(roughness, material->Roughness());

    // metalness
    float metalness = 0.9f;
    material->SetMetalness(metalness);
    EXPECT_FLOAT_EQ(metalness, material->Metalness());
  }

  // shader type
  enum ShaderType shaderType = ShaderType::ST_PIXEL;
  material->SetShaderType(shaderType);
  EXPECT_EQ(shaderType, material->ShaderType());

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(MaterialTest, GZ_UTILS_TEST_DISABLED_ON_WIN32(Copy))
{
  ScenePtr scene = engine->CreateScene("copy_scene");
  ASSERT_NE(nullptr, scene);

  MaterialPtr material = scene->CreateMaterial();
  ASSERT_NE(nullptr, material);

  math::Color ambient(0.5f, 0.2f, 0.4f, 1.0f);
  math::Color diffuse(0.1f, 0.9f, 0.3f, 1.0f);
  math::Color specular(0.8f, 0.7f, 0.0f, 1.0f);
  math::Color emissive(0.6f, 0.4f, 0.2f, 1.0f);
  double shininess = 0.8;
  double transparency = 0.3;
  bool alphaFromTexture = true;
  double alphaThreshold = 0.9;
  bool twoSidedEnabled = false;
  double reflectivity = 0.5;
  bool castShadows = false;
  bool receiveShadows = false;
  bool reflectionEnabled = true;
  bool lightingEnabled = false;
  bool depthCheckEnabled = false;
  bool depthWriteEnabled = false;
  float roughness = 0.5f;
  float metalness = 0.1f;

  std::string textureName =
    common::joinPaths(TEST_MEDIA_PATH, "texture.png");
  auto textureImg = std::make_shared<gz::common::Image>();
  textureImg->Load(textureName);
  ASSERT_NE(textureImg, nullptr);
  ASSERT_TRUE(textureImg->Valid());
  std::string normalMapName = textureName;
  std::string roughnessMapName = "roughness_" + textureName;
  std::string metalnessMapName = "metalness_" + textureName;
  std::string envMapName = "env_" + textureName;
  std::string emissiveMapName = "emissive_" + textureName;
  std::string lightMapName = "light_" + textureName;
  enum ShaderType shaderType = ShaderType::ST_PIXEL;

  material->SetAmbient(ambient);
  material->SetDiffuse(diffuse);
  material->SetSpecular(specular);
  material->SetEmissive(emissive);
  material->SetShininess(shininess);
  material->SetTransparency(transparency);
  material->SetAlphaFromTexture(alphaFromTexture, alphaThreshold,
      twoSidedEnabled);
  material->SetReflectivity(reflectivity);
  material->SetCastShadows(castShadows);
  material->SetReceiveShadows(receiveShadows);
  material->SetReflectionEnabled(reflectionEnabled);
  material->SetLightingEnabled(lightingEnabled);
  material->SetDepthCheckEnabled(depthCheckEnabled);
  material->SetDepthWriteEnabled(depthWriteEnabled);
  material->SetTexture(textureName, textureImg);
  material->SetNormalMap(normalMapName, textureImg);
  material->SetShaderType(shaderType);
  material->SetRoughnessMap(roughnessMapName, textureImg);
  material->SetMetalnessMap(metalnessMapName, textureImg);
  material->SetEnvironmentMap(envMapName, textureImg);
  material->SetEmissiveMap(emissiveMapName, textureImg);
  material->SetLightMap(lightMapName, textureImg, 1u);
  material->SetRoughness(roughness);
  material->SetMetalness(metalness);

  // test cloning a material
  MaterialPtr clone = material->Clone("clone");
  EXPECT_TRUE(scene->MaterialRegistered("clone"));
  EXPECT_EQ(ambient, clone->Ambient());
  EXPECT_EQ(diffuse, clone->Diffuse());
  EXPECT_EQ(specular, clone->Specular());
  EXPECT_EQ(emissive, clone->Emissive());
  EXPECT_DOUBLE_EQ(shininess, clone->Shininess());
  EXPECT_DOUBLE_EQ(transparency, clone->Transparency());
  EXPECT_EQ(alphaFromTexture, clone->TextureAlphaEnabled());
  EXPECT_DOUBLE_EQ(alphaThreshold, clone->AlphaThreshold());
  EXPECT_EQ(twoSidedEnabled, clone->TwoSidedEnabled());
  EXPECT_DOUBLE_EQ(reflectivity, clone->Reflectivity());
  EXPECT_EQ(castShadows, clone->CastShadows());
  EXPECT_EQ(receiveShadows, clone->ReceiveShadows());
  EXPECT_EQ(reflectionEnabled, clone->ReflectionEnabled());
  EXPECT_EQ(lightingEnabled, clone->LightingEnabled());
  EXPECT_EQ(depthCheckEnabled, clone->DepthCheckEnabled());
  EXPECT_EQ(depthWriteEnabled, clone->DepthWriteEnabled());
  EXPECT_EQ(textureName, clone->Texture());
  EXPECT_EQ(textureImg, clone->TextureData());
  EXPECT_TRUE(clone->HasTexture());
  EXPECT_EQ(normalMapName, clone->NormalMap());
  EXPECT_EQ(textureImg, clone->NormalMapData());
  EXPECT_TRUE(clone->HasNormalMap());
  EXPECT_EQ(shaderType, clone->ShaderType());
  if (material->Type() == MaterialType::MT_PBS)
  {
    EXPECT_FLOAT_EQ(roughness, clone->Roughness());
    EXPECT_FLOAT_EQ(metalness, clone->Metalness());
    EXPECT_EQ(roughnessMapName, clone->RoughnessMap());
    EXPECT_EQ(textureImg, clone->RoughnessMapData());
    EXPECT_EQ(metalnessMapName, clone->MetalnessMap());
    EXPECT_EQ(textureImg, clone->MetalnessMapData());
    EXPECT_EQ(envMapName, clone->EnvironmentMap());
    EXPECT_EQ(textureImg, clone->EnvironmentMapData());
    EXPECT_EQ(emissiveMapName, clone->EmissiveMap());
    EXPECT_EQ(textureImg, clone->EmissiveMapData());
    EXPECT_EQ(lightMapName, clone->LightMap());
    EXPECT_EQ(textureImg, clone->LightMapData());
    EXPECT_EQ(1u, clone->LightMapTexCoordSet());
  }

  // test copying a material
  MaterialPtr copy = scene->CreateMaterial("copy");
  EXPECT_TRUE(scene->MaterialRegistered("copy"));
  copy->CopyFrom(material);
  EXPECT_EQ(ambient, copy->Ambient());
  EXPECT_EQ(diffuse, copy->Diffuse());
  EXPECT_EQ(specular, copy->Specular());
  EXPECT_EQ(emissive, copy->Emissive());
  EXPECT_DOUBLE_EQ(shininess, copy->Shininess());
  EXPECT_DOUBLE_EQ(transparency, copy->Transparency());
  EXPECT_EQ(alphaFromTexture, copy->TextureAlphaEnabled());
  EXPECT_DOUBLE_EQ(alphaThreshold, copy->AlphaThreshold());
  EXPECT_EQ(twoSidedEnabled, copy->TwoSidedEnabled());
  EXPECT_DOUBLE_EQ(reflectivity, copy->Reflectivity());
  EXPECT_EQ(castShadows, copy->CastShadows());
  EXPECT_EQ(receiveShadows, copy->ReceiveShadows());
  EXPECT_EQ(reflectionEnabled, copy->ReflectionEnabled());
  EXPECT_EQ(lightingEnabled, copy->LightingEnabled());
  EXPECT_EQ(textureName, copy->Texture());
  EXPECT_EQ(textureImg, copy->TextureData());
  EXPECT_TRUE(copy->HasTexture());
  EXPECT_EQ(normalMapName, copy->NormalMap());
  EXPECT_EQ(textureImg, copy->NormalMapData());
  EXPECT_TRUE(copy->HasNormalMap());
  EXPECT_EQ(shaderType, copy->ShaderType());
  if (material->Type() == MaterialType::MT_PBS)
  {
    EXPECT_FLOAT_EQ(roughness, copy->Roughness());
    EXPECT_FLOAT_EQ(metalness, copy->Metalness());
    EXPECT_EQ(roughnessMapName, copy->RoughnessMap());
    EXPECT_EQ(textureImg, copy->RoughnessMapData());
    EXPECT_EQ(metalnessMapName, copy->MetalnessMap());
    EXPECT_EQ(textureImg, copy->MetalnessMapData());
    EXPECT_EQ(envMapName, copy->EnvironmentMap());
    EXPECT_EQ(textureImg, copy->EnvironmentMapData());
    EXPECT_EQ(emissiveMapName, copy->EmissiveMap());
    EXPECT_EQ(textureImg, copy->EmissiveMapData());
    EXPECT_EQ(lightMapName, copy->LightMap());
    EXPECT_EQ(textureImg, copy->LightMapData());
    EXPECT_EQ(1u, copy->LightMapTexCoordSet());
  }

  // test copying from a common material
  // common::Material currently only has a subset of material properties
  common::Material comMat;
  comMat.SetAmbient(ambient);
  comMat.SetDiffuse(diffuse);
  comMat.SetSpecular(specular);
  comMat.SetEmissive(emissive);
  comMat.SetShininess(shininess);
  comMat.SetTransparency(transparency);
  comMat.SetAlphaFromTexture(alphaFromTexture, alphaThreshold,
      twoSidedEnabled);
  comMat.SetLighting(lightingEnabled);
  comMat.SetTextureImage(textureName, textureImg);
  common::Pbr pbr;
  pbr.SetType(common::PbrType::METAL);
  pbr.SetRoughness(roughness);
  pbr.SetMetalness(metalness);
  pbr.SetAlbedoMap(textureName);
  pbr.SetNormalMap(normalMapName,
      common::NormalMapSpace::TANGENT, textureImg);
  pbr.SetRoughnessMap(roughnessMapName, textureImg);
  pbr.SetMetalnessMap(metalnessMapName, textureImg);
  pbr.SetEmissiveMap(emissiveMapName, textureImg);
  // TODO(anyone) Add support for binary environment maps to pbr material
  // when a format that supports it is introduced
  pbr.SetEnvironmentMap(envMapName);
  pbr.SetLightMap(lightMapName, 1u, textureImg);
  comMat.SetPbrMaterial(pbr);

  MaterialPtr comCopy = scene->CreateMaterial("comCopy");
  EXPECT_TRUE(scene->MaterialRegistered("comCopy"));
  comCopy->CopyFrom(comMat);
  EXPECT_EQ(ambient, comCopy->Ambient());
  EXPECT_EQ(diffuse, comCopy->Diffuse());
  EXPECT_EQ(specular, comCopy->Specular());
  EXPECT_EQ(emissive, comCopy->Emissive());
  EXPECT_DOUBLE_EQ(shininess, comCopy->Shininess());
  EXPECT_DOUBLE_EQ(transparency, comCopy->Transparency());
  EXPECT_EQ(alphaFromTexture, comCopy->TextureAlphaEnabled());
  EXPECT_DOUBLE_EQ(alphaThreshold, comCopy->AlphaThreshold());
  EXPECT_EQ(twoSidedEnabled, comCopy->TwoSidedEnabled());
  // \todo(anyon) add reflectivity to common::Material?
  // EXPECT_DOUBLE_EQ(reflectivity, comCopy->Reflectivity());
  EXPECT_EQ(lightingEnabled, comCopy->LightingEnabled());
  EXPECT_EQ(textureName, comCopy->Texture());
  EXPECT_EQ(textureImg, comCopy->TextureData());
  EXPECT_TRUE(comCopy->HasTexture());
  if (material->Type() == MaterialType::MT_PBS)
  {
    EXPECT_DOUBLE_EQ(roughness, comCopy->Roughness());
    EXPECT_DOUBLE_EQ(metalness, comCopy->Metalness());
    EXPECT_TRUE(comCopy->HasNormalMap());
    EXPECT_EQ(normalMapName, comCopy->NormalMap());
    EXPECT_EQ(textureImg, comCopy->NormalMapData());
    EXPECT_TRUE(comCopy->HasRoughnessMap());
    EXPECT_EQ(roughnessMapName, comCopy->RoughnessMap());
    EXPECT_EQ(textureImg, comCopy->RoughnessMapData());
    EXPECT_TRUE(comCopy->HasMetalnessMap());
    EXPECT_EQ(metalnessMapName, comCopy->MetalnessMap());
    EXPECT_EQ(textureImg, comCopy->MetalnessMapData());
    EXPECT_TRUE(comCopy->HasEmissiveMap());
    EXPECT_EQ(emissiveMapName, comCopy->EmissiveMap());
    EXPECT_EQ(textureImg, comCopy->EmissiveMapData());
    EXPECT_TRUE(comCopy->HasLightMap());
    EXPECT_EQ(lightMapName, comCopy->LightMap());
    EXPECT_EQ(1u, comCopy->LightMapTexCoordSet());
    EXPECT_EQ(textureImg, comCopy->LightMapData());
    EXPECT_TRUE(comCopy->HasEnvironmentMap());
    EXPECT_EQ(envMapName, comCopy->EnvironmentMap());
  }

  // Clean up
  engine->DestroyScene(scene);
}
