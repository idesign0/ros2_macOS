/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_OGRE2_OGRE2GZOGRERENDERINGMODE_HH_
#define GZ_RENDERING_OGRE2_OGRE2GZOGRERENDERINGMODE_HH_

#include "gz/rendering/config.hh"
#include "gz/rendering/ogre2/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE
    {
      /// \brief Rendering modes so that GzHlms implementations
      /// follow alternate code paths or extra customizations
      /// when they're enabled
      enum GzOgreRenderingMode
      {
        /// \brief Normal rendering
        GORM_NORMAL = 0,

        /// \brief Render a solid color explicit per object.
        /// Used by e.g. Segmentation camera mode
        GORM_SOLID_COLOR = 1,

        /// \brief Like GORM_SOLID_COLOR, but if CustomParameter 2u
        /// is present, raw diffuse texture will be multiplied against
        /// the solid colour.
        ///
        /// Also Unlit will behave as if GORM_NORMAL
        ///
        /// Used by thermal camera
        GORM_SOLID_THERMAL_COLOR_TEXTURED = 2,

        /// \brief Total number of rendering modes
        GORM_COUNT = 3,
      };
    }  // namespace GZ_RENDERING_VERSION_NAMESPACE
  }    // namespace rendering
}  // namespace gz

#endif
