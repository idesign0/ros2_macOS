/*
 * Copyright (C) 2025 Open Source Robotics Foundation
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

#ifndef GZ_RENDERING_OGRE_OGREFRUSTUMVISUAL_HH_
#define GZ_RENDERING_OGRE_OGREFRUSTUMVISUAL_HH_

#include <memory>

#include "gz/rendering/base/BaseFrustumVisual.hh"
#include "gz/rendering/ogre/OgreVisual.hh"
#include "gz/rendering/ogre/OgreIncludes.hh"
#include "gz/rendering/ogre/OgreScene.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // Forward declaration
    class OgreFrustumVisualPrivate;

    /// \brief Ogre implementation of a Frustum Visual.
    class GZ_RENDERING_OGRE_VISIBLE OgreFrustumVisual
      : public BaseFrustumVisual<OgreVisual>
    {
      /// \brief Constructor
      protected: OgreFrustumVisual();

      /// \brief Destructor
      public: virtual ~OgreFrustumVisual();

      // Documentation inherited.
      public: virtual void Init() override;

      // Documentation inherited.
      public: virtual void PreRender() override;

      // Documentation inherited.
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void Update() override;

      /// \brief Create the Frustum Visual in ogre
      private: void Create();

      /// \brief Clear data stored by dynamiclines
      private: void ClearVisualData();

      // Documentation inherited
      public: virtual void SetVisible(bool _visible) override;

      /// \brief Frustum Visual should only be created by scene.
      private: friend class OgreScene;

      /// \brief Private data class
      private: std::unique_ptr<OgreFrustumVisualPrivate> dataPtr;
    };
    }
  }
}
#endif
