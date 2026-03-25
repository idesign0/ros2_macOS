/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_OGRE_OGREGEOMETRY_HH_
#define GZ_RENDERING_OGRE_OGREGEOMETRY_HH_

#include <gz/utils/SuppressWarning.hh>

#include "gz/rendering/base/BaseGeometry.hh"
#include "gz/rendering/ogre/OgreObject.hh"

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
    class GZ_RENDERING_OGRE_VISIBLE OgreGeometry :
      public BaseGeometry<OgreObject>
    {
      protected: OgreGeometry();

      public: ~OgreGeometry() override;

      public: bool HasParent() const override;

      public: VisualPtr Parent() const override;

      public: virtual Ogre::MovableObject *OgreObject() const = 0;

      protected: virtual void SetParent(OgreVisualPtr _parent);

      GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
      protected: OgreVisualPtr parent;
      GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING

      private: friend class OgreVisual;
    };
    }
  }
}
#endif
