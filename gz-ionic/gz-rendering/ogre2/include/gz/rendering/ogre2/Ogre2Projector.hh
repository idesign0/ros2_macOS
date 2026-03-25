/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_OGRE2_OGRE2PROJECTOR_HH_
#define GZ_RENDERING_OGRE2_OGRE2PROJECTOR_HH_


#include <gz/utils/ImplPtr.hh>

#include "gz/rendering/config.hh"

#include "gz/rendering/base/BaseProjector.hh"
#include "gz/rendering/ogre2/Export.hh"
#include "gz/rendering/ogre2/Ogre2Visual.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {

    /// \brief Ogre 2.x implementation of a Projector class.
    class GZ_RENDERING_OGRE2_VISIBLE Ogre2Projector :
      public BaseProjector<Ogre2Visual>
    {
      /// \brief Constructor.
      protected: Ogre2Projector();

      /// \brief Destructor.
      public: virtual ~Ogre2Projector();

      // Documentation inherited.
      public: virtual void PreRender() override;

      // Documentation inherited.
      public: virtual void SetEnabled(bool _enabled) override;

      /// \brief Create projector resources
      private: void CreateProjector();

      /// \brief Update the list of cameras that the listener is added to.
      /// It loops through all cameras in each iteration to make sure
      /// we handle the decal's visibility in each of these cameras' view
      private: void UpdateCameraListener();

      /// \brief Only the ogre scene can instanstiate this class
      private: friend class Ogre2Scene;

      /// \cond warning
      /// \brief Private data pointer
      GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
      /// \endcond
    };
    }
  }
}
#endif
