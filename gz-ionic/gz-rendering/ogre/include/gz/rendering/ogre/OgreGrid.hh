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

#ifndef GZ_RENDERING_OGRE_OGREGRID_HH_
#define GZ_RENDERING_OGRE_OGREGRID_HH_

#include <memory>
#include "gz/rendering/base/BaseGrid.hh"
#include "gz/rendering/ogre/OgreGeometry.hh"
#include "gz/rendering/ogre/OgreIncludes.hh"

namespace Ogre
{
  class MovableObject;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // Forward declaration
    class OgreGridPrivate;

    /// \brief Ogre implementation of a grid geometry.
    class GZ_RENDERING_OGRE_VISIBLE OgreGrid
      : public BaseGrid<OgreGeometry>
    {
      /// \brief Constructor
      protected: OgreGrid();

      /// \brief Destructor
      public: ~OgreGrid() override;

      // Documentation inherited.
      public: void Init() override;

      // Documentation inherited.
      public: Ogre::MovableObject *OgreObject() const override;

      // Documentation inherited.
      public: void PreRender() override;

      // Documentation inherited.
      public: MaterialPtr Material() const override;

      // Documentation inherited.
      public: void SetMaterial(MaterialPtr _material, bool _unique) override;

      /// \brief Set material to grid geometry.
      /// \param[in] _material Ogre material.
      protected: virtual void SetMaterialImpl(OgreMaterialPtr _material);

      /// \brief Create the grid geometry in ogre
      private: void Create();

      /// \brief Grid should only be created by scene.
      private: friend class OgreScene;

      /// \brief Private data class
      private: std::unique_ptr<OgreGridPrivate> dataPtr;
    };
    }
  }
}
#endif
