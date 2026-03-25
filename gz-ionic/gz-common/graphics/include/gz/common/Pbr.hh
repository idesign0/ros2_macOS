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
#ifndef GZ_COMMON_PBR_HH_
#define GZ_COMMON_PBR_HH_

#include <memory>
#include <string>

#include <gz/utils/ImplPtr.hh>

#include <gz/common/graphics/Export.hh>
#include <gz/common/Image.hh>

namespace gz
{
namespace common
{
  /// \brief Type of PBR workflow.
  enum class PbrType : int
  {
    /// \brief No workflow
    NONE = 0,

    /// \brief Metal/Roughness workflow
    METAL = 1,

    /// \brief Specular/Glossiness workflow
    SPECULAR = 2,
  };

  /// \brief Space the normal map is defined in
  enum class NormalMapSpace: int
  {
    /// \brief Tangent space
    TANGENT = 0,

    /// \brief Object space
    OBJECT = 1,
  };

  /// \brief This class contains Physically-Based-Rendering (PBR) workflow
  /// properties.
  class GZ_COMMON_GRAPHICS_VISIBLE Pbr
  {
    /// \brief Default constructor
    public: Pbr();

    /// \brief Return true if both Pbr objects contain the same values.
    /// \param[_in] _pbr Pbr value to compare.
    /// \returen True if 'this' == _pbr.
    public: bool operator==(const Pbr &_pbr) const;

    /// \brief Return true this Pbr object does not contain the same
    /// values as the passed in parameter.
    /// \param[_in] _pbr Pbr value to compare.
    /// \returen True if 'this' != _pbr.
    public: bool operator!=(const Pbr &_pbr) const;

    /// \brief Get the albedo map filename. This will be an empty string if
    /// an albedo map has not been set.
    /// \return Filename of the albedo map, or empty string if an albedo map
    /// has not been specified.
    public: std::string AlbedoMap() const;

    /// \brief Set the albedo map filename.
    /// \param[in] _map Filename of the albedo map.
    public: void SetAlbedoMap(const std::string &_map);

    /// \brief Get the normal map filename. This will be an empty string if
    /// a normal map has not been set.
    /// \return Filename of the normal map, or empty string if a normal map
    /// has not been specified.
    public: std::string NormalMap() const;

    /// \brief Gets the normal map data,
    /// if the texture was loaded from memory, otherwise a nullptr
    /// \return A pointer to the image that was loaded from memory
    public: std::shared_ptr<const Image> NormalMapData() const;

    /// \brief Set the normal map filename.
    /// \param[in] _map Filename of the normal map.
    /// \param[in] _space Space that the normal map is defined in.
    /// Defaults to tangent space.
    /// \param[in] _img The image containing the texture if image has been
    /// loaded in memory
    public: void SetNormalMap(const std::string &_map,
        NormalMapSpace _space = NormalMapSpace::TANGENT,
        const std::shared_ptr<const Image> &_img = nullptr);

    /// \brief Get the normal map type, either tangent or object space
    /// \return Space that the normal map is defined in
    public: NormalMapSpace NormalMapType() const;

    /// \brief Get the environment map filename. This will be an empty string
    /// if an environment map has not been set.
    /// \return Filename of the environment map, or empty string if an
    /// environment map has not been specified.
    public: std::string EnvironmentMap() const;

    /// \brief Set the environment map filename.
    /// \param[in] _map Filename of the environment map.
    public: void SetEnvironmentMap(const std::string &_map);

    /// \brief Get the ambient occlusion map filename. This will be an empty
    /// string if an ambient occlusion map has not been set.
    /// \return Filename of the ambient occlusion map, or empty string if an
    /// ambient occlusion map has not been specified.
    public: std::string AmbientOcclusionMap() const;

    /// \brief Set the ambient occlusion map filename.
    /// \param[in] _map Filename of the ambient occlusion map.
    public: void SetAmbientOcclusionMap(const std::string &_map);

    /// \brief Get the roughness map filename for metal workflow. This will be
    /// an empty string if a roughness map has not been set.
    /// \return Filename of the roughness map, or empty string if a roughness
    /// map has not been specified.
    public: std::string RoughnessMap() const;

    /// \brief Set the roughness map filename for metal workflow.
    /// \param[in] _map Filename of the roughness map.
    /// \param[in] _img The image containing the texture if image has been
    /// loaded in memory
    public: void SetRoughnessMap(const std::string &_map,
        const std::shared_ptr<const Image> &_img = nullptr);

    /// \brief Gets the roughness map data,
    /// if the texture was loaded from memory, otherwise a nullptr
    /// \return A pointer to the image that was loaded from memory
    public: std::shared_ptr<const Image> RoughnessMapData() const;

    /// \brief Get the metalness map filename for metal workflow. This will be
    /// an empty string if a metalness map has not been set.
    /// \return Filename of the metalness map, or empty string if a metalness
    /// map has not been specified.
    public: std::string MetalnessMap() const;

    /// \brief Set the metalness map filename for metal workflow.
    /// \param[in] _map Filename of the metalness map.
    /// \param[in] _img The image containing the texture if image has been
    /// loaded in memory
    public: void SetMetalnessMap(const std::string &_map,
        const std::shared_ptr<const Image> &_img = nullptr);

    /// \brief Gets the metalness map data,
    /// if the texture was loaded from memory, otherwise a nullptr
    /// \return A pointer to the image that was loaded from memory
    public: std::shared_ptr<const Image> MetalnessMapData() const;

    /// \brief Get the emissive map filename. This will be an empty string
    /// if an emissive map has not been set.
    /// \return Filename of the emissive map, or empty string if an emissive
    /// map has not been specified.
    public: std::string EmissiveMap() const;

    /// \brief Set the emissive map filename.
    /// \param[in] _map Filename of the emissive map.
    /// \param[in] _img The image containing the texture if image has been
    /// loaded in memory
    public: void SetEmissiveMap(const std::string &_map,
        const std::shared_ptr<const Image> &_img = nullptr);

    /// \brief Gets the emissive map data,
    /// if the texture was loaded from memory, otherwise a nullptr
    /// \return A pointer to the image that was loaded from memory
    public: std::shared_ptr<const Image> EmissiveMapData() const;

    /// \brief Get the light map filename. This will be an empty string
    /// if an light map has not been set.
    /// \return Filename of the light map, or empty string if a light
    /// map has not been specified.
    public: std::string LightMap() const;

    /// \brief Gets the light map data,
    /// if the texture was loaded from memory, otherwise a nullptr
    /// \return A pointer to the image that was loaded from memory
    public: std::shared_ptr<const Image> LightMapData() const;

    /// \brief Set the light map filename.
    /// \param[in] _map Filename of the light map.
    /// \param[in] _uvSet Index of the texture coordinate set
    /// \param[in] _img The image containing the texture if image has been
    /// loaded in memory
    public: void SetLightMap(const std::string &_map, unsigned int _uvSet = 0u,
        const std::shared_ptr<const Image> &_img = nullptr);

    /// \brief Get the light map texture coordinate set.
    /// \return Index of the light map texture coordinate set
    public: unsigned int LightMapTexCoordSet() const;

    /// \brief Get the metalness value of the material for metal workflow
    /// \return metalness value of the material
    public: double Metalness() const;

    /// \brief Set the metalness value of the material for metal workflow.
    /// \param[in] _value Metalness value.
    public: void SetMetalness(double _value);

    /// \brief Get the roughness value of the material for metal workflow
    /// \return roughness value of the material
    public: double Roughness() const;

    /// \brief Set the roughness value of the material for metal workflow.
    /// \param[in] _value Roughness value.
    public: void SetRoughness(double _value);

    /// \brief Get the glossiness map filename for specular workflow. This will
    /// be an empty string if a glossiness map has not been set.
    /// \return Filename of the glossiness map, or empty string if a glossiness
    /// map has not been specified.
    public: std::string GlossinessMap() const;

    /// \brief Set the glossiness map filename for specular workflow.
    /// \param[in] _map Filename of the glossiness map.
    public: void SetGlossinessMap(const std::string &_map);

    /// \brief Get the glossiness value of the material for specular workflow
    /// \return Glossiness value of the material
    public: double Glossiness() const;

    /// \brief Set the glossiness value of the material for specular workflow.
    /// \param[in] _value Glossiness value.
    public: void SetGlossiness(double _value);

    /// \brief Get the specular map filename for specular workflow. This will
    /// be an empty string if a specular map has not been set.
    /// \return Filename of the specular map, or empty string if a specular
    /// map has not been specified.
    public: std::string SpecularMap() const;

    /// \brief Set the specular map filename for specular workflow.
    /// \param[in] _map Filename of the specular map.
    public: void SetSpecularMap(const std::string &_map);

    /// \brief Get the workflow type.
    /// \return PBR workflow type.
    public: PbrType Type() const;

    /// \brief Set the PBR workflow to use
    /// \param[in] _type Workflow type to use
    public: void SetType(PbrType _type);

    /// \brief Private data pointer.
    GZ_UTILS_IMPL_PTR(dataPtr)
  };
}
}
#endif
