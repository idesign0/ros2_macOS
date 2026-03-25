/*
 * Copyright 2020 Open Source Robotics Foundation
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
#include <string>
#include <vector>
#include <gz/math/Vector3.hh>

#include "gz/common/Pbr.hh"

/// \brief Private data for Pbr class
class gz::common::Pbr::Implementation
{
  /// \brief Workflow type
  public: PbrType type = PbrType::NONE;

  /// \brief Albedo map
  public: std::string albedoMap = "";

  /// \brief Normal map
  public: std::string normalMap = "";

  /// \brief Pointer containing the normal map data, if loaded from memory
  public: std::shared_ptr<const Image> normalMapData = nullptr;

  /// \brief Normal map space
  public: NormalMapSpace normalMapSpace = NormalMapSpace::TANGENT;

  /// \brief Environment map
  public: std::string environmentMap = "";

  /// \brief Ambient occlusion map
  public: std::string ambientOcclusionMap = "";

  /// \brief Roughness map (metal workflow only)
  public: std::string roughnessMap = "";

  /// \brief Pointer containing the roughness map data,
  /// if loaded from memory
  public: std::shared_ptr<const Image> roughnessMapData = nullptr;

  /// \brief Metalness map (metal workflow only)
  public: std::string metalnessMap = "";

  /// \brief Pointer containing the metalness map data,
  /// if loaded from memory
  public: std::shared_ptr<const Image> metalnessMapData = nullptr;

  /// \brief Emissive map
  public: std::string emissiveMap = "";

  /// \brief Pointer containing the emissive map data,
  /// if loaded from memory
  public: std::shared_ptr<const Image> emissiveMapData = nullptr;

  /// \brief Light map
  public: std::string lightMap;

  /// \brief Pointer containing the light map data,
  /// if loaded from memory
  public: std::shared_ptr<const Image> lightMapData = nullptr;

  /// \brief Light map texture coordinate set
  public: unsigned int lightMapUvSet = 0u;

  /// \brief Roughness value (metal workflow only)
  public: double roughness = 0.5;

  /// \brief Metalness value (metal workflow only)
  public: double metalness = 0.0;

  /// \brief Specular map (specular workflow only)
  public: std::string specularMap = "";

  /// \brief Glossiness map (specular workflow only)
  public: std::string glossinessMap = "";

  /// \brief Glossiness value (specular workflow only)
  public: double glossiness = 0.0;
};


using namespace gz;
using namespace common;

/////////////////////////////////////////////////
Pbr::Pbr()
  : dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

//////////////////////////////////////////////////
bool Pbr::operator!=(const Pbr &_pbr) const
{
  return !(*this == _pbr);
}

/////////////////////////////////////////////////
bool Pbr::operator==(const Pbr &_pbr) const
{
  return (this->dataPtr->albedoMap == _pbr.dataPtr->albedoMap)
    && (this->dataPtr->normalMap == _pbr.dataPtr->normalMap)
    && (this->dataPtr->normalMapData == _pbr.dataPtr->normalMapData)
    && (this->dataPtr->metalnessMap == _pbr.dataPtr->metalnessMap)
    && (this->dataPtr->metalnessMapData == _pbr.dataPtr->metalnessMapData)
    && (this->dataPtr->roughnessMap == _pbr.dataPtr->roughnessMap)
    && (this->dataPtr->roughnessMapData == _pbr.dataPtr->roughnessMapData)
    && (this->dataPtr->glossinessMap == _pbr.dataPtr->glossinessMap)
    && (this->dataPtr->environmentMap == _pbr.dataPtr->environmentMap)
    && (this->dataPtr->emissiveMap == _pbr.dataPtr->emissiveMap)
    && (this->dataPtr->emissiveMapData == _pbr.dataPtr->emissiveMapData)
    && (this->dataPtr->lightMap == _pbr.dataPtr->lightMap)
    && (this->dataPtr->lightMapData == _pbr.dataPtr->lightMapData)
    && (this->dataPtr->ambientOcclusionMap ==
        _pbr.dataPtr->ambientOcclusionMap)
    && (gz::math::equal(
        this->dataPtr->metalness, _pbr.dataPtr->metalness))
    && (gz::math::equal(
        this->dataPtr->roughness, _pbr.dataPtr->roughness))
    && (gz::math::equal(
        this->dataPtr->glossiness, _pbr.dataPtr->glossiness));
}

//////////////////////////////////////////////////
std::string Pbr::AlbedoMap() const
{
  return this->dataPtr->albedoMap;
}

//////////////////////////////////////////////////
void Pbr::SetAlbedoMap(const std::string &_map)
{
  this->dataPtr->albedoMap = _map;
}

//////////////////////////////////////////////////
std::string Pbr::NormalMap() const
{
  return this->dataPtr->normalMap;
}

//////////////////////////////////////////////////
NormalMapSpace Pbr::NormalMapType() const
{
  return this->dataPtr->normalMapSpace;
}

//////////////////////////////////////////////////
std::shared_ptr<const Image> Pbr::NormalMapData() const
{
  return this->dataPtr->normalMapData;
}

//////////////////////////////////////////////////
void Pbr::SetNormalMap(const std::string &_map, NormalMapSpace _space,
                       const std::shared_ptr<const Image> &_img)
{
  this->dataPtr->normalMap = _map;
  this->dataPtr->normalMapSpace = _space;
  this->dataPtr->normalMapData = _img;
}

//////////////////////////////////////////////////
std::string Pbr::EnvironmentMap() const
{
  return this->dataPtr->environmentMap;
}

//////////////////////////////////////////////////
void Pbr::SetEnvironmentMap(const std::string &_map)
{
  this->dataPtr->environmentMap = _map;
}

//////////////////////////////////////////////////
std::string Pbr::AmbientOcclusionMap() const
{
  return this->dataPtr->ambientOcclusionMap;
}

//////////////////////////////////////////////////
void Pbr::SetAmbientOcclusionMap(const std::string &_map)
{
  this->dataPtr->ambientOcclusionMap = _map;
}

//////////////////////////////////////////////////
std::string Pbr::RoughnessMap() const
{
  return this->dataPtr->roughnessMap;
}

//////////////////////////////////////////////////
std::shared_ptr<const Image> Pbr::RoughnessMapData() const
{
  return this->dataPtr->roughnessMapData;
}

//////////////////////////////////////////////////
void Pbr::SetRoughnessMap(const std::string &_map,
                          const std::shared_ptr<const Image> &_img)
{
  this->dataPtr->roughnessMap = _map;
  this->dataPtr->roughnessMapData = _img;
}

//////////////////////////////////////////////////
std::string Pbr::MetalnessMap() const
{
  return this->dataPtr->metalnessMap;
}

//////////////////////////////////////////////////
std::shared_ptr<const Image> Pbr::MetalnessMapData() const
{
  return this->dataPtr->metalnessMapData;
}

//////////////////////////////////////////////////
void Pbr::SetMetalnessMap(const std::string &_map,
                          const std::shared_ptr<const Image> &_img)
{
  this->dataPtr->metalnessMap = _map;
  this->dataPtr->metalnessMapData = _img;
}

//////////////////////////////////////////////////
double Pbr::Metalness() const
{
  return this->dataPtr->metalness;
}

//////////////////////////////////////////////////
void Pbr::SetMetalness(double _metalness)
{
  this->dataPtr->metalness = _metalness;
}

//////////////////////////////////////////////////
double Pbr::Roughness() const
{
  return this->dataPtr->roughness;
}

//////////////////////////////////////////////////
void Pbr::SetRoughness(double _roughness)
{
  this->dataPtr->roughness = _roughness;
}

//////////////////////////////////////////////////
std::string Pbr::SpecularMap() const
{
  return this->dataPtr->specularMap;
}

//////////////////////////////////////////////////
void Pbr::SetSpecularMap(const std::string &_map)
{
  this->dataPtr->specularMap = _map;
}

//////////////////////////////////////////////////
std::string Pbr::GlossinessMap() const
{
  return this->dataPtr->glossinessMap;
}

//////////////////////////////////////////////////
void Pbr::SetGlossinessMap(const std::string &_map)
{
  this->dataPtr->glossinessMap = _map;
}

//////////////////////////////////////////////////
void Pbr::SetGlossiness(double _glossiness)
{
  this->dataPtr->glossiness = _glossiness;
}

//////////////////////////////////////////////////
double Pbr::Glossiness() const
{
  return this->dataPtr->glossiness;
}

//////////////////////////////////////////////////
std::string Pbr::EmissiveMap() const
{
  return this->dataPtr->emissiveMap;
}

//////////////////////////////////////////////////
std::shared_ptr<const Image> Pbr::EmissiveMapData() const
{
  return this->dataPtr->emissiveMapData;
}

//////////////////////////////////////////////////
void Pbr::SetEmissiveMap(const std::string &_map,
                         const std::shared_ptr<const Image> &_img)
{
  this->dataPtr->emissiveMap = _map;
  this->dataPtr->emissiveMapData = _img;
}

//////////////////////////////////////////////////
std::string Pbr::LightMap() const
{
  return this->dataPtr->lightMap;
}

//////////////////////////////////////////////////
std::shared_ptr<const Image> Pbr::LightMapData() const
{
  return this->dataPtr->lightMapData;
}

//////////////////////////////////////////////////
void Pbr::SetLightMap(const std::string &_map, unsigned int _uvSet,
                      const std::shared_ptr<const Image> &_img)
{
  this->dataPtr->lightMap = _map;
  this->dataPtr->lightMapUvSet = _uvSet;
  this->dataPtr->lightMapData = _img;
}

//////////////////////////////////////////////////
unsigned int Pbr::LightMapTexCoordSet() const
{
  return this->dataPtr->lightMapUvSet;
}

//////////////////////////////////////////////////
PbrType Pbr::Type() const
{
  return this->dataPtr->type;
}

//////////////////////////////////////////////////
void Pbr::SetType(PbrType _type)
{
  this->dataPtr->type = _type;
}

