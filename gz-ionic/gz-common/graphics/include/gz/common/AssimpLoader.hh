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
#ifndef GZ_COMMON_ASSIMPLOADER_HH_
#define GZ_COMMON_ASSIMPLOADER_HH_

#include <string>
#include <gz/common/graphics/Export.hh>
#include <gz/common/MeshLoader.hh>

#include <gz/utils/ImplPtr.hh>

namespace gz
{
  namespace common
  {
    /// \class AssimpLoader AssimpLoader.hh gz/common/AssimpLoader.hh
    /// \brief Class used to load mesh files using the assimp lodaer
    class GZ_COMMON_GRAPHICS_VISIBLE AssimpLoader : public MeshLoader
    {
      /// \brief Constructor
      public: AssimpLoader();

      /// \brief Destructor
      public: virtual ~AssimpLoader();

      /// \brief Load a mesh
      /// \param[in] _filename Mesh file to load
      /// \return Pointer to a new Mesh
      public: virtual Mesh *Load(const std::string &_filename) override;

      /// \internal
      /// \brief Pointer to private data.
      GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
    };
  }
}
#endif
