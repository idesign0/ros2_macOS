/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#ifndef GZ_RENDERING_OGRE2_OGRE2MATERIALSWITCHER_HH_
#define GZ_RENDERING_OGRE2_OGRE2MATERIALSWITCHER_HH_

#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <gz/math/Color.hh>
#include "gz/rendering/config.hh"
#include "gz/rendering/ogre2/Export.hh"
#include "gz/rendering/ogre2/Ogre2RenderTypes.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreCamera.h>
#include <OgreMaterial.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // forward declarations
    class Ogre2SelectionBuffer;

    /// \brief Helper class to assign unique colors to renderables
    class GZ_RENDERING_OGRE2_VISIBLE Ogre2MaterialSwitcher :
      public Ogre::Camera::Listener
    {
      /// \brief Constructor
      public: explicit Ogre2MaterialSwitcher(Ogre2ScenePtr _scene);

      /// \brief Destructor
      public: ~Ogre2MaterialSwitcher();

      /// \brief Get the entity with a specific color
      /// \param[in] _color The entity's color.
      public: std::string EntityName(
              const gz::math::Color &_color) const;

      /// \brief Reset the color value incrementor
      public: void Reset();

      /// \brief Ogre's pre render update callback
      /// \param[in] _cam Ogre render target event containing information about
      /// the source render target.
      public: virtual void cameraPreRenderScene(
                  Ogre::Camera *_cam) override;

      /// \brief Ogre's post render update callback
      /// \param[in] _cam Ogre render target event containing information about
      /// the source render target.
      public: virtual void cameraPostRenderScene(Ogre::Camera *_cam) override;

      /// \brief Current unique color value
      private: gz::math::Color currentColor;

      /// \brief Color dictionary that maps the unique color value to
      /// renderable name
      private: std::map<unsigned int, std::string> colorDict;

      /// \brief A map of ogre datablock pointer to their original blendblocks
      private: std::unordered_map<Ogre::HlmsDatablock *,
          const Ogre::HlmsBlendblock *> datablockMap;

      /// \brief A map of ogre sub item pointer to their original low level
      /// material.
      /// Most objects don't use one so it should be almost always empty.
      private:
        std::vector<std::pair<Ogre::SubItem *, Ogre::MaterialPtr>> materialMap;

      /// \brief Increment unique color value that will be assigned to the
      /// next renderable
      private: void NextColor();

      /// \brief Selection Buffer class that make use of this class for
      /// selecting entitiies
      public: friend class Ogre2SelectionBuffer;

      /// \brief Plain material technique
      private: Ogre2ScenePtr scene;
    };
    }
  }
}
#endif
