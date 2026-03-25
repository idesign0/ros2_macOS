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

#ifndef GZ_COMMON_OBJLOADER_HH_
#define GZ_COMMON_OBJLOADER_HH_

#include <string>
#include <memory>

#include <gz/utils/ImplPtr.hh>

#include <gz/common/MeshLoader.hh>
#include <gz/common/graphics/Export.hh>

namespace gz
{
  namespace common
  {
    // class OBJ Loader private class;
    class OBJLoaderPrivate;

    /// \brief Class used to load obj mesh files
    class GZ_COMMON_GRAPHICS_VISIBLE OBJLoader : public MeshLoader
    {
      /// \brief Constructor
      public: OBJLoader();

      /// \brief Destructor
      public: virtual ~OBJLoader();

      /// \brief Load a mesh
      /// \param[in] _filename OBJ file to load
      /// \return Pointer to a new Mesh
      public: virtual Mesh *Load(const std::string &_filename);

      /// \internal
      /// \brief Private data pointer.
      GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
}
#endif
