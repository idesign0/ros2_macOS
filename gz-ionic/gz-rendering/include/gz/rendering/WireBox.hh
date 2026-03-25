/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_WIREBOX_HH_
#define GZ_RENDERING_WIREBOX_HH_

#include "gz/rendering/config.hh"
#include "gz/rendering/Geometry.hh"
#include "gz/math/AxisAlignedBox.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \class WireBox WireBox.hh gz/rendering/WireBox
    /// \brief Draws a wireframe box.
    class GZ_RENDERING_VISIBLE WireBox :
      public virtual Geometry
    {
      /// \brief Constructor
      protected: WireBox();

      /// \brief Destructor
      public: virtual ~WireBox();

      /// \brief Set the axis aligned box
      /// \param[in] _box The axis aligned box
      public: virtual void SetBox(
                  const gz::math::AxisAlignedBox &_box) = 0;

      /// \brief Get the wireframe box.
      /// \return The wireframe box.
      public: virtual gz::math::AxisAlignedBox Box() const = 0;
    };
    }
  }
}

#endif
