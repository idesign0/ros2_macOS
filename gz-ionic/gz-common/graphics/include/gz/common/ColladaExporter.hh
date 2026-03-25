/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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
#ifndef GZ_COMMON_COLLADAEXPORTER_HH_
#define GZ_COMMON_COLLADAEXPORTER_HH_

#include <memory>
#include <string>
#include <vector>

#include <gz/common/MeshExporter.hh>
#include <gz/common/graphics/Export.hh>

#include <gz/math/Color.hh>
#include <gz/math/Matrix4.hh>

#include <gz/utils/ImplPtr.hh>

namespace gz
{
  namespace common
  {
    /// \brief This struct contains light data specifically for collada export
    /// Defaults set based on collada 1.4 specifications
    struct ColladaLight
    {
      /// \brief Name of the light
      std::string name;

      /// \brief Type of the light. Either "point", "directional" or "spot"
      std::string type;

      /// \brief Light direction (directional/spot lights only)
      math::Vector3d direction;

      /// \brief Light position (non directional lights only)
      math::Vector3d position;

      /// \brief Light diffuse color
      math::Color diffuse;

      /// \brief Constant attentuation
      double constantAttenuation = 1.0;

      /// \brief Linear attentuation
      double linearAttenuation = 0.0;

      /// \brief Quadratic attentuation
      double quadraticAttenuation = 0.0;

      /// \brief Falloff angle in degrees
      double falloffAngleDeg = 180.0;

      /// \brief Fallof exponent
      double falloffExponent = 0.0;
    };

    /// \brief Class used to export Collada mesh files
    class GZ_COMMON_GRAPHICS_VISIBLE ColladaExporter : public MeshExporter
    {
      /// \brief Constructor
      public: ColladaExporter();

      /// \brief Destructor
      public: virtual ~ColladaExporter();

      /// \brief Export a mesh to a file
      /// \param[in] _mesh Pointer to the mesh to be exported
      /// \param[in] _filename Exported file's path and name
      /// \param[in] _exportTextures True to export texture images to
      /// '../materials/textures' folder
      public: virtual void Export(const Mesh *_mesh,
          const std::string &_filename, bool _exportTextures = false);

      /// \brief Export a mesh to a file
      /// \param[in] _mesh Pointer to the mesh to be exported
      /// \param[in] _filename Exported file's path and name
      /// \param[in] _exportTextures True to export texture images to
      /// '../materials/textures' folder
      /// \param[in] _submeshToMatrix Matrices of submeshes
      public: void Export(const Mesh *_mesh,
          const std::string &_filename, bool _exportTextures,
          const std::vector<math::Matrix4d> &_submeshToMatrix);

      /// \brief Export a mesh to a file
      /// \param[in] _mesh Pointer to the mesh to be exported
      /// \param[in] _filename Exported file's path and name
      /// \param[in] _exportTextures True to export texture images to
      /// '../materials/textures' folder
      /// \param[in] _submeshToMatrix Matrices of submeshes
      /// \param[in] _lights List of lights to export
      public: void Export(const Mesh *_mesh,
          const std::string &_filename, bool _exportTextures,
          const std::vector<math::Matrix4d> &_submeshToMatrix,
          const std::vector<ColladaLight> &_lights);

      /// \brief Pointer to private data.
      GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
}
#endif
