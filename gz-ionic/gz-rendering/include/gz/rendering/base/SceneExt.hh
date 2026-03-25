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
#ifndef GZ_RENDERING_SCENEEXT_HH_
#define GZ_RENDERING_SCENEEXT_HH_

#include <string>

#include "gz/rendering/config.hh"
#include "gz/rendering/RenderTypes.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Scene Extension class. Provides API extension to the Scene
    /// class without breaking ABI.
    class GZ_RENDERING_VISIBLE SceneExt
    {
      /// \brief Constructor
      /// \param[in] _scene Pointer to scene
      public: SceneExt(Scene *_scene)
      {
        this->scene = _scene;
      }

      /// \brief Destructor
      public: ~SceneExt() = default;

      /// \brief Generic create function
      /// \param[in] _type Type of object to create
      /// \param[in] _name Name of object
      public: virtual ObjectPtr CreateExt(const std::string & _type,
          const std::string & _name = "")
      {
        (void)_type;
        (void)_name;
        return ObjectPtr();
      }

      /// \brief Pointer to scene
      protected: Scene *scene{nullptr};
    };
    }
  }
}
#endif
